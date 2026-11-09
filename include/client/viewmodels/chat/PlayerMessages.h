#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_PLAYER_MESSAGES_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_PLAYER_MESSAGES_H

#include "client/services/GameServices.h"
#include "ui/MessagePanel.h"
#include "ui/Screen.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief Short messages for the player in the popup of PlayerMessages.xml (Re_TalkMsgFormPlus_1, at the top) or CentreMessages.xml (Re_TalkMsgFormPlus_2, the middle).
    class PlayerMessages {
        public:
            static constexpr int kDefaultDurationMs = 3000; // how long a message stays up, milliseconds

            /// @brief Loads a message layout.
            /// @param services Game services (layout, party message hook).
            /// @param screen Screen the popup is loaded into.
            /// @param layout Layout file; "CentreMessages.xml" makes the centre box.
            PlayerMessages(
                GameServices& services,
                ui::Screen& screen,
                const std::string& layout = "PlayerMessages.xml"
            );

            /// @brief Shows a message in the popup.
            /// @param text Message text.
            /// @param durationMs How long it stays, in ms.
            void show(const std::string& text, int durationMs = kDefaultDurationMs);

            /// @brief Shows a qol party member's message, grouping those shown within kGroupMs below each other in one box.
            /// @param text Message text; a text the box shows already is not shown twice.
            /// @param durationMs How long it stays, in ms.
            void showGrouped(const std::string& text, int durationMs);

            /// @brief Sets a callback told every text shown.
            /// @param observer The callback.
            void setObserver(std::function<void(const std::string&)> observer) { m_observer = std::move(observer); }

            /// @brief Keeps the top of the box at a minimum (40 in a battle, 10 otherwise).
            /// @param minTop Minimum top in px.
            /// @details Original: TSe_ShortMsgPanel.RenderWithChildren.
            void keepBelow(int minTop);

        private:
            static constexpr std::uint64_t kGroupMs = 800; // messages shown within this many milliseconds of the panel appearing are grouped
            static constexpr std::size_t kGroupedLines = 15; // most lines in a grouped message panel

            GameServices& m_services;
            ui::Screen& m_screen;
            ui::MessagePanel& m_panel;
            bool m_centre = false;
            bool m_grouping = false;
            std::uint64_t m_shownMs = 0;
            std::vector<std::string> m_group;
            std::function<void(const std::string&)> m_observer;
    };

}

#endif
