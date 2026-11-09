#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_PETS_PET_INFO_POPUP_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_PETS_PET_INFO_POPUP_H

#include "client/services/GameServices.h"
#include "core/Point.h"
#include "protocol/PetInfoBlock.h"
#include "ui/Element.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <string>
#include <vector>

namespace client {

    /// @brief The pet info popup (original PetInfoPopup.xml, TCY_PopFNpcInfoForm).
    /// @details Yellow lines on a panel at the mouse while it is over the partner's pet in a pet trade or a pet of a pet stall, kept inside the 800 x 600 screen.
    class PetInfoPopup {
        public:
            /// @brief Loads the popup layout.
            /// @param services Shared game services.
            /// @param screen Screen the popup is added to.
            PetInfoPopup(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Shows the pet's lines on a panel whose top left corner is the mouse.
            /// @details Original ShowPetInfo and DrawControl.
            /// @param pet The pet's info block.
            /// @param maxHp The pet's maximum HP.
            /// @param maxSp The pet's maximum SP.
            /// @param mouse Mouse position the panel is placed at.
            void show(const protocol::PetInfoBlock& pet, int maxHp, int maxSp, core::Point mouse);

            /// @brief Hides the popup.
            void hide();

        private:
            /// @brief Paints the text lines on the panel.
            /// @param canvas Target canvas.
            /// @param at Top left corner of the panel.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Element& m_panel;
            const ui::Spot& m_widthSpot;
            const ui::Spot& m_textSpot;
            std::vector<std::string> m_lines;
    };

}

#endif
