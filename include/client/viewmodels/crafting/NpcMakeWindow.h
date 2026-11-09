#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_NPC_MAKE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_NPC_MAKE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/rendering/painters/PetInfoPainter.h"
#include "client/models/pets/PetTeam.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/models/world/MapCharacter.h"
#include "ui/BlockList.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace client {

    /// @brief Form asking who makes a thing: the player (first row) or one of his human-kind pets.
    /// @details Original: NpcMakeWindow.xml, WS_Menu.pas TFNPC_MakeForm; shown with a Maid in the house.
    class NpcMakeWindow {
        public:
            using Picked = std::function<bool(int petPlace, int petId)>;

            /// @brief Loads the window layout and wires its events.
            /// @param services Game services.
            /// @param screen Screen the window is loaded into.
            /// @param pets The player's pets.
            /// @param player The player's map character.
            /// @param messages Player message line.
            /// @param closed Called when the window closes.
            NpcMakeWindow(
                GameServices& services,
                ui::Screen& screen,
                const PetTeam& pets,
                const MapCharacter& player,
                PlayerMessages& messages,
                std::function<void()> closed
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Lists the makeable pets (kind 0) and shows the form; with none the player is chosen at once.
            /// @details Original: ShowCtrl and ListMakeablePets.
            /// @param picked Continuation called with the chosen pet.
            void open(Picked picked);

            /// @brief Closes the window.
            /// @details Original: HideCtrl.
            void close();

        private:
            /// @brief Checks the chosen pet, then calls the opener's continuation.
            /// @details Original: NpcMakeItemPicked.
            /// @param row Chosen row (0: the player).
            void pick(int row);

            /// @brief Paints each pet's small face.
            /// @param canvas Target canvas.
            /// @param at Top-left of the area.
            /// @param nowMs Current time in ms.
            void paintFaces(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the player's face and name and each pet's name.
            /// @param canvas Target canvas.
            /// @param at Top-left of the area.
            void paintNames(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const PetTeam& m_pets;
            const MapCharacter& m_player;
            PlayerMessages& m_messages;
            std::function<void()> m_closed;
            ui::Window& m_window;
            ui::BlockList& m_list;
            const ui::Spot& m_firstPetSpot;
            const ui::Spot& m_playerFaceSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_playerNameSpot;
            const ui::Spot& m_untradableNameSpot;
            PetInfoPainter m_face;
            Picked m_picked;
            std::vector<int> m_places;
    };

}

#endif
