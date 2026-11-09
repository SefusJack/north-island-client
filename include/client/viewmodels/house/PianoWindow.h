#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_PIANO_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_PIANO_WINDOW_H

#include "client/services/GameServices.h"
#include "core/Point.h"
#include "ui/Key.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <array>
#include <functional>

namespace client {

    /// @brief The Piano's form: seven white keys played with the mouse or the keys A S D F G H J.
    /// @details Original: PianoWindow.xml, CJ_Menu.pas TPianoForm; each note sounds at once (PlayPianoNote) and goes to the server (62/57), which lets the others in the house hear it; mouse play needs the player seated at the piano.
    class PianoWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<bool()> seated;
                std::function<void()> sceneMusic;
            };

            static constexpr int kNotes = 7;

            /// @brief Loads the form, centres it and wires its events.
            /// @param services Game services (layouts, pictures, sound, connection).
            /// @param screen Screen the form is loaded into.
            /// @param hooks Owner callbacks.
            PianoWindow(
                GameServices& services,
                ui::Screen& screen,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form.
            /// @details Original: ShowCtrl.
            void show();

            /// @brief Hides the form without restoring the scene music.
            /// @details Original: HideCtrl.
            void hide() { m_window.setVisible(false); }

            /// @brief Closes the form and restores the scene music.
            /// @details Original: ClosePianoForm.
            void close();

            /// @brief Plays a note locally.
            /// @param note Note 1..7.
            /// @details Original: PlayPianoNote (sound WAV9904 to WAV9910).
            void playNote(int note) const;

            /// @brief Strikes the note of a keyboard key of the row A S D F G H J when no key is held.
            /// @param key The key event.
            /// @return True when it was a piano key.
            /// @details Original: PianoKeyPressed.
            bool keyDown(const ui::KeyEvent& key);

            /// @brief Clears the pressed state once the keys are let go (once a frame).
            /// @details Original: FKeyPressed.
            void update();

        private:
            /// @brief Finds the white key note under a point of the form.
            /// @param local Point relative to the form.
            /// @return Note 1..7, or 0 for none.
            /// @details Original: PianoMouseKey.
            static int noteAt(core::Point local);

            /// @brief Handles a press on the form: while seated, strikes the key under the mouse.
            /// @param local Point relative to the form.
            /// @details Original: TPianoForm.HandleMouseDown.
            void mouseDown(core::Point local);

            /// @brief Sounds a note, draws it pressed and sends it to the server (62/57).
            /// @param note Note 1..7.
            void strike(int note);

            /// @brief Draws the picture of the key that is down.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @details Original: DrawPianoKey.
            void paintKey(gfx::Canvas& canvas, core::Point at) const;

            GameServices& m_services;
            ui::Screen& m_screen;
            Hooks m_hooks;
            ui::Window& m_window;
            std::array<gfx::PictureId, kNotes> m_keyPictures{};
            bool m_pressed = false;
            int m_note = 0;
            bool m_mouseHeld = false;
    };

}

#endif
