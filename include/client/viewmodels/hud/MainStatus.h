#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_MAIN_STATUS_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_MAIN_STATUS_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/rendering/characters/PortraitBlink.h"
#include "client/rendering/hud/Gauge.h"
#include "client/rendering/painters/PetInfoPainter.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/character/VehicleGauge.h"
#include "client/models/pets/PetTeam.h"
#include "ui/DigitText.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageView.h"
#include "ui/ProgressBar.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <functional>
#include <optional>

namespace client {

    /// @brief The status panel at the top left of the world: portrait, level, element, job, HP/SP gauges, experience bar, gold and the fighting pet.
    /// @details Original: MainStatus.xml, TSe_MainStatus.
    class MainStatus {
        public:
            /// @brief Loads the panel layout and installs the painters of its drawing areas.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param status Player status shown.
            /// @param look Character look drawn in the portrait.
            /// @param pets Pet team (the active pet is shown).
            MainStatus(
                GameServices& services,
                ui::Screen& screen,
                const PlayerStatus& status,
                const CharacterLook& look,
                const PetTeam& pets
            );

            /// @brief Shows the panel.
            void show() { m_panel.setVisible(true); }

            /// @brief Hides the panel.
            void hide() { m_panel.setVisible(false); }

            /// @brief Slides the panel out to the left in half a second, then hides it and calls onGone (the battle's panel comes).
            /// @details Original: SetMainStatusSlideState 2.
            /// @param nowMs Current time in milliseconds.
            /// @param onGone Called when the slide is finished.
            void slideOut(std::uint64_t nowMs, std::function<void()> onGone);

            /// @brief Slides the panel back in.
            /// @details Original: SetMainStatusSlideState 1.
            /// @param nowMs Current time in milliseconds.
            void slideIn(std::uint64_t nowMs);

            /// @brief Advances the slide animation.
            /// @details Original: UpdateMainStatusSlide.
            /// @param nowMs Current time in milliseconds.
            void update(std::uint64_t nowMs);

            /// @brief Shows the status as it is now.
            void refresh();

            /// @brief Sets the source of the vehicle ridden, when the player is not a passenger.
            /// @param gauge Returns the vehicle gauge, or nullopt when none.
            void setVehicleGauge(std::function<std::optional<VehicleGauge>()> gauge) { m_vehicleGauge = std::move(gauge); }

        private:
            /// @brief Moves the panel horizontally from its place.
            /// @param x Horizontal offset from the panel's place.
            void placeAt(int x);

            /// @brief Paints the fighting pet's info (its bar hints are drawn by paintHints, over the player's gauges).
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @param nowMs Current time in milliseconds.
            void paintPet(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the vehicle ridden with its rails, wear and fuel.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintVehicle(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the hints over the whole panel: the pet's HP, SP and experience bars, the HP or SP gauge while the mouse is on a pixel of its picture, and the experience bar.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintHints(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const PlayerStatus& m_status;
            const CharacterLook& m_look;
            const PetTeam& m_pets;
            ui::Element& m_panel;
            ui::ImageView& m_element;
            ui::ImageView& m_job;
            ui::DigitText& m_level;
            ui::DigitText& m_hp;
            ui::DigitText& m_sp;
            ui::ProgressBar& m_expBar;
            ui::DigitText& m_expPercent;
            ui::DigitText& m_gold;
            ui::DrawingArea& m_hpArea;
            ui::DrawingArea& m_spArea;
            ui::DrawingArea& m_petArea;
            const ui::Spot& m_petPartsSpot;
            Gauge m_hpGauge;
            Gauge m_spGauge;
            PortraitBlink m_blink;
            PetInfoPainter m_petInfo;
            ui::Element& m_recovery;

            /// @brief State of the panel's slide animation.
            enum class Slide { None, Out, In };
            Slide m_slide = Slide::None;
            std::uint64_t m_slideMs = 0;
            std::function<void()> m_onGone;
            std::function<std::optional<VehicleGauge>()> m_vehicleGauge;
    };

}

#endif
