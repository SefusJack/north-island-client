#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_POTENTIAL_RETURN_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_POTENTIAL_RETURN_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/FigureAnimation.h"
#include "client/rendering/painters/PetInfoPainter.h"
#include "client/models/items/Inventory.h"
#include "client/models/pets/PetTeam.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/viewmodels/chat/TalkDialog.h"
#include "ui/BlockList.h"
#include "ui/IconGrid.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <functional>
#include <random>
#include <vector>

namespace client {

    /// @brief Potential Return form: pick a pet and use the Anti-Potential Pill to make its potential 0.
    /// @details Original: PotentialReturnWindow.xml, HL_Menu.pas TPotentialReturnForm; pill 34351; OK sends 23/130 (pill place, pet place); result 23/216.
    class PotentialReturnWindow {
        public:
            /// @brief Callbacks into the rest of the game.
            struct Hooks {
                std::function<bool()> fighting;
                std::function<void()> closeItemWindow;
                std::function<void(const core::Bytes&)> send;
            };

            /// @brief Builds the window and wires its list, painters and buttons.
            /// @param services Game services.
            /// @param screen Screen the window lives on.
            /// @param pets The player's pet team.
            /// @param inventory The player's bag.
            /// @param messages Player message sink.
            /// @param talk Dialog used for the confirmation question.
            /// @param hooks Callbacks into the rest of the game.
            PotentialReturnWindow(
                GameServices& services,
                ui::Screen& screen,
                const PetTeam& pets,
                const Inventory& inventory,
                PlayerMessages& messages,
                TalkDialog& talk,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True while open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form for the pill used in a bag place; refused in a battle.
            /// @details Original: FPendingItemSlot and ShowCtrl.
            /// @param bagSlot Bag place of the pill.
            void open(int bagSlot);

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

            /// @brief Reloads the pet list when the pets change under an open form.
            /// @details Original: LoadPotentialReturnPetList.
            void refreshPets();

            /// @brief The pet in a place changed; the chosen row is chosen again when it is that pet's.
            /// @details Original: RecvPotentialResult, RecvPotentialReturnResult (23/216).
            /// @param petPlace Pet place that changed.
            void petChanged(int petPlace);

        private:
            /// @brief Fills the list with a row for each pet that is no soldier.
            /// @details Original: LoadPotentialReturnPetList.
            void loadList();

            /// @brief Chooses a row: the pet's figure, abilities and potential.
            /// @details Original: SelectPotentialReturnTarget.
            /// @param row Row index.
            void selectTarget(int row);

            /// @brief Gives the pet place of a row.
            /// @param row Row index.
            /// @return The pet place, 0 when no such row.
            int petOf(int row) const;

            /// @brief Checks the refusals shared by confirm and send; tells the player why on failure.
            /// @details Original: shared by ConfirmPotentialReturn and SendPotentialReturn.
            /// @param place Pet place.
            /// @return True when the pill may be used on the pet.
            bool allowed(int place);

            /// @brief Asks the player to confirm using the pill.
            /// @details Original: ConfirmPotentialReturn.
            void confirm();

            /// @brief Sends the pill's place and the pet's place.
            /// @details Original: SendPotentialReturn (23/130).
            void send();

            /// @brief Paints the pets' figures in the rows.
            /// @param canvas Target canvas.
            /// @param at Window origin.
            /// @param nowMs Current time in milliseconds.
            void paintFigures(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the pill, rows, chosen pet, wheel and abilities.
            /// @details Original: RenderWithChildren, after the controls.
            /// @param canvas Target canvas.
            /// @param at Window origin.
            /// @param nowMs Current time in milliseconds.
            void paintForm(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the name, level and potential of each row.
            /// @details Original: DrawPotentialReturnPetRows.
            /// @param canvas Target canvas.
            /// @param at Window origin.
            void paintRows(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints a mark and a flame on the wheel for every potential level.
            /// @details Original: DrawPotentialReturnFires.
            /// @param canvas Target canvas.
            /// @param at Window origin.
            void paintFires(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints each ability and what the pill takes off it.
            /// @details Original: DrawPotentialReturnBonuses.
            /// @param canvas Target canvas.
            /// @param at Window origin.
            void paintBonuses(gfx::Canvas& canvas, core::Point at);

            static constexpr int kWheel = 12; // potential levels / marks round the wheel

            GameServices& m_services;
            ui::Screen& m_screen;
            const PetTeam& m_pets;
            const Inventory& m_inventory;
            PlayerMessages& m_messages;
            TalkDialog& m_talk;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::BlockList& m_targets;
            ui::IconGrid& m_item;
            const ui::Spot& m_rowsSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_haveSpot;
            const ui::Spot& m_takenSpot;
            const ui::Spot& m_previewSpot;
            std::vector<PetInfoPainter> m_infos;
            FigureAnimation m_animation;
            std::minstd_rand m_random;
            std::vector<int> m_rowPets;
            std::array<int, kWheel + 1> m_fireFrames{};
            std::array<int, 5> m_values{};
            int m_potential = 0;
            int m_targetRow = 0;
            int m_previewRise = 0;
            int m_itemSlot = 0;
            int m_pendingSlot = 0;
    };

}

#endif
