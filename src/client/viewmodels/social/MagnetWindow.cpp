#include "client/viewmodels/social/MagnetWindow.h"

#include "client/rules/TargetNames.h"
#include "client/rules/Vitality.h"
#include "protocol/MagnetRequests.h"

namespace client {

    namespace {

        constexpr int kMagicMagnet = 34021; // item 34021: Magic Magnet
        constexpr int kMessageMs = 2000; // message display time, ms
        constexpr int kMostLevel = 50; // minimum total level (level plus reborn) of both players to use the magnet

        /// @brief Loads MagnetWindow.xml into the screen and returns its window.
        /// @param services Shared game services (layout and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The MagnetWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MagnetWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("MagnetWindow");
        }

        /// @brief Gives the message text for a magnet result code.
        /// @details Original RecvMagnetResult.
        /// @param code Result code from the server (or the reason codes sent with 23/90).
        /// @return The text to show.
        std::string resultText(int code) {
            switch (code) {
                case 1: return "He or she is trading, cancel"; // result 1: the asked player is trading
                case 2: return "He or she set a stall,cancel"; // result 2: the asked player keeps a stall
                case 3: return "He or she is using the Magic Magnet,cancel"; // result 3: the asked player is using a Magic Magnet
                case 4: return "You are regarded as the unwelcome people, cancel "; // result 4: the asked player refuses (unwelcome)
                case 5: return "Busy.. cancel"; // result 5: busy (the wearer's side)
                case 6: return "He or she is busy...cancel"; // result 6: the asked player is busy
                case 7: return "Battle...,cancel"; // result 7: the wearer is in battle
                case 8: return "He or she is in battle, cancel"; // result 8: the asked player is in battle
                case 9: return "Operation unavailable in the current scene"; // result 9: the scene bars the function (wearer's side)
                case 10: return "The function is not available in his or her current scene"; // result 10: the scene bars the function for the asked player
                case 11: return "Trading\xA1K cancel"; // result 11: trading (the wearer's side)
                case 12: return "set a stall\xA1K cancel"; // result 12: a stall is open (the wearer's side)
                case 13: return "Using vehicle,cancel"; // result 13: on a vehicle (the wearer's side)
                case 14: return "He or she is using vehicle,cancel"; // result 14: the asked player is on a vehicle
                case 15: return "Can't use the function, because your level is less than 50"; // result 15: the wearer's level is under 50
                case 16: return "Can't use the function, because his or her level is less than 50"; // result 16: the asked player's level is under 50
                case 17: return "Can't find players according to the name"; // result 17: no player of that name is known
                case 18: return "did not respond"; // result 18: no answer within the time (timeout)
                case 19: return "did not respond."; // result 19: no answer (variant text)
                case 20: return "Teleport Succeeded"; // result 20: the teleport succeeded
                case 21: return "Can't teleport players to your vicinity."; // result 21: the target cannot be teleported to the wearer's vicinity
                case 254: return "This player is off line"; // result 254: the player is offline
                case 255: return "He or she cancelled"; // result 255: the asked player cancelled
                default: return "Cancel"; // any other code: plain cancel
            }
        }

    }

    MagnetWindow::MagnetWindow(
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
          m_names(screen.get<ui::ListBox>("MagnetNames")),
          m_name(screen.get<ui::TextBox>("MagnetName")),
          m_friendsTick(screen.get<ui::CheckBox>("MagnetFriends")),
          m_guildTick(screen.get<ui::CheckBox>("MagnetGuild")) {
        screen.on("onMagnetClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onMagnetUse", [this](const ui::EventArgs&) { use(); });
        screen.on("onMagnetUp", [this](const ui::EventArgs&) { m_names.scrollUp(); });
        screen.on("onMagnetDown", [this](const ui::EventArgs&) { m_names.scrollDown(); });
        screen.on("onMagnetFilter", [this](const ui::EventArgs&) { fillNames(); });
        screen.on("onMagnetPick", [this](const ui::EventArgs& args) { pick(args.index); });
    }

    void MagnetWindow::open(int bagSlot) {
        m_itemSlot = bagSlot;
        fillNames();
        m_window.setVisible(true);
        if (m_screen.context().bringAbovePopups) {
            m_screen.context().bringAbovePopups(m_window.id());
        }
    }

    void MagnetWindow::close() { m_window.setVisible(false); }

    void MagnetWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        if (m_hooks.inFight() || (m_itemSlot != 0 && m_inventory.bagItem(m_itemSlot).itemId != kMagicMagnet)) {
            close();
        }
    }

    void MagnetWindow::fillNames() {
        m_names.clear();
        for (const std::string& name : targetNames(m_friends, m_guild, m_hooks.ownId(), m_friendsTick.isChecked(), m_guildTick.isChecked())) {
            m_names.addItem(name);
        }
    }

    void MagnetWindow::pick(int row) {
        if (row < 0 || row >= m_names.itemCount()) {
            return;
        }
        const std::string& name = m_names.itemText(row);
        if (!m_hooks.playerNamed(name)) {
            m_messages.show("The player isn't online", 1500); // message display time in ms
        } else {
            m_name.setText(name);
        }
    }

    void MagnetWindow::use() {
        if (m_hooks.inFight() || m_inventory.bagItem(m_itemSlot).itemId != kMagicMagnet || m_marks.lockNotice(m_itemSlot, 0) ||
            !m_marks.isAvailable(m_itemSlot)) {
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
        const auto target = m_hooks.playerNamed(name);
        if (!target) {
            m_messages.show("Can't find players according to the name", kMessageMs);
        } else if (!target->wearsMagnet) {
            m_messages.show("Can't use the function, because he or she doesn't wear Magic Magnet", kMessageMs);
        } else if (m_hooks.ownLevel() < kMostLevel) {
            m_messages.show("Can't use the function, because your level is less than 50", kMessageMs);
        } else if (totalLevel(target->level, target->reborn) < kMostLevel) {
            m_messages.show("Can't use the function, because his or her level is less than 50", kMessageMs);
        } else if (m_hooks.onVehicle()) {
            m_messages.show("Using vehicle,cancel", kMessageMs);
        } else if (m_hooks.sceneBarsUse()) {
            m_messages.show("The current scene doesn't allow you to use the function", kMessageMs);
        } else {
            m_hooks.send(protocol::askMagnetRequest(m_itemSlot, target->id));
            close();
        }
    }

    void MagnetWindow::asked(std::int32_t requesterId) {
        m_requester = requesterId;
        m_asking = true;
        const auto requester = m_hooks.playerWithId(requesterId);
        m_hooks.ask(
            (requester ? requester->name : std::string()) + " Teleport you to his area by using Magic Magnet, do you agree? ",
            [this] { accept(); },
            [this] { refuse(); },
            [this] { timeout(); }
        );
    }

    void MagnetWindow::waiting(std::int32_t targetId, int bagSlot) {
        (void)targetId;
        m_waitingSlot = bagSlot;
        m_marks.setBusyKind(bagSlot, 1);
        m_hooks.marksChanged();
    }

    void MagnetWindow::accept() {
        m_asking = false;
        m_reason = 0;
        const auto requester = m_hooks.playerWithId(m_requester);
        if (!requester) {
            m_reason = 17; // reason 17: the requester is not known
        } else if (m_hooks.inFight()) {
            m_reason = 7; // reason 7: in a fight
        } else if (!m_hooks.ownWearsMagnet()) {
            m_hooks.send(protocol::refuseMagnetRequest());
            return;
        } else if (m_hooks.ownLevel() < kMostLevel) {
            m_reason = 15; // reason 15: own level under 50
        } else if (totalLevel(requester->level, requester->reborn) < kMostLevel) {
            m_reason = 16; // reason 16: the requester's level under 50
        } else if (m_hooks.stalled()) {
            m_reason = 12; // reason 12: a stall is open
        } else if (m_hooks.onVehicle()) {
            m_reason = 13; // reason 13: on a vehicle
        } else if (m_hooks.sceneBarsAnswer()) {
            m_reason = 9; // reason 9: the scene bars the function
        } else {
            m_hooks.send(protocol::agreeMagnetRequest());
            return;
        }
        m_hooks.send(protocol::cannotMagnetRequest(m_reason));
    }

    void MagnetWindow::refuse() {
        m_asking = false;
        m_hooks.send(protocol::refuseMagnetRequest());
    }

    void MagnetWindow::timeout() {
        m_asking = false;
        m_reason = 18; // reason 18: no answer within the time
        m_hooks.send(protocol::cannotMagnetRequest(m_reason));
    }

    void MagnetWindow::giveUp() {
        if (m_waitingSlot != 0) {
            refuse();
        }
    }

    void MagnetWindow::result(int code) {
        m_messages.show(resultText(code), kMessageMs);
        if (m_asking) {
            m_asking = false;
            m_hooks.closeAsk();
        }
        if (m_waitingSlot != 0 && m_marks.busyKind(m_waitingSlot) == 1) {
            m_marks.setBusyKind(m_waitingSlot, 0);
            m_hooks.marksChanged();
        }
        m_requester = 0;
        m_waitingSlot = 0;
        m_reason = 0;
    }

}
