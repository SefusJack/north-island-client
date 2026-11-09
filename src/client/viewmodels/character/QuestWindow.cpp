#include "client/viewmodels/character/QuestWindow.h"

#include "gfx/TextStyle.h"
#include "ui/DrawingArea.h"

#include <array>
#include <cctype>

namespace client {

    namespace {

        constexpr int kMostFlagBits = 1000; // quest flag bits (Mark.dat flag bit) scanned for finished quests: 1..1000
        constexpr int kEndSegment = 99; // text part "#99" of a mark: shown once the quest has ended
        constexpr int kSegments = 101; // text parts "#00".."#100" of a mark, indexed by the quest counter

        /// @brief Loads QuestWindow.xml into the screen.
        /// @param services Game services (layout, load context).
        /// @param screen Screen to load into.
        /// @return The "Quests" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("QuestWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Quests");
        }

        /// @brief Longest prefix of at most columns bytes that never splits a Big5 pair.
        /// @param text Text to measure.
        /// @param columns Byte limit.
        /// @return Length in bytes.
        /// @details Original: GetDbcsSafeLength.
        std::size_t safeLength(const std::string& text, std::size_t columns) {
            std::size_t length = 0;
            while (length < text.size()) {
                const std::size_t size =
                    static_cast<unsigned char>(text[length]) > 0x7F && length + 1 < text.size() ? 2 : 1; // lead byte of a two-byte Big5 character is above 0x7F
                if (length + size > columns) {
                    break;
                }
                length += size;
            }
            return length;
        }

        /// @brief Splits text into rows of at most columns bytes, also cutting at a carriage return.
        /// @param text Text to wrap.
        /// @param columns Row width in bytes.
        /// @return The rows, without empty ones.
        /// @details Original: AddWrappedLine (list width 190 px / 8 px per character).
        std::vector<std::string> wrapRows(std::string text, std::size_t columns) {
            std::vector<std::string> rows;
            while (!text.empty()) {
                const std::size_t cr = text.find('\r');
                std::string row;
                if (cr != std::string::npos && cr < columns) {
                    row = text.substr(0, cr);
                    text.erase(0, cr + 1);
                } else {
                    const std::size_t length = safeLength(text, columns);
                    row = text.substr(0, length);
                    text.erase(0, length);
                }
                if (!row.empty()) {
                    rows.push_back(row);
                }
            }
            return rows;
        }

