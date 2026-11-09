#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_POTENTIAL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_POTENTIAL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/FigureAnimation.h"
#include "client/rendering/painters/PetInfoPainter.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/models/pets/PetTeam.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/viewmodels/chat/TalkDialog.h"
#include "client/models/world/MapCharacter.h"
#include "ui/BlockList.h"
#include "ui/IconGrid.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <random>
#include <vector>

namespace client {

    /// @brief The potential form (PotentialWindow.xml, TPotentialForm): pick the player or a pet and use a potential pill on it.
    /// @details Original: pills Potential Pill 34269, 34289 and Super 34350 by mode; the result arrives as 23/213 and lights the next mark.
    class PotentialWindow {
        public:
            /// @brief Callbacks into the rest of the client.
            struct Hooks {
                std::function<bool()> fighting;
                std::function<void()> closeItemWindow;
                std::function<void(const core::Bytes&)> send;
            };

            /// @brief Loads PotentialWindow.xml and wires the form.
            /// @param services Game services (layout, pictures, items, characters).
            /// @param screen Screen the window is loaded into.
            /// @param status Player status (potential, stats, level).
            /// @param pets Pet team.
            /// @param inventory Bag the pills are taken from.
            /// @param marks Bag slot marks (locks, availability).
            /// @param player The player's map character (name, look).
            /// @param messages Short on-screen message line.
            /// @param talk Dialog used for the confirmation questions.
            /// @param hooks Callbacks into the client.
            PotentialWindow(
                GameServices& services,
                ui::Screen& screen,
                const PlayerStatus& status,
                const PetTeam& pets,
                const Inventory& inventory,
                const BagMarks& marks,
                const MapCharacter& player,
                PlayerMessages& messages,
                TalkDialog& talk,
                Hooks hooks
            );

            /// @brief Whether the form is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows or hides the form; the mode stays what it was.
            /// @details Original: ToggleFormShown (the Potential button).
            void toggle();

            /// @brief Opens the form for a pill use.
            /// @param mode 0 Potential Pill 34269, 1 34289, 2 Super 34350.
            /// @details Original: FPendingMode, then ShowCtrl.
            void open(int mode);

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

            /// @brief Reloads the target list when the pets change under an open form.
            /// @details Original: LoadPotentialPetList.
            void refreshPets();

            /// @brief Re-selects the row when the pet in a place changed under an open form (the Return form's result).
            /// @param petPlace Pet place that changed.
            void petChanged(int petPlace);

            /// @brief Applies a potential result: the player's or a pet's potential is now this (the models are already changed).
            /// @param petPlace 0 for the player, else the pet place.
            /// @param before Potential before.
            /// @param now Potential now.
            /// @details Original: RecvPotentialResult 1.
            void result(int petPlace, int before, int now);

        private:
            /// @brief Shows the form (not during a battle) and resets it.
            void show();

            /// @brief Fills the target rows: the player, then each non-soldier pet.
            void loadList();

            /// @brief Chooses a row and computes its preview and base abilities.
            /// @param row Row index (0 is the player).
            void selectTarget(int row);

            /// @brief Item id of the pill for the current mode.
            /// @return Pill item id.
            int itemId() const;

            /// @brief Counts the pills of the form's kind in the bag that are free to use, at most 999.
            /// @return Pill count.
            int pillCount() const;

            /// @brief Finds the first usable pill slot in the bag.
            /// @return Bag slot, or 0 when none.
            int firstPillSlot() const;

            /// @brief The refusals ConfirmPotential and SendPotential share (a message tells why).
            /// @param place Pet place, 0 for the player.
            /// @return False when refused.
            bool allowed(int place);

            /// @brief The pet of a row.
            /// @param row Row index.
            /// @return Pet place, 0 for the player's row.
            int petOf(int row) const;

            /// @brief Checks the target and asks for confirmation before sending.
            /// @details Original: ConfirmPotential.
            void confirm();

            /// @brief Re-checks and sends the potential request (23/126).
            /// @details Original: SendPotential.
            void send();

            /// @brief Paints the pet figures in the target rows.
            /// @param canvas Target canvas.
            /// @param at Origin of the drawing area.
            /// @param nowMs Current time in ms.
            void paintFigures(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the whole form: rows, pill count, preview figure, fires and bonuses.
            /// @param canvas Target canvas.
            /// @param at Origin of the drawing area.
            /// @param nowMs Current time in ms.
            void paintForm(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints one target row: name, level and potential/max.
            /// @param canvas Target canvas.
            /// @param at Origin of the drawing area.
            /// @param row Row index.
            /// @param name Name text.
            /// @param level Level shown at the right.
            /// @param potential Potential value shown in red.
            /// @param nameRise Vertical offset applied to a long name.
            /// @details Original: DrawPotentialPlayerRow and DrawPotentialPetRows.
            void paintRow(
                gfx::Canvas& canvas,
                core::Point at,
                int row,
                const std::string& name,
                int level,
                int potential,
                int nameRise
            );

            /// @brief Paints a mark and flame for each potential level and a small flame at the next mark.
            /// @param canvas Target canvas.
            /// @param at Origin of the drawing area.
            /// @details Original: DrawPotentialFires.
            void paintFires(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints each ability without the potential and what the potential adds.
            /// @param canvas Target canvas.
            /// @param at Origin of the drawing area.
            /// @details Original: DrawPotentialBonuses.
            void paintBonuses(gfx::Canvas& canvas, core::Point at);

            static constexpr int kWheel = 12; // potential levels / marks round the wheel

            GameServices& m_services;
            ui::Screen& m_screen;
            const PlayerStatus& m_status;
            const PetTeam& m_pets;
            const Inventory& m_inventory;
            const BagMarks& m_marks;
            const MapCharacter& m_player;
            PlayerMessages& m_messages;
            TalkDialog& m_talk;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::BlockList& m_targets;
            ui::IconGrid& m_item;
            ui::ImageButton& m_ok;
            const ui::Spot& m_rowsSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_haveSpot;
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
            int m_mode = 0;
            int m_pendingMode = 0;
            int m_itemSlot = 0;
            bool m_requestSent = false;
    };

}

#endif
