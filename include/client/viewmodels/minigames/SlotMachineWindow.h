#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_SLOT_MACHINE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_SLOT_MACHINE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/minigames/MachineTables.h"
#include "client/subsystems/minigames/SlotMachine.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <array>
#include <cstdint>
#include <functional>
#include <memory>

namespace client {

    /// @brief The slot machine's screen: machine picture with ten prizes and hints, three reels, sliding banner, prize flash, and the rocker, Lucky Star and Exit buttons over a SlotMachine.
    /// @details Original: SlotMachineWindow.xml, DrawSlotmachGame. The machine kind is the one started (8, 10 or 19).
    class SlotMachineWindow {
        public:
            /// @brief Loads the form and wires the painter and the Play, Lucky Star and Exit buttons.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param hooks Game hooks handed on to each SlotMachine.
            SlotMachineWindow(
                GameServices& services,
                ui::Screen& screen,
                GameHooks hooks
            );

            /// @brief Tells whether the machine form is showing.
            /// @return True when the form is visible.
            bool isOpen() const { return m_form.isVisible(); }

            /// @brief Gives the current machine; only valid after open().
            /// @return The SlotMachine.
            SlotMachine& machine() { return *m_machine; }

            /// @brief Creates the machine of this kind and shows its picture under the instructions.
            /// @details Original: RecvSportStart: TSlotmachGame.Create and TTrunEggForm.SetupPrizeButtons.
            /// @param kind Machine kind (8, 10 or 19).
            /// @param nowMs Current time in milliseconds.
            void open(int kind, std::uint64_t nowMs);

            /// @brief Begins play after the instructions.
            void begin();

            /// @brief Hides the form and drops the machine.
            void close();

            /// @brief Per-frame update: reel clock work, music and the buttons' visibility.
            /// @details Original: UpdateSlotmachGame: the forms show once the machine is active.
            /// @param nowMs Current time in milliseconds.
            void update(std::uint64_t nowMs);

        private:
            /// @brief Draws the machine, prizes, reels, flash, Lucky Star icon and sliding banner.
            /// @details Original: DrawSlotmachGame.
            /// @param canvas Canvas to draw on.
            /// @param at Position of the drawing area (unused).
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Places the prize areas and sets their hints.
            /// @details Original: TTrunEggForm.SetupPrizeButtons.
            void setUpPrizes();

            /// @brief Finds the picture of an item's icon.
            /// @param itemId Item id.
            /// @return The icon picture, or an invalid id when there is none.
            gfx::PictureId iconOf(int itemId) const;

            GameServices& m_services;
            GameHooks m_hooks;
            std::unique_ptr<SlotMachine> m_machine;
            ui::Element& m_form;
            std::array<std::reference_wrapper<ui::DrawingArea>, MachineTables::kPrizes> m_prizes;
            ui::ImageButton& m_play;
            ui::ImageButton& m_luckyStar;
            ui::ImageButton& m_exit;
            const ui::Spot& m_backgroundSpot;
            const ui::Spot& m_bannerSpot;
            const ui::Spot& m_flashSpot;
            const ui::Spot& m_exitSpot;
            const ui::Spot& m_starIconSpot;
            std::uint64_t m_nowMs = 0;
    };

}

#endif
