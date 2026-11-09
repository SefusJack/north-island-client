#include "client/viewmodels/social/BlessBalloonWindow.h"

#include "client/rules/TargetNames.h"

#include <utility>

namespace client {

    namespace {

        constexpr int kBlissLamp = 34325; // item 34325: Bliss Lamp
        constexpr int kMessageMs = 2000; // message display time, ms

        /// @brief Loads BlessBalloonWindow.xml into the screen and returns its window.
        /// @param services Shared game services (layout and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The BlessBalloon window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("BlessBalloonWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("BlessBalloon");
        }

    }

    BlessBalloonWindow::BlessBalloonWindow(
        GameServices& services,
        ui::Screen& screen,
        const FriendList& friends,
        const Guild& guild,
        const Inventory& inventory,
        BagMarks& marks,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_screen(screen),
          m_friends(friends),
          m_guild(guild),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_names(screen.get<ui::ListBox>("BlessBalloonNames")),
          m_name(screen.get<ui::TextBox>("BlessBalloonName")),
          m_friendsTick(screen.get<ui::CheckBox>("BlessBalloonFriends")),
          m_guildTick(screen.get<ui::CheckBox>("BlessBalloonGuild")) {
        screen.on("onBlessBalloonClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onBlessBalloonUse", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onBlessBalloonUp", [this](const ui::EventArgs&) { m_names.scrollUp(); });
        screen.on("onBlessBalloonDown", [this](const ui::EventArgs&) { m_names.scrollDown(); });
        screen.on("onBlessBalloonFilter", [this](const ui::EventArgs&) { fillNames(); });
        screen.on("onBlessBalloonPick", [this](const ui::EventArgs& args) { pick(args.index); });
    }

    void BlessBalloonWindow::release() {
        if (m_bagSlot >= 1 && m_bagSlot <= BagMarks::kSlots && m_marks.useMark(m_bagSlot) == BagMarks::kBlessBalloon) {
            m_marks.setUseMark(m_bagSlot, 0);
            m_hooks.marksChanged();
        }
        m_bagSlot = 0;
        m_name.clear();
    }

    void BlessBalloonWindow::close() {
        release();
        m_window.setVisible(false);
    }

    void BlessBalloonWindow::open(int bagSlot) {
        if (bagSlot < 1 || bagSlot > BagMarks::kSlots) {
            return;
        }
        if (m_window.isVisible() || m_hooks.messageOpen()) {
            m_hooks.closeMessage();
        }
        m_bagSlot = bagSlot;
        m_marks.setUseMark(bagSlot, BagMarks::kBlessBalloon);
        const core::Rect items = m_hooks.itemWindowBounds();
        const core::Rect area = m_window.bounds();
        const int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
        m_window.setPosition({left, items.y});
        fillNames();
        m_name.clear();
        m_window.setVisible(true);
        m_window.fitToScreen();
        if (m_screen.context().bringAbovePopups) {
            m_screen.context().bringAbovePopups(m_window.id());
        }
        m_hooks.hideItemWindow();
        m_hooks.marksChanged();
    }

    void BlessBalloonWindow::setName(const std::string& name) {
        if (m_window.isVisible()) {
            m_name.setText(name);
        }
    }

    void BlessBalloonWindow::fillNames() {
        m_names.clear();
        for (const std::string& name : targetNames(m_friends, m_guild, m_hooks.ownId(), m_friendsTick.isChecked(), m_guildTick.isChecked())) {
            m_names.addItem(name);
        }
    }

    void BlessBalloonWindow::pick(int row) {
        if (row < 0 || row >= m_names.itemCount()) {
            return;
        }
        const std::string& name = m_names.itemText(row);
        if (!m_hooks.playerNamed(name)) {
            m_messages.show("The player isn't online", 1500); // message display time in ms
            return;
        }
        m_name.setText(name);
        m_screen.focus("BlessBalloonName");
    }

    void BlessBalloonWindow::confirm() {
        if (m_hooks.inFight() || m_inventory.bagItem(m_bagSlot).itemId != kBlissLamp) {
            return;
        }
        const std::string name = m_name.text();
        if (name.empty()) {
            m_messages.show("Enter the name", kMessageMs);
            return;
        }
        std::int32_t targetId = m_hooks.ownId();
        if (name != m_hooks.ownName()) {
            const auto found = m_hooks.playerNamed(name);
            if (!found) {
                m_messages.show("Can't find players according to the name", kMessageMs);
                return;
            }
            targetId = *found;
        }
        if (m_hooks.messageOpen()) {
            return;
        }
        m_window.setVisible(false);
        m_hooks.askMessage(m_bagSlot, targetId);
    }

}
