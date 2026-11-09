#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_BED_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_BED_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HouseInterior.h"
#include "client/rendering/painters/PetInfoPainter.h"
#include "client/models/pets/Pet.h"
#include "client/models/pets/PetTeam.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/BlockList.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace client {

    /// @brief The pet bed window of a house (BedWindow.xml, TTH_UseBedForm): sleeping pets on the left, carried pets on the right.
    /// @details Original: TH_Menu.pas TTH_UseBedForm. 62/43 puts the checked carried pets to bed, 62/44 wakes the checked sleepers.
    class BedWindow {
        public:
            /// @brief Callbacks into the rest of the client that the bed window needs.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
                std::function<bool(int)> petInStall;
            };

            /// @brief Loads the window layout and wires the lists and buttons.
            /// @param services Game services (layouts, pictures).
            /// @param screen Screen the window is loaded into.
            /// @param house The house interior with its utensils and beds.
            /// @param pets The player's pet team.
            /// @param messages Player message line for refusals.
            /// @param hooks Callbacks into the rest of the client.
            BedWindow(
                GameServices& services,
                ui::Screen& screen,
                const HouseInterior& house,
                const PetTeam& pets,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is shown.
            /// @return True when the window is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the window for the bed of a furniture piece and lists the pets again.
            /// @details Original: ShowCtrl; a bed has 1 to 4 places.
            /// @param furnIndex Furniture piece index of the bed.
            void open(int furnIndex);

            /// @brief Hides the window.
            void close() { m_window.setVisible(false); }

            /// @brief Rebuilds both lists (a row per carried pet and per sleeper, nothing checked) after the team or the sleepers changed.
            /// @details Original: RefreshBedPetLists.
            void refresh();

        private:
            /// @brief Toggles the check of a carried pet row; a pet that may not sleep here is refused and stays unchecked.
            /// @details Original: CheckPetStorable. The checked pets must also fit the bed's free places.
            /// @param row Row of the right list (from 0).
            void clickCarried(int row);

            /// @brief Toggles the check of a sleeper row; the carried pets plus the ones to wake must fit the team of four.
            /// @details Original: CheckBedRetrieveSpace.
            /// @param row Row of the left list (from 0).
            void clickSleeper(int row);

            /// @brief Sends one put-to-bed request (62/43) for each checked carried pet.
            /// @details Original: StorePetsInBed.
            void store();

            /// @brief Sends one take-from-bed request (62/44) for each checked sleeper.
            /// @details Original: RetrievePetsFromBed.
            void retrieve();

            /// @brief Gives the carried pet of a row of the right list.
            /// @param row Row of the right list (from 0).
            /// @return The pet, empty when there is none.
            const std::optional<Pet>& carried(int row) const;

            /// @brief Gives the pet asleep for a row of the left list.
            /// @param row Row of the left list (from 0).
            /// @return The pet, empty when there is none.
            std::optional<Pet> sleeper(int row) const;

            /// @brief Counts the checked rows of a list, leaving one row out.
            /// @param list List to count in.
            /// @param except Row not to count.
            /// @return Number of checked rows.
            int checkedCount(ui::BlockList& list, int except) const;

            /// @brief Paints the extra bed panels; closes the window when the player left the house floor room.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top-left corner of the paint area.
            void paintForm(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the pet info of each row of both lists.
            /// @param canvas Target canvas.
            /// @param at Top-left corner of the paint area.
            /// @param nowMs Current time in ms.
            void paintInfos(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the pet names (two lines when long, red for a pet that cannot be traded) and the element icons.
            /// @details Original: RenderWithChildren, after the controls.
            /// @param canvas Target canvas.
            /// @param at Top-left corner of the paint area.
            void paintNames(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const HouseInterior& m_house;
            const PetTeam& m_pets;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::BlockList& m_sleeperList;
            ui::BlockList& m_carriedList;
            const ui::Spot& m_carriedInfoSpot;
            const ui::Spot& m_sleeperInfoSpot;
            const ui::Spot& m_carriedElementSpot;
            const ui::Spot& m_sleeperElementSpot;
            const ui::Spot& m_nameOffsetSpot;
            const ui::Spot& m_untradableSpot;
            const ui::Spot& m_panelSpot;
            PetInfoPainter m_carriedInfo;
            PetInfoPainter m_sleeperInfo;
            int m_furnIndex = 0;
            int m_bedKind = 0;
    };

}

#endif
