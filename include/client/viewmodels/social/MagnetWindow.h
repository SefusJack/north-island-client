#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_MAGNET_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_MAGNET_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/social/FriendList.h"
#include "client/models/guild/Guild.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/models/social/MagnetPlayer.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/CheckBox.h"
#include "ui/ListBox.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief The Magic Magnet form (original MagnetWindow.xml, HL_Menu.pas THL_MagnetForm, accessory 34021).
    /// @details A wearer of level 50 or more picks a player who wears one too and is online; the server asks that player ("<name> Teleport you to his area by using Magic Magnet, do you agree? ", 10 seconds, 23/80 1) and, when he agrees, brings him to the wearer (23/80 3 with 20). The asked one's checks make him refuse or say why he cannot (23/90 2, 3, 4); the wearer's magnet is busy while he waits (23/80 2) and a right click on it gives up (RefuseMagnetTeleport).
    class MagnetWindow {
        public:
            /// @brief Callbacks the form uses to reach the rest of the client.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<std::optional<MagnetPlayer>(const std::string&)> playerNamed;
                std::function<std::optional<MagnetPlayer>(std::int32_t)> playerWithId;
                std::function<std::string()> ownName;
                std::function<std::int32_t()> ownId;
                std::function<int()> ownLevel;
                std::function<bool()> ownWearsMagnet;
                std::function<bool()> inFight;
                std::function<bool()> onVehicle;
                std::function<bool()> stalled;
                std::function<bool()> sceneBarsUse;
                std::function<bool()> sceneBarsAnswer;
                std::function<void(const std::string&, std::function<void()>, std::function<void()>, std::function<void()>)> ask;
                std::function<void()> closeAsk;
                std::function<void()> marksChanged;
            };

            /// @brief Builds the form.
            /// @param services Shared game services.
            /// @param screen Screen the window is added to.
            /// @param friends The friend list used for the name list.
            /// @param guild The guild used for the name list.
            /// @param inventory The player's bag.
            /// @param marks Marks drawn on bag cells.
            /// @param messages Sink for player messages.
            /// @param hooks Callbacks to the client.
            MagnetWindow(
                GameServices& services,
                ui::Screen& screen,
                const FriendList& friends,
                const Guild& guild,
                const Inventory& inventory,
                BagMarks& marks,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True while the form is open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form for the item in a bag place with the list filled.
            /// @details Original HandleMagnetChoice 0 (item place in FControls.V[5]) and ShowCtrl.
            /// @param bagSlot Bag place of the Magic Magnet.
            void open(int bagSlot);

            /// @brief Hides the form.
            void close();

            /// @brief Closes the form in a fight or when the item in its place is no Magic Magnet.
            /// @details Original DrawControl.
            void update();

            /// @brief Shows the question for the asked player.
            /// @details Original RecvMagnetRequest 1.
            /// @param requesterId Id of the player who asks.
            void asked(std::int32_t requesterId);

            /// @brief Marks the wearer's magnet busy while the answer is awaited.
            /// @details Original RecvMagnetRequest 2.
            /// @param targetId Id of the asked player.
            /// @param bagSlot Bag place of the magnet.
            void waiting(std::int32_t targetId, int bagSlot);

            /// @brief Shows the result of a request and clears the waiting state.
            /// @details Original RecvMagnetResult.
            /// @param code Result code from the server.
            void result(int code);

            /// @brief Gives up while the answer is awaited.
            /// @details Original ItemSlotRightClick of the Magic Magnet (RefuseMagnetTeleport).
            void giveUp();

        private:
            /// @brief Fills the name list from friends and guild members per the tick boxes.
            /// @details Original FillMagnetTargetList and ToggleMagnetFilter.
            void fillNames();

            /// @brief Types the picked name in the field, if the player is online.
            /// @details Original PickMagnetTarget: a double click on a name.
            /// @param row List row.
            void pick(int row);

            /// @brief Checks the name and the conditions, then sends the request.
            /// @details Original UseMagicMagnet.
            void use();

            /// @brief Answers the question with the asked one's checks.
            /// @details Original AcceptMagnetTeleport.
            void accept();

            /// @brief Refuses the teleport.
            /// @details Original RefuseMagnetTeleport.
            void refuse();

            /// @brief Answers that the time ran out.
            /// @details Original MagnetTeleportTimeout.
            void timeout();

            ui::Screen& m_screen;
            const FriendList& m_friends;
            const Guild& m_guild;
            const Inventory& m_inventory;
            BagMarks& m_marks;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::ListBox& m_names;
            ui::TextBox& m_name;
            ui::CheckBox& m_friendsTick;
            ui::CheckBox& m_guildTick;
            int m_itemSlot = 0;
            std::int32_t m_requester = 0;
            int m_waitingSlot = 0;
            bool m_asking = false;
            int m_reason = 0;
    };

}

#endif
