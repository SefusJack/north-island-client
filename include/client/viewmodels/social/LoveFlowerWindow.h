#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_LOVE_FLOWER_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_LOVE_FLOWER_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/social/FriendList.h"
#include "client/models/guild/Guild.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Rect.h"
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

    /// @brief The Passion Bouquet's name form (original LoveFlowerWindow.xml, MK_Menu.pas TLoveFlowerForm).
    /// @details Item 34233 "love flower" ("Used to send out messages to the target"): the name of the player to send a message to, typed or picked from friends and guild members. OK goes on to the message form (LoveFlowerMessageWindow), and the bouquet's cell is marked while it is out.
    class LoveFlowerWindow {
        public:
            /// @brief Callbacks the form uses to reach the rest of the client.
            struct Hooks {
                std::function<core::Rect()> itemWindowBounds;
                std::function<void()> marksChanged;
                std::function<bool()> inFight;
                std::function<std::int32_t()> ownId;
                std::function<std::string()> ownName;
                std::function<std::optional<std::int32_t>(const std::string&)> playerNamed;
                std::function<void(int, std::int32_t)> askMessage;
                std::function<bool()> messageOpen;
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
            LoveFlowerWindow(
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

            /// @brief Opens the form for the bouquet in a bag place.
            /// @details Original OpenLoveFlowerForm: the bouquet of a bag place was used.
            /// @param bagSlot Bag place of the bouquet.
            void open(int bagSlot);

            /// @brief Releases the bouquet's cell and hides the form.
            /// @details Original CloseLoveFlowerForm: ReleaseLoveFlowerSlot and the form goes.
            void close();

            /// @brief Frees the bouquet's cell.
            /// @details Original ReleaseLoveFlowerSlot.
            void release();

        private:
            /// @brief Fills the name list from friends and guild members per the tick boxes.
            /// @details Original FillFlowerTargetList and ToggleFlowerFilter.
            void fillNames();

            /// @brief Types the picked name in the field, if the player is online.
            /// @details Original PickFlowerTarget: a double click on a name.
            /// @param row List row.
            void pick(int row);

            /// @brief Checks the name and goes on to the message form.
            /// @details Original ConfirmFlowerTarget.
            void confirm();

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
            int m_bagSlot = 0;
    };

}

#endif
