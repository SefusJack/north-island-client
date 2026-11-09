#include "client/viewmodels/instances/InstanceCreateForm.h"

#include "client/rendering/hud/PageNumbers.h"
#include "ui/DrawingArea.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kRowsPerPage = 10; // instances listed per page
        constexpr int kShortMs = 1500; // short message duration, ms

        /// @brief Loads the form's layout and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the form is loaded into.
        /// @return The form's window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("InstanceCreateForm.xml"), services.loadContext());
            return screen.get<ui::Window>("InstanceCreateForm");
        }

    }

    InstanceCreateForm::InstanceCreateForm(
        GameServices& services,
        ui::Screen& screen,
        Instances& instances,
        PlayerMessages& messages
    )
        : m_screen(screen),
          m_instances(instances),
          m_messages(messages),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::ListBox>("InstanceCreateList")),
          m_titleSpot(screen.get<ui::Spot>("InstanceCreateTitle")),
          m_playersSpot(screen.get<ui::Spot>("InstanceCreatePlayers")),
          m_levelSpot(screen.get<ui::Spot>("InstanceCreateLevel")),
          m_guildSpot(screen.get<ui::Spot>("InstanceCreateGuild")),
          m_timeSpot(screen.get<ui::Spot>("InstanceCreateTime")),
          m_descriptionSpot(screen.get<ui::Spot>("InstanceCreateDescription")),
          m_pageNumbersSpot(screen.get<ui::Spot>("InstanceCreatePageNumbers")),
          m_messageSpot(screen.get<ui::Spot>("InstanceCreateMessage")),
          m_message(services, screen, [this](const std::string& text) { send(text); }) {
        screen.get<ui::DrawingArea>("InstanceCreateTexts").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onInstanceCreateClose", [this](const ui::EventArgs&) { closeAndClear(); });
        screen.on("onInstanceCreateCreate", [this](const ui::EventArgs&) { confirmCreate(); });
        screen.on("onInstanceCreatePrevious", [this](const ui::EventArgs&) { previousPage(); });
        screen.on("onInstanceCreateNext", [this](const ui::EventArgs&) { nextPage(); });
        screen.on("onInstanceCreateSelect", [this](const ui::EventArgs& args) { select(args.index); });
        screen.on("onInstanceCreateDouble", [this](const ui::EventArgs& args) {
            if (args.index >= 0 && args.index < static_cast<int>(m_shown.size())) {
                confirmCreate();
            }
        });
    }

    void InstanceCreateForm::open() {
        if (!m_loaded) {
            loadList();
        }
        m_message.close();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void InstanceCreateForm::close() {
        m_window.setVisible(false);
        m_message.close();
    }

    void InstanceCreateForm::clearSelection() {
        m_selected = -1;
        m_list.setSelectedIndex(-1);
    }

    void InstanceCreateForm::listChanged() {
        if (m_loaded) {
            m_loaded = false;
            m_selected = -1;
            m_page = 0;
            m_pages = 0;
            m_all.clear();
            m_shown.clear();
            m_list.clear();
        }
        loadList();
    }

    void InstanceCreateForm::loadList() {
        if (m_loaded) {
            return;
        }
        m_all = m_instances.createRows();
        m_shown = m_all;
        m_list.clear();
        for (const InstanceCreateRow& row : m_shown) {
            m_list.addItem(row.title);
        }
        m_page = 1;
        const int count = static_cast<int>(m_all.size());
        m_pages = std::max(1, count / kRowsPerPage + (count % kRowsPerPage != 0 ? 1 : 0));
        m_loaded = true;
    }

    void InstanceCreateForm::showPage(int page) {
        m_selected = -1;
        m_shown.clear();
        m_list.clear();
        const std::size_t first = static_cast<std::size_t>(std::max(page - 1, 0) * kRowsPerPage);
        for (std::size_t i = first; i < m_all.size() && static_cast<int>(m_shown.size()) < kRowsPerPage; ++i) {
            m_shown.push_back(m_all[i]);
            m_list.addItem(m_all[i].title);
        }
    }

    void InstanceCreateForm::previousPage() {
        if (m_message.isOpen() || m_page - 1 < 1) {
            return;
        }
        --m_page;
        showPage(m_page);
    }

    void InstanceCreateForm::nextPage() {
        if (m_message.isOpen() || m_page + 1 > m_pages) {
            return;
        }
        ++m_page;
        showPage(m_page);
    }

    void InstanceCreateForm::select(int index) {
        if (!m_message.isOpen()) {
            m_selected = index;
            m_list.setSelectedIndex(index);
        }
    }

    void InstanceCreateForm::confirmCreate() {
        if (m_selected == -1) {
            m_messages.show("Hasn't choosen the instance you want to create", kShortMs);
            return;
        }
        if (m_selected < static_cast<int>(m_shown.size()) && m_instances.mayCreateScene(m_shown[static_cast<std::size_t>(m_selected)].sceneId)) {
            const core::Rect bounds = m_window.bounds();
            m_message.open({bounds.x + m_messageSpot.at().x, bounds.y + m_messageSpot.at().y});
        }
    }

    void InstanceCreateForm::send(const std::string& message) {
        if (m_selected == -1) {
            m_messages.show("Hasn't choosen the instance you want to create", kShortMs);
            return;
        }
        if (m_selected < static_cast<int>(m_shown.size()) && m_instances.create(m_shown[static_cast<std::size_t>(m_selected)].sceneId, message)) {
            m_message.clearText();
            close();
        }
    }

    void InstanceCreateForm::closeAndClear() {
        clearSelection();
        close();
    }

    void InstanceCreateForm::paint(gfx::Canvas& canvas, core::Point at) {
        if (m_message.isOpen()) {
            m_message.place({at.x + m_messageSpot.at().x, at.y + m_messageSpot.at().y});
            if (m_selected != -1) {
                m_list.setSelectedIndex(m_selected);
            }
        }
        int shown = m_list.hoverIndex();
        if (m_selected != -1) {
            shown = m_selected;
        }
        if (shown >= 0 && shown < static_cast<int>(m_shown.size())) {
            const InstanceCreateRow& row = m_shown[static_cast<std::size_t>(shown)];
            const gfx::TextStyle style{m_titleSpot.color(), gfx::TextEffect::Plain, std::nullopt};
            const auto text = [&](const std::string& line, core::Point where) {
                canvas.drawText({at.x + where.x, at.y + where.y}, line, style, m_titleSpot.size().width, m_titleSpot.size().height);
            };
            text(row.title, m_titleSpot.at());
            text(row.players, m_playersSpot.at());
            text(row.level, m_levelSpot.at());
            text(row.guildOnly, m_guildSpot.at());
            text(row.time, m_timeSpot.at());
            text(row.description, m_descriptionSpot.at());
        }
        drawPageNumbersFrom(canvas, {at.x + m_pageNumbersSpot.at().x, at.y + m_pageNumbersSpot.at().y}, m_page, m_pages);
    }

}