        /// @brief Splits a counter mark's text into its "#NN" parts, indexed by NN.
        /// @param text Mark text; text without a leading '#' counts as part "#01".
        /// @return The parts by number.
        std::array<std::string, kSegments> segmentsOf(std::string text) {
            std::array<std::string, kSegments> segments;
            if (text.empty() || text[0] != '#') {
                text = "#01" + text;
            }
            std::size_t start = 0;
            int segment = 0;
            for (std::size_t i = 0; i < text.size(); ++i) {
                if (text[i] == '#') {
                    start = i + 3; // skip '#' and the two digits
                    const bool digits =
                        i + 2 < text.size() && std::isdigit(static_cast<unsigned char>(text[i + 1])) && std::isdigit(static_cast<unsigned char>(text[i + 2]));
                    segment = digits ? (text[i + 1] - '0') * 10 + (text[i + 2] - '0') : 0; // two decimal digits make the part number
                }
                if (i >= start) {
                    segments[static_cast<std::size_t>(segment)] += text[i];
                }
            }
            return segments;
        }

    }

    QuestWindow::QuestWindow(
        GameServices& services,
        ui::Screen& screen,
        QuestMarks& marks
    )
        : m_services(services),
          m_screen(screen),
          m_marks(marks),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::ListBox>("QuestsList")),
          m_detail(screen.get<ui::ListBox>("QuestsDetail")),
          m_title(screen.get<ui::Memo>("QuestsTitle")),
          m_doneSpot(screen.get<ui::Spot>("QuestsDoneColor")),
          m_goingOnSpot(screen.get<ui::Spot>("QuestsGoingOnColor")),
          m_detailSpot(screen.get<ui::Spot>("QuestsDetailColor")),
          m_pageSpot(screen.get<ui::Spot>("QuestsPageColor")) {
        screen.get<ui::DrawingArea>("QuestsMarks").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintMarks(canvas, at); });
        screen.on("onQuestsPick", [this](const ui::EventArgs& args) { showDetail(args.index); });
        screen.on("onQuestsPageUp", [this](const ui::EventArgs&) { turnPage(-1); });
        screen.on("onQuestsPageDown", [this](const ui::EventArgs&) { turnPage(1); });
        screen.on("onQuestsClose", [this](const ui::EventArgs&) { m_window.setVisible(false); });
    }

    void QuestWindow::toggle() {
        if (m_window.isVisible()) {
            m_window.setVisible(false);
            return;
        }
        update();
        show();
    }

    void QuestWindow::toggleFromNotebook() {
        if (m_window.isVisible()) {
            m_window.setVisible(false);
            return;
        }
        show();
    }

    void QuestWindow::update() {
        if (m_marks.changed()) {
            refresh();
            m_marks.markSeen();
        }
    }

    void QuestWindow::show() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        const int count = static_cast<int>(m_entries.size());
        const int pages = count / kPageRows + (count % kPageRows > 0 ? 1 : 0);
        for (int i = 0; i <= pages; ++i) {
            turnPage(1);
        }
    }

    void QuestWindow::refresh() {
        m_entries.clear();
        m_list.clear();
        m_scrollPos = 0;
        m_list.setTopIndex(0);
        const formats::MarkTable& table = m_services.marks;
        for (int flagBit = 1; flagBit <= kMostFlagBits; ++flagBit) {
            if (!m_marks.hasFlag(flagBit)) {
                continue;
            }
            const auto mark = table.at(table.indexOfFlag(flagBit));
            if (mark && !mark->get().hiddenInList) {
                m_list.addItem(mark->get().name, m_doneSpot.color());
                m_entries.push_back({mark->get().markId, true});
            }
        }
        for (int slot = 1; slot <= QuestMarks::kSlots; ++slot) {
            if (const int markId = m_marks.slot(slot).markId; markId > 0) {
                addGoingOn(table.indexOf(markId));
            }
        }
    }

    void QuestWindow::addGoingOn(int startIndex) {
        const formats::MarkTable& table = m_services.marks;
        const auto start = table.at(startIndex);
        if (!start) {
            return;
        }
        int questIndex = startIndex;
        while (questIndex <= table.count() && table.at(questIndex)->get().flagBit == 0) {
            ++questIndex;
        }
        const auto quest = table.at(questIndex);
        if (!quest) {
            return;
        }
        const int markId = quest->get().markId;
        for (const Entry& entry : m_entries) {
            if (entry.markId == markId && !entry.done) {
                return;
            }
        }
        if (!start->get().hiddenInList) {
            m_list.addItem(quest->get().name, m_goingOnSpot.color());
            m_entries.push_back({markId, false});
        }
    }

    void QuestWindow::showDetail(int index) {
        if (index < 0 || index >= static_cast<int>(m_entries.size())) {
            return;
        }
        const int markId = m_entries[static_cast<std::size_t>(index)].markId;
        const int slot = m_marks.slotOf(markId - 1);
        m_title.setText(m_list.itemText(index));
        m_detail.clear();
        const auto counter = m_services.marks.find(markId - 1);
        const auto segments = segmentsOf(counter ? counter->get().text : std::string());
        std::string text;
        if (slot == 0) {
            text = !segments[kEndSegment].empty() ? segments[kEndSegment] : "The quest has ended.";
        } else if (const int count = m_marks.slot(slot).count; count < kSegments) {
            text = segments[static_cast<std::size_t>(count)];
        }
        constexpr std::size_t kColumns = 190 / 8; // detail list is 190 px wide, 8 px per character
        for (const std::string& row : wrapRows(text, kColumns)) {
            m_detail.addItem(row, m_detailSpot.color());
        }
    }

    void QuestWindow::turnPage(int step) {
        const int count = static_cast<int>(m_entries.size());
        if (count == kPageRows) {
            return;
        }
        if (step < 0) {
            m_scrollPos = std::max(0, m_scrollPos - kPageRows);
        } else if (m_scrollPos + kPageRows < count) {
            m_scrollPos += kPageRows;
        }
        m_list.setTopIndex(m_scrollPos);
    }

    void QuestWindow::paintMarks(gfx::Canvas& canvas, core::Point at) {
        const int count = static_cast<int>(m_entries.size());
        for (int index = m_scrollPos; index < count && index < m_scrollPos + kPageRows; ++index) {
            const auto icon = m_services.pictures.find(m_entries[static_cast<std::size_t>(index)].done ? "icon_pass_1" : "icon_nopass_1");
            if (icon) {
                canvas.drawPicture(*icon, {at.x + 22, at.y + 57 + (index % kPageRows) * 24}); // icon column x 22, first row y 57, 24 px row pitch
            }
        }
        const int pages = count / kPageRows + (count % kPageRows > 0 ? 1 : 0);
        const std::string page = std::to_string(m_scrollPos / kPageRows + 1) + "/" + std::to_string(pages);
        const int width = static_cast<int>(page.size()) * 8; // character width in px
        // page number centred at x 115, y 300
        canvas.drawText({at.x + 115 - width / 2, at.y + 300}, page, {m_pageSpot.color(), gfx::TextEffect::Plain, std::nullopt});
    }

}
