#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_BLESS_BALLOON_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_BLESS_BALLOON_WINDOW_H

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

    /// @brief The Bliss Lamp's name form (original BlessBalloonWindow.xml, CJ_Menu.pas TBlessBalloonForm).
    /// @details Item 34325 "bless balloon" ("Send best wishes to a good friend"): the name of the player to send best wishes to, typed or picked from friends and guild members. OK or Enter goes on to the message form (BlessMessageWindow), and the lamp's cell is marked while the form is up.
    class BlessBalloonWindow {
        public:
            /// @brief Callbacks the form uses to reach the rest of the client.
            struct Hooks {
                std::function<core::Rect()> itemWindowBounds;
                std::function<void()> hideItemWindow;
                std::function<void()> marksChanged;
                std::function<bool()> inFight;
                std::function<std::int32_t()> ownId;
                std::function<std::string()> ownName;
                std::function<std::optional<std::int32_t>(const std::string&)> playerNamed;
                std::function<void(int, std::int32_t)> askMessage;
                std::function<void()> closeMessage;
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
            BlessBalloonWindow(
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

            /// @brief Opens the form for the lamp in a bag place.
            /// @details Original OpenBlessBalloonForm: the lamp of a bag place was used.
            /// @param bagSlot Bag place of the lamp.
            void open(int bagSlot);

            /// @brief Releases the lamp's cell and hides the form.
            /// @details Original HideCtrl: ReleaseBlessSlot and the form goes.
            void close();

            /// @brief Frees the lamp's cell and clears the name.
            /// @details Original ReleaseBlessSlot.
            void release();

            /// @brief Sets the name when a player is clicked in the world while the form is up.
            /// @details Original SetWhisperTarget.
            /// @param name The player's name.
            void setName(const std::string& name);

        private:
            /// @brief Fills the name list from friends and guild members per the tick boxes.
            /// @details Original FillBlessTargetList and ToggleBlessFilter.
            void fillNames();

            /// @brief Types the picked name in the field, if the player is online.
            /// @details Original PickBlessTarget: a double click on a name.
            /// @param row List row.
            void pick(int row);

            /// @brief Checks the name and goes on to the message form.
            /// @details Original ConfirmBlessTarget.
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
