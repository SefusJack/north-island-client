#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_STALL_PETS_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_STALL_PETS_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/FigureAnimation.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/pets/PetTeam.h"
#include "client/viewmodels/pets/PetInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "protocol/PetStallGoods.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief Window of someone's pet stall: its pets, marked by a click, and Buy.
    /// @details Original: StallPetsWindow.xml, TTH_BuyPetSallForm; pet list 23/113, Buy 23/63.
    class StallPetsWindow {
        public:
            /// @brief Loads the window and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param pets Player's pet team.
            /// @param status Player status (level, gold).
            /// @param messages Player message line.
            /// @param info Pet info popup.
            StallPetsWindow(
                GameServices& services,
                ui::Screen& screen,
                const PetTeam& pets,
                const PlayerStatus& status,
                PlayerMessages& messages,
                PetInfoPopup& info
            );

            /// @brief Tells whether the window is visible.
            /// @return True when shown.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Gives the id of the stall's owner.
            /// @return Owner id of the shown stall.
            std::int32_t ownerId() const { return m_goods.ownerId; }

            /// @brief Sizes the form for the pets, then shows it.
            /// @details Original: RecvPetStallOffer (23/113).
            /// @param goods Pets offered by the stall.
            void show(const protocol::PetStallGoods& goods);

            /// @brief Closes the form.
            /// @details Original: CloseBuyPetStall.
            void close();

        private:
            /// @brief Marks or unmarks a pet place after the level-86 rule and amity checks.
            /// @details Original: HandleMouseDown.
            /// @param place Pet place (1-based).
            void toggle(int place);

            /// @brief Sends the purchase of the marked pets.
            /// @details Original: BuyStallPets (23/63).
            void buy();

            /// @brief Sums the price of the marked pets.
            /// @return Total price in gold.
            std::int32_t total() const;

            /// @brief Paints the pets, the total and the info popup on hover.
            /// @param canvas Target canvas.
            /// @param at Window origin on the canvas.
            /// @param mouse Mouse position.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point at, core::Point mouse, std::uint64_t nowMs);

            static constexpr int kPlaces = 3;

            GameServices& m_services;
            ui::Screen& m_screen;
            const PetTeam& m_pets;
            const PlayerStatus& m_status;
            PlayerMessages& m_messages;
            PetInfoPopup& m_info;
            ui::Window& m_window;
            const ui::Spot& m_textSpot;
            protocol::PetStallGoods m_goods;
            bool m_infoShown = false;
            std::array<bool, kPlaces + 1> m_marked{};
            std::array<FigureAnimation, kPlaces + 1> m_animations{};
    };

}

#endif
