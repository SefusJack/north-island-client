#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_CHARACTER_SELECT_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_CHARACTER_SELECT_FORM_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/models/character/FigureAnimation.h"
#include "client/rendering/characters/PortraitBlink.h"
#include "protocol/CharacterSummary.h"
#include "ui/DigitText.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/ImageView.h"
#include "ui/Label.h"
#include "ui/ProgressBar.h"
#include "ui/Screen.h"

#include <functional>
#include <optional>
#include <vector>

namespace client {

    /// @brief The character select screen: the account's two character slots; enter, create or delete.
    /// @details Original: CharacterSelectForm.xml.
    class CharacterSelectForm {
        public:
            /// @brief Actions the form reports to its owner.
            struct Callbacks {
                std::function<void(int slot)> enter;
                std::function<void(int slot)> create;
                std::function<void(int slot)> remove;
                std::function<void()> previous;
            };

            /// @brief Builds the form and wires its controls.
            /// @param services Shared game services.
            /// @param screen Screen the form lives on.
            /// @param callbacks Owner callbacks.
            CharacterSelectForm(
                GameServices& services,
                ui::Screen& screen,
                Callbacks callbacks
            );

            /// @brief Shows the characters of the session.
            void show();

            /// @brief Hides the form.
            void hide() { m_form.setVisible(false); }

            /// @brief Whether the form is visible.
            /// @return True when shown.
            bool isVisible() const { return m_form.isVisible(); }

            /// @brief The direction the slot's character faces (8..15; the arrows turn it).
            /// @param slot Slot number 1..2.
            /// @return The direction.
            int direction(int slot) const { return m_slots[static_cast<std::size_t>(slot - 1)].direction; }

        private:
            /// @brief The controls and state of one character slot.
            struct Slot {
                ui::Element& empty;
                ui::Element& info;
                ui::Element& emptyIcon;
                ui::Element& details;
                ui::ImageView& element;
                ui::Label& name;
                ui::DigitText& level;
                ui::Element& career;
                ui::ImageView& job;
                ui::DigitText& hp;
                ui::DigitText& sp;
                ui::ProgressBar& expBar;
                ui::DigitText& expPercent;
                ui::DigitText& gold;
                ui::ImageButton& createOrDelete;
                ui::DrawingArea& portrait;
                ui::DrawingArea& figure;
                int direction = 11; // facing 11: default direction (directions run 8..15)
                std::optional<CharacterLook> look;
                FigureAnimation animation;
                PortraitBlink blink;
            };

            /// @brief Looks up the controls of a slot.
            /// @param screen Screen holding the controls.
            /// @param number Slot number 1..2.
            /// @return The slot.
            static Slot findSlot(ui::Screen& screen, int number);

            /// @brief Fills a slot from a character, or shows it empty.
            /// @param slot The slot.
            /// @param character The character, or none for an empty slot.
            void fillSlot(Slot& slot, const std::optional<protocol::CharacterSummary>& character);

            /// @brief Turns a slot's figure.
            /// @param slot Slot number 1..2.
            /// @param step -1 left, 1 right.
            void turn(int slot, int step);

            /// @brief Creates a character in an empty slot or deletes the one in it.
            /// @param slot Slot number 1..2.
            void createOrDelete(int slot);

            /// @brief Paints the standing figure, turned by the arrows and animated.
            /// @param slot The slot.
            /// @param canvas Target canvas.
            /// @param feet Position of the figure's feet.
            /// @param nowMs Current time in ms.
            void paintFigure(Slot& slot, gfx::Canvas& canvas, core::Point feet, std::uint64_t nowMs);

            /// @brief Paints the character's portrait.
            /// @param slot The slot.
            /// @param canvas Target canvas.
            /// @param anchor Portrait anchor position.
            /// @param nowMs Current time in ms.
            void paintPortrait(Slot& slot, gfx::Canvas& canvas, core::Point anchor, std::uint64_t nowMs);

            GameServices& m_services;
            Callbacks m_callbacks;
            ui::Element& m_form;
            std::vector<Slot> m_slots;
    };

}

#endif
