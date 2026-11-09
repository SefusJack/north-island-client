#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_DYE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_DYE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/models/character/FigureAnimation.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>

namespace client {

    /// @brief The dye form (DyeWindow.xml): the player shown twice (portrait and figure) with the red, green and blue (0..9) of the dyed part set by arrows; OK sends the colours.
    /// @details Original: TH_Menu.pas TTH_ChangeColorForm; the part is hair, skin, eyes or clothes. OK sends the colours with the dye's place (23/38).
    class DyeWindow {
        public:
            static constexpr int kHair = 1;
            static constexpr int kSkin = 2;
            static constexpr int kEyes = 3;
            static constexpr int kClothes = 4;

            /// @brief Loads the window layout and wires its arrows, buttons and painter.
            /// @param services Game services (layouts, characters, pictures, connection).
            /// @param screen Screen the layout is loaded into.
            /// @param ownLook The player's own look, shown in the previews.
            DyeWindow(
                GameServices& services,
                ui::Screen& screen,
                const CharacterLook& ownLook
            );

            /// @brief Whether the window is showing.
            /// @return True while visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form for the dye of a bag place and its part, or closes it when open (IsChangeColorItem then ToggleChangeColorForm).
            /// @param bagSlot Bag place of the dye.
            /// @param part kHair, kSkin, kEyes or kClothes.
            void toggle(int bagSlot, int part);

            /// @brief Gets the part a dye item changes (item sub type 56, 57, 102 or 148).
            /// @param itemSubType Item sub type.
            /// @return The part number, 0 when the item is not a dye.
            static int partOf(int itemSubType);

            /// @brief Hides the window.
            void hide() { m_window.setVisible(false); }

        private:
            /// @brief Shows the window and loads the part's colour components (ShowCtrl, LoadColorComponents).
            void show();

            /// @brief Steps one colour channel up or down (IncColorComponent, DecColorComponent).
            /// @param channel Channel index (red, green or blue).
            /// @param by Step, positive or negative.
            void step(int channel, int by);

            /// @brief Puts the original colours back (RestoreOriginalColor).
            void restore();

            /// @brief Sends the chosen colours with the dye's place (23/38, SendChangeColor).
            void send();

            /// @brief Shows a channel's value in its digit display.
            /// @param channel Channel index.
            /// @param value Value 0..9.
            void setDigits(int channel, int value);

            /// @brief Paints the two previews of the player with the changed colours.
            /// @param canvas Target canvas.
            /// @param at Top-left of the paint area.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            GameServices& m_services;
            ui::Screen& m_screen;
            const CharacterLook& m_ownLook;
            ui::Window& m_window;
            const ui::Spot& m_standingSpot;
            const ui::Spot& m_profileSpot;
            const ui::Spot& m_iconSpot;
            const ui::Spot& m_barSpot;
            CharacterLook m_standing;
            CharacterLook m_profile;
            int m_part = kHair;
            int m_bagSlot = 0;
            std::array<int, 4> m_original{};
            std::array<int, 4> m_colors{};
            FigureAnimation m_animation;
    };

}

#endif
