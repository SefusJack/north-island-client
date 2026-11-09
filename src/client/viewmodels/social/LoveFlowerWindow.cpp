#include "client/viewmodels/social/LoveFlowerWindow.h"

#include "client/rules/TargetNames.h"

#include <utility>

namespace client {

    namespace {

        constexpr int kPassionBouquet = 34233; // item 34233: Passion Bouquet
        constexpr int kMessageMs = 2000; // message display time, ms

        /// @brief Loads LoveFlowerWindow.xml into the screen and returns its window.
        /// @param services Shared game services (layout and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The LoveFlower window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("LoveFlowerWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("LoveFlower");
        }

    }

    LoveFlowerWindow::LoveFlowerWindow(
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
          m_names(screen.get<ui::ListBox>("LoveFlowerNames")),
          m_name(screen.get<ui::TextBox>("LoveFlowerName")),
          m_friendsTick(screen.get<ui::CheckBox>("LoveFlowerFriends")),
          m_guildTick(screen.get<ui::CheckBox>("LoveFlowerGuild")) {
        screen.on("onLoveFlowerClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onLoveFlowerUse", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onLoveFlowerUp", [this](const ui::EventArgs&) { m_names.scrollUp(); });
        screen.on("onLoveFlowerDown", [this](const ui::EventArgs&) { m_names.scrollDown(); });
        screen.on("onLoveFlowerFilter", [this](const ui::EventArgs&) { fillNames(); });
        screen.on("onLoveFlowerPick", [this](const ui::EventArgs& args) { pick(args.index); });
    }

    void LoveFlowerWindow::release() {
        if (m_bagSlot >= 1 && m_bagSlot <= BagMarks::kSlots && m_marks.useMark(m_bagSlot) == BagMarks::kLoveFlower) {
            m_marks.setUseMark(m_bagSlot, 0);
            m_hooks.marksChanged();
        }
        m_bagSlot = 0;
    }

    void LoveFlowerWindow::close() {
        release();
        m_window.setVisible(false);
    }

    void LoveFlowerWindow::open(int bagSlot) {
        if (bagSlot < 1 || bagSlot > BagMarks::kSlots) {
            return;
        }
        m_bagSlot = bagSlot;
        m_marks.setUseMark(bagSlot, BagMarks::kLoveFlower);
        const core::Rect items = m_hooks.itemWindowBounds();
        const core::Rect area = m_window.bounds();
        const int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
        m_window.setPosition({left, items.y});
        fillNames();
        m_window.setVisible(true);
        if (m_screen.context().bringAbovePopups) {
            m_screen.context().bringAbovePopups(m_window.id());
        }
        m_hooks.marksChanged();
    }

    void LoveFlowerWindow::fillNames() {
        m_names.clear();
        for (const std::string& name : targetNames(m_friends, m_guild, m_hooks.ownId(), m_friendsTick.isChecked(), m_guildTick.isChecked())) {
            m_names.addItem(name);
        }
    }

    void LoveFlowerWindow::pick(int row) {
        if (row < 0 || row >= m_names.itemCount()) {
            return;
        }
        const std::string& name = m_names.itemText(row);
        if (!m_hooks.playerNamed(name)) {
            m_messages.show("The player isn't online", 1500); // message display time in ms
            return;
        }
        m_name.setText(name);
    }

    void LoveFlowerWindow::confirm() {
        if (m_hooks.inFight() || m_inventory.bagItem(m_bagSlot).itemId != kPassionBouquet) {
            return;
        }
        const std::string name = m_name.text();
        if (name.empty()) {
            m_messages.show("Enter the name", kMessageMs);
            return;
        }
        if (name == m_hooks.ownName()) {
            m_messages.show("You can't use it", kMessageMs);
            return;
        }
        const auto found = m_hooks.playerNamed(name);
        if (!found) {
            m_messages.show("Can't find players according to the name", kMessageMs);
            return;
        }
        if (m_hooks.messageOpen()) {
            return;
        }
        m_window.setVisible(false);
        m_hooks.askMessage(m_bagSlot, *found);
        m_name.clear();
    }

}
