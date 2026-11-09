#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_PET_STALL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_TRADE_PET_STALL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/FigureAnimation.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/pets/PetTeam.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "protocol/StallSale.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief Window for the player's own pet stall: up to three pets put up with prices, shop name, gold and sales income.
    /// @details Original: PetStallWindow.xml, TTH_PetStallForm; Set a Stall 23/60, End Stall 23/61, sales 23/114.
    class PetStallWindow {
        public:
            /// @brief Queries and actions the window needs from the rest of the client.
            struct Hooks {
                std::function<int()> mapId;
                std::function<bool()> inTeam;
                std::function<bool()> keepsPetStall;
                std::function<bool()> keepsAnyStall;
                std::function<void(const std::string& line)> chatLine;
                std::function<bool()> usesVehicle;
                std::function<bool()> fishing;
                std::function<bool()> remoteOn;
            };

            /// @brief Loads the window layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window layout is loaded into.
            /// @param pets The player's pet team.
            /// @param status The player's status (gold).
            /// @param messages Player message line for refusals.
            /// @param hooks Queries and actions supplied by the scene.
            PetStallWindow(
                GameServices& services,
                ui::Screen& screen,
                const PetTeam& pets,
                const PlayerStatus& status,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the window when closed, hides it when open.
            /// @details Original: TogglePetStallForm.
            void toggle();

            /// @brief Shows the window, with a fresh form unless the stall is kept; no pets, no form.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the window.
            void hide() { m_window.setVisible(false); }

            /// @brief The server opened the stall: the prices are kept, the form goes away.
            /// @details Original: RecvOwnPetStall.
            void stallOpened();

            /// @brief The stall closed: every place empties.
            /// @details Original: ClearOwnPetStall.
            void stallClosed() { clearPlaces(); }

            /// @brief Shows a sale: chat line, emptied place, income.
            /// @details Original: RecvPetStallSale (23/114).
            /// @param sale The sale.
            void showSale(const protocol::StallSale& sale);

            /// @brief Whether the pet of a team place is put up for sale.
            /// @details Original: TFollowNpc.FInStall.
            /// @param place Pet team place.
            /// @return True when the pet is in the stall.
            bool petInStall(int place) const { return place >= 1 && place <= PetTeam::kPlaces && m_inStall[static_cast<std::size_t>(place)]; }

        private:
            /// @brief One stall place: the pet put up and its mark.
            struct Place {
                int petPlace = 0;
                int npcId = 0;
                bool marked = false;
            };

            /// @brief Chooses the next pet of the team.
            /// @details Original: SelectNextStallPet.
            void selectNext();

            /// @brief Chooses the previous pet of the team.
            /// @details Original: SelectPrevStallPet.
            void selectPrevious();

            /// @brief Shows the chosen pet's name and gear.
            void showSelected();

            /// @brief Puts the chosen pet into the first free stall place, after the refusal checks.
            /// @details Original: AddPetToStall.
            void add();

            /// @brief Takes the marked pets out of the stall.
            /// @details Original: RemoveSelectedStallPets.
            void removeMarked();

            /// @brief Empties every place and lets the prices be typed again.
            /// @details Original: ClearPetStallSlots.
            void clearPlaces();

            /// @brief Empties one stall place.
            /// @param place Stall place (1..3).
            void emptyPlace(int place);

            /// @brief Closes the form; an unkept stall is cleared first.
            /// @details Original: ClosePetStallForm.
            void close();

            /// @brief Checks the game's conditions and asks the server to open the stall.
            /// @details Original: StartPetStall (23/60).
            void keep();

            /// @brief Asks the server to end the stall.
            /// @details Original: StopPetStall (23/61).
            void revoke();

            /// @brief Redraws the gear and names of the chosen pet and the stall places.
            void refresh();

            /// @brief Draws the pets, their numbers, the highlights, gold and income.
            /// @param canvas Canvas to draw on.
            /// @param at Top-left of the drawing area.
            /// @param mouse Mouse position.
            /// @param nowMs Time in milliseconds (animation).
            void paint(gfx::Canvas& canvas, core::Point at, core::Point mouse, std::uint64_t nowMs);

            /// @brief The price box of a place.
            /// @param place Stall place (1..3).
            /// @return The text box.
            ui::TextBox& price(int place);

            /// @brief The price typed for a place.
            /// @param place Stall place (1..3).
            /// @return The price, 0 when empty.
            std::int32_t priceOf(int place);

            static constexpr int kPlaces = 3;

            GameServices& m_services;
            ui::Screen& m_screen;
            const PetTeam& m_pets;
            const PlayerStatus& m_status;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_shopName;
            ui::TextBox& m_petName;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_fixedNameSpot;
            std::array<Place, kPlaces + 1> m_places{};
            std::array<bool, PetTeam::kPlaces + 1> m_inStall{};
            int m_order = 0;
            int m_selected = 0;
            int m_count = 0;
            bool m_open = false;
            std::int32_t m_income = 0;
            std::string m_lastName;
            std::array<FigureAnimation, kPlaces + 1> m_animations{};
    };

}

#endif
