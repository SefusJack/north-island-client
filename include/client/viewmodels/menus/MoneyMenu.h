#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MENUS_MONEY_MENU_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MENUS_MONEY_MENU_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <functional>

namespace client {

    /// @brief The gold menu: gold carried, discard, trades and stalls.
    /// @details Original: MoneyMenu.xml, TCY_MoneyMenu. Discard opens the place-money box; the trades let the next
    /// player clicked be asked (mouse modes 9 items, 10 pets, 19 furniture); the stalls open their forms. Each first
    /// checks what the game checks: level 10, enough time online (25/44), the map, a team.
    class MoneyMenu {
        public:
            /// @brief Callbacks into the world and the forms the menu opens.
            struct Hooks {
                std::function<int()> mapId;
                std::function<bool()> inTeam;
                std::function<void()> placeMoney;
                std::function<void(int mode)> pickTradePartner;
                std::function<void(int kind)> openStall;
                std::function<bool()> onFlatGround;
                std::function<int()> stallKept;
                std::function<bool()> onTrojanWarMap;
                std::function<bool()> onQuizZoneMap;
                std::function<bool()> bathing;
                std::function<bool()> noStallScene;
            };

            /// @brief Loads the menu layout and wires its buttons and gold painter.
            /// @param services Shared game services.
            /// @param screen Screen the menu is loaded into.
            /// @param status Player status (gold, level).
            /// @param messages Player message line.
            /// @param hooks Callbacks into the world and forms.
            MoneyMenu(
                GameServices& services,
                ui::Screen& screen,
                const PlayerStatus& status,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the menu is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens or closes the menu.
            void toggle();

            /// @brief Hides the menu.
            void close() { m_window.setVisible(false); }

            /// @brief Sets whether the player has been online long enough to trade.
            /// @details Original: FEnoughOnlineTime, set by 25/44.
            /// @param enough True when online long enough.
            void setEnoughOnlineTime(bool enough) { m_enoughOnlineTime = enough; }

        private:
            /// @brief Checks level and online time shared by the buttons.
            /// @return False (after showing the game's message) when refused.
            bool levelAndTimeAllow();

            /// @brief Opens the place-money box when allowed.
            /// @details Original: PlaceMoneyClick.
            void placeMoney();

            /// @brief Starts picking a trade partner when allowed.
            /// @details Original: TradeItemClick, TradePetClick, TradeUtensilClick.
            /// @param mode Mouse mode: 9 items, 10 pets, 19 furniture.
            void trade(int mode);

            /// @brief Opens a stall form when allowed.
            /// @details Original: StallItemClick, StallPetClick, StallUtensilClick.
            /// @param kind Stall kind: 1 items, 2 pets, 3 furniture.
            void stall(int kind);

            ui::Screen& m_screen;
            const PlayerStatus& m_status;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            const ui::Spot& m_goldSpot;
            bool m_enoughOnlineTime = false;
    };

}

#endif
