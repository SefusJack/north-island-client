#include "client/viewmodels/events/Top100Window.h"

#include "client/rendering/hud/PageNumbers.h"

#include <algorithm>
#include <array>

namespace client {

    namespace {

        constexpr int kDigitRows = 10; // the digit picture holds the ten digits 0..9 stacked
        // job icon picture per job number 1..6 (index 0 unused): Killer, Warrior, Knight, Wit, Priest, Seer
        const std::array<const char*, 7> kJobIcons = {"", "Icon_Asn", "Icon_Samurai", "Icon_Knight", "Icon_Sage", "Icon_Clergyman", "Icon_Prophet"};
        const std::array<const char*, 7> kJobNames = {"", "Killer ", "Warrior ", "Knight ", "Wit ", "Priest ", "Seer "}; // job names per job number 1..6

        /// @brief Loads Top100Window.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "Top100Window" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("Top100Window.xml"), services.loadContext());
            return screen.get<ui::Window>("Top100Window");
        }

        /// @brief Builds the label of a place.
        /// @param place Place in the list.
        /// @return "NO." and the place padded to three characters.
        std::string placeLabel(int place) {
            std::string number = "   " + std::to_string(place);
            return "NO." + number.substr(number.size() - 3); // the place in three characters
        }

    }

    Top100Window::Top100Window(
        GameServices& services,
        ui::Screen& screen,
        Top100& ranks
    )
        : m_services(services),
          m_screen(screen),
          m_ranks(ranks),
          m_window(loadWindow(services, screen)),
          m_eventBackgroundSpot(screen.get<ui::Spot>("Top100EventBackground")),
          m_pageNumbersSpot(screen.get<ui::Spot>("Top100PageNumbers")),
          m_rowsSpot(screen.get<ui::Spot>("Top100Rows")),
          m_myRankSpot(screen.get<ui::Spot>("Top100MyRank")),
          m_levelWordSpot(screen.get<ui::Spot>("Top100LevelWord")),
          m_levelNameSpot(screen.get<ui::Spot>("Top100LevelName")),
          m_moneyIconSpot(screen.get<ui::Spot>("Top100MoneyIcon")),
          m_nameSpot(screen.get<ui::Spot>("Top100Name")),
          m_eventNameSpot(screen.get<ui::Spot>("Top100EventName")),
          m_textColorSpot(screen.get<ui::Spot>("Top100TextColor")) {
        for (int i = 1; i <= Top100::kRowsPerPage; ++i) {
            m_numbers.push_back(screen.get<ui::TextBox>("Top100No" + std::to_string(i)));
            m_jobs.push_back(screen.get<ui::DrawingArea>("Top100Job" + std::to_string(i)));
            const int row = i;
            m_jobs.back().get().setPainter([this, row](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                const int job = m_ranks.kind() == Top100::Kind::Level ? m_ranks.level((m_ranks.page() - 1) * Top100::kRowsPerPage + row).job : 0;
                if (job >= 1 && job <= 6) { // jobs 1..6 have an icon
                    if (const auto icon = m_services.pictures.find(kJobIcons[static_cast<std::size_t>(job)])) {
                        canvas.drawPicture(*icon, at);
                    }
                }
            });
        }
        screen.get<ui::DrawingArea>("Top100Layout").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { layout(canvas, at); });
        screen.get<ui::DrawingArea>("Top100Page").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onTop100Close", [this](const ui::EventArgs&) { close(); });
        screen.on("onTop100TabLevel", [this](const ui::EventArgs&) { m_ranks.showLevel(); });
        screen.on("onTop100TabMoney", [this](const ui::EventArgs&) { m_ranks.showMoney(); });
        screen.on("onTop100TabQuiz", [this](const ui::EventArgs&) { m_ranks.showQuiz(); });
        screen.on("onTop100TabPk", [this](const ui::EventArgs&) { m_ranks.showPk(); });
        screen.on("onTop100TabEvent", [this](const ui::EventArgs&) { m_ranks.showEvents(); });
        // event tab numbers 1 and 2
        screen.on("onTop100EventFirst", [this](const ui::EventArgs&) { m_ranks.showEventTab(1); });
        screen.on("onTop100EventSecond", [this](const ui::EventArgs&) { m_ranks.showEventTab(2); });
        screen.on("onTop100Previous", [this](const ui::EventArgs&) { m_ranks.previousPage(); });
        screen.on("onTop100Next", [this](const ui::EventArgs&) { m_ranks.nextPage(); });
    }

    void Top100Window::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_ranks.open();
    }

    std::vector<Top100Window::Row> Top100Window::shownRows() const {
        std::vector<Row> rows;
        const int first = (m_ranks.page() - 1);
        switch (m_ranks.kind()) {
            case Top100::Kind::Level: // level ranking: places up to the first with level 0
                for (int i = first * Top100::kRowsPerPage + 1; i <= m_ranks.page() * Top100::kRowsPerPage; ++i) {
                    if (i > m_ranks.levelCount() || m_ranks.level(i).level == 0) {
                        break;
                    }
                    rows.push_back({(i - 1) % Top100::kRowsPerPage + 1, i});
                }
                break;
            case Top100::Kind::Money: // money ranking: places up to the first empty name
                for (int i = first * Top100::kRowsPerPage + 1; i <= m_ranks.page() * Top100::kRowsPerPage; ++i) {
                    if (i > m_ranks.moneyCount() || m_ranks.money(i).empty()) {
                        break;
                    }
                    rows.push_back({(i - 1) % Top100::kRowsPerPage + 1, i});
                }
                break;
            case Top100::Kind::Quiz: // Q&A ranking: places up to the first empty name
                for (int i = first * Top100::kRowsPerPage + 1; i <= m_ranks.page() * Top100::kRowsPerPage && m_ranks.quizCount() != 0; ++i) {
                    if (i > m_ranks.quizCount() || m_ranks.quiz(i).empty()) {
                        break;
                    }
                    rows.push_back({(i - 1) % Top100::kRowsPerPage + 1, i});
                }
                break;
            case Top100::Kind::Pk: // PK ranking: places whose rank is 1..10
                for (int i = 1; i <= Top100::kRowsPerPage && m_ranks.pkCount() != 0; ++i) {
                    const int place = i + first * Top100::kRowsPerPage;
                    if (place > m_ranks.pkCount() || m_ranks.pk(place).rank < 1 || m_ranks.pk(place).rank > 10) { // valid ranks are 1..10
                        break;
                    }
                    rows.push_back({i, place});
                }
                break;
            case Top100::Kind::EventFirst: // first event ranking (shares the code of the second)
            case Top100::Kind::EventSecond: { // second event ranking
                const int tab = m_ranks.kind() == Top100::Kind::EventFirst ? 1 : 2;
                for (int i = 1; i <= Top100::kEventRowsPerPage && m_ranks.eventCount(tab) != 0; ++i) {
                    const int place = i + first * Top100::kEventRowsPerPage;
                    if (place > m_ranks.eventCount(tab) || m_ranks.event(tab, place).rank < 1 || m_ranks.event(tab, place).rank > 10) { // valid ranks are 1..10
                        break;
                    }
                    rows.push_back({i, place});
                }
                break;
            }
            case Top100::Kind::None: break; // nothing shown yet
        }
        return rows;
    }

    void Top100Window::layout(gfx::Canvas& canvas, core::Point at) {
        const Top100::Kind kind = m_ranks.kind();
        const bool events = kind == Top100::Kind::EventFirst || kind == Top100::Kind::EventSecond;
        m_screen.get<ui::ImageButton>("Top100TabLevel").setSelected(kind == Top100::Kind::None || kind == Top100::Kind::Level);
        m_screen.get<ui::ImageButton>("Top100TabMoney").setSelected(kind == Top100::Kind::Money);
        m_screen.get<ui::ImageButton>("Top100TabQuiz").setSelected(kind == Top100::Kind::Quiz);
        m_screen.get<ui::ImageButton>("Top100TabPk").setSelected(kind == Top100::Kind::Pk);
        m_screen.get<ui::ImageButton>("Top100TabEvent").setSelected(events);
        m_screen.get<ui::ImageButton>("Top100EventFirst").setVisible(events);
        m_screen.get<ui::ImageButton>("Top100EventSecond").setVisible(events);
        m_screen.get<ui::ImageButton>("Top100EventFirst").setSelected(m_ranks.eventTab() == 1);
        m_screen.get<ui::ImageButton>("Top100EventSecond").setSelected(m_ranks.eventTab() == 2);
        if (events) {
            if (const auto background = m_services.pictures.find("DEventBg_1")) {
                canvas.drawPicture(*background, {at.x + m_eventBackgroundSpot.at().x, at.y + m_eventBackgroundSpot.at().y});
            }
        }
        const bool paged = kind != Top100::Kind::None && m_ranks.pageCount() > 1;
        m_screen.get<ui::Element>("Top100Previous").setVisible(paged);
        m_screen.get<ui::Element>("Top100Next").setVisible(paged);
        m_screen.get<ui::TextBox>("Top100Header").setTextFromStart(m_ranks.header());
        for (auto& edit : m_numbers) {
            edit.get().setVisible(false);
        }
        for (auto& job : m_jobs) {
            job.get().setVisible(false);
        }
        for (const Row& row : shownRows()) {
            const std::size_t index = static_cast<std::size_t>(row.row - 1);
            switch (kind) {
                case Top100::Kind::Level: // level list: label with the place
                case Top100::Kind::Money: // money list: label with the place
                case Top100::Kind::Quiz: // Q&A list: label with the place
                    m_numbers[index].get().setText(placeLabel(row.place));
                    m_numbers[index].get().setVisible(true);
                    break;
                case Top100::Kind::Pk: // PK list: label with the PK rank
                    m_numbers[index].get().setText(placeLabel(m_ranks.pk(row.place).rank));
                    m_numbers[index].get().setVisible(true);
                    break;
                case Top100::Kind::EventFirst: // event lists have no number boxes
                case Top100::Kind::EventSecond: // event lists have no number boxes
                case Top100::Kind::None: break; // nothing shown yet
            }
            if (kind == Top100::Kind::Level) {
                const int job = m_ranks.level(row.place).job;
                if (job >= 1 && job <= 6) { // jobs 1..6 have an icon
                    m_jobs[index].get().setVisible(true);
                    m_jobs[index].get().setHint(kJobNames[static_cast<std::size_t>(job)]);
                }
            }
        }
    }

    void Top100Window::paint(gfx::Canvas& canvas, core::Point at) {
        const Top100::Kind kind = m_ranks.kind();
        if (kind != Top100::Kind::None && m_ranks.pageCount() > 1) {
            drawPageNumbers(canvas, {at.x + m_pageNumbersSpot.at().x, at.y + m_pageNumbersSpot.at().y}, m_ranks.page(), m_ranks.pageCount());
        }
        const std::vector<Row> rows = shownRows();
        switch (kind) {
            case Top100::Kind::Level: paintLevels(canvas, at, rows); break; // level ranking
            case Top100::Kind::EventFirst: // first event ranking
            case Top100::Kind::EventSecond: paintEvents(canvas, at, rows); break; // second event ranking
            case Top100::Kind::Money: // money ranking
            case Top100::Kind::Quiz: // Q&A ranking
            case Top100::Kind::Pk: paintNames(canvas, at, rows); break; // PK ranking
            case Top100::Kind::None: break; // nothing to paint
        }
    }

    void Top100Window::paintLevels(gfx::Canvas& canvas, core::Point at, const std::vector<Row>& rows) {
        const gfx::TextStyle style{m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const int mine = m_ranks.myLevelRank();
        if (mine == INT32_MAX) {
            canvas.drawText(
                {at.x + m_myRankSpot.at().x, at.y + m_myRankSpot.at().y},
                "You are not in the ranking list",
                style,
                m_myRankSpot.size().width,
                m_myRankSpot.size().height
            );
        } else if (mine != 0) {
            canvas.drawText(
                {at.x + m_myRankSpot.at().x, at.y + m_myRankSpot.at().y},
                "Your level ranks " + std::to_string(mine) + " place in the competition ",
                style,
                m_myRankSpot.size().width,
                m_myRankSpot.size().height
            );
        }
        const auto word = m_services.pictures.find("Main_Word_Lv");
        const auto digits = m_services.pictures.find("Num_White3_1");
        for (const Row& row : rows) {
            const int y = at.y + m_rowsSpot.at().y + (row.row - 1) * m_rowsSpot.pitch().y;
            if (word) {
                canvas.drawPicture(*word, {at.x + m_rowsSpot.at().x - m_levelWordSpot.at().x, y});
            }
            if (digits) {
                const gfx::Picture& picture = m_services.pictures.picture(*digits);
                const int digitHeight = picture.height() / kDigitRows;
                const std::string level = std::to_string(m_ranks.level(row.place).level);
                for (std::size_t j = 0; j < level.size(); ++j) {
                    // digits start two digit widths (less 1 px each) right of the "Lv" word origin, 1 px lower
                    canvas.drawPicture(
                        *digits,
                        {(static_cast<int>(j) + 2) * (picture.width() - 1) + at.x + m_rowsSpot.at().x, y + 1},
                        core::Rect{0, (level[j] - '0') * digitHeight, picture.width(), digitHeight}
                    );
                }
            }
            canvas.drawText(
                {at.x + m_rowsSpot.at().x + m_levelNameSpot.at().x, y},
                m_ranks.level(row.place).name,
                style,
                m_nameSpot.size().width,
                16
            ); // 16: text box height
        }
    }

    void Top100Window::paintNames(gfx::Canvas& canvas, core::Point at, const std::vector<Row>& rows) {
        const gfx::TextStyle style{m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const Top100::Kind kind = m_ranks.kind();
        if (kind == Top100::Kind::Money) {
            const int mine = m_ranks.myMoneyRank();
            if (mine == INT32_MAX) {
                canvas.drawText(
                    {at.x + m_myRankSpot.at().x, at.y + m_myRankSpot.at().y},
                    "You are not in the ranking list",
                    style,
                    m_myRankSpot.size().width,
                    m_myRankSpot.size().height
                );
            } else if (mine != 0) {
                canvas.drawText(
                    {at.x + m_myRankSpot.at().x, at.y + m_myRankSpot.at().y},
                    "Your money ranks " + std::to_string(mine) + " place in the competition ",
                    style,
                    m_myRankSpot.size().width,
                    m_myRankSpot.size().height
                );
            }
        }
        const auto coins = m_services.pictures.find("GdMoney2");
        for (const Row& row : rows) {
            const int y = at.y + m_rowsSpot.at().y + (row.row - 1) * m_rowsSpot.pitch().y;
            if (kind == Top100::Kind::Money) {
                if (coins) {
                    canvas.drawPicture(*coins, {at.x + m_rowsSpot.at().x - m_moneyIconSpot.at().x, y - m_moneyIconSpot.at().y});
                }
                canvas.drawText(
                    {at.x + m_rowsSpot.at().x + m_nameSpot.at().x, y},
                    m_ranks.money(row.place),
                    style,
                    m_nameSpot.size().width,
                    16
                ); // 16: text box height
            } else {
                const std::string& name = kind == Top100::Kind::Quiz ? m_ranks.quiz(row.place) : m_ranks.pk(row.place).name;
                canvas.drawText({at.x + m_rowsSpot.at().x + m_nameSpot.at().x, y}, name, style, m_nameSpot.size().width, 16); // 16: text box height
            }
        }
    }

    void Top100Window::paintEvents(gfx::Canvas& canvas, core::Point at, const std::vector<Row>& rows) {
        const gfx::TextStyle style{m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        for (const Row& row : rows) {
            const Top100::Place& place = m_ranks.event(m_ranks.eventTab(), row.place);
            canvas.drawText(
                {at.x + m_rowsSpot.at().x + m_eventNameSpot.at().x, at.y + m_rowsSpot.at().y + m_eventNameSpot.at().y + (row.row - 1) * m_rowsSpot.pitch().y},
                place.server + "   " + place.name,
                style,
                m_eventNameSpot.size().width,
                26
            ); // 26: text box height
        }
    }

}
