#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_BEFORE_CAREER_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_BEFORE_CAREER_WINDOW_H

#include "client/services/GameServices.h"
#include "client/viewmodels/character/CareerHooks.h"
#include "client/models/character/CareerModel.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief First class form: a player with equal strength and intelligence picks the soldier or the mage line.
    /// @details Original: BeforeCareerWindow.xml, TBeforeCareerForm; OK goes on to the class form (CareerWindow) of that line.
    class BeforeCareerWindow {
        public:
            /// @brief Builds the window and wires its painter and buttons.
            /// @param services Game services.
            /// @param screen Screen the window lives on.
            /// @param hooks Callbacks into the scene.
            BeforeCareerWindow(
                GameServices& services,
                ui::Screen& screen,
                CareerHooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True while open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form with the two model figures and nothing chosen.
            /// @details Original: ResetBeforeCareerSelection, LoadBeforeCareerModels, SetBeforeCareerMode, ShowCtrl.
            /// @param changeMode Non-zero when the class is being changed (Transfer Mirror).
            void open(int changeMode);

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Paints the models, the chosen frame and the description.
            /// @details Original: DrawBeforeCareerForm.
            /// @param canvas Target canvas.
            /// @param at Window origin.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Finds the model slot under a point of the form.
            /// @details Original: BeforeCareerModelAtPoint.
            /// @param local Point relative to the window.
            /// @return Slot 1..2, or 0 for none.
            int slotAt(core::Point local) const;

            /// @brief Left press: chooses or unchooses the model under the mouse.
            /// @param args Mouse event data.
            void pressed(const ui::EventArgs& args);

            /// @brief Right press: unchooses the chosen model.
            /// @param args Mouse event data.
            void rightPressed(const ui::EventArgs& args);

            /// @brief Checks the choice and goes on to the class form of the chosen line.
            /// @details Original: ConfirmBeforeCareer.
            void confirm();

            /// @brief Cancels the form; the event goes on.
            /// @details Original: CancelBeforeCareer.
            void cancel();

            /// @brief Draws one model figure.
            /// @param canvas Target canvas.
            /// @param model The model.
            /// @param feet Position of its feet.
            /// @param nowMs Current time in milliseconds.
            void drawModel(gfx::Canvas& canvas, CareerModel& model, core::Point feet, std::uint64_t nowMs);

            GameServices& m_services;
            ui::Screen& m_screen;
            CareerHooks m_hooks;
            ui::Window& m_window;
            const ui::Spot& m_chosenSpot;
            const ui::Spot& m_selfSpot;
            const ui::Spot& m_descriptionSpot;
            CareerModel m_self;
            std::array<CareerModel, 2> m_models;
            int m_changeMode = 0;
            int m_hover = 0;
            int m_selected = 0;
    };

}

#endif
