#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_MUSIC_CENTER_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_MUSIC_CENTER_WINDOW_H

#include "client/services/GameServices.h"
#include "client/data/MusicSongs.h"
#include "core/Bytes.h"
#include "ui/ImageButton.h"
#include "ui/ImageView.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief The Music Center form of a house: the owner picks the house's song from the sound folder's files, ten to a page.
    /// @details Original: MusicCenterWindow.xml, RE_MigrantManage_Tools.pas TMusicCenter, furniture 39115; Play, Pause, the arrows and a double click on the list send 62/61; 62/59 tells everyone in the house what plays.
    class MusicCenterWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
                std::function<bool()> inHouse;
                std::function<std::int32_t()> ownId;
            };

            static constexpr int kMusicCenterItem = 39115; // item 39115: Modular Stereos (house furniture, type 30)

            /// @brief Loads the form, reads the song list and wires its events.
            /// @param services Game services (layouts, sound data, music player).
            /// @param screen Screen the form is loaded into.
            /// @param hooks Owner callbacks.
            MusicCenterWindow(
                GameServices& services,
                ui::Screen& screen,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form with the playing song's page.
            /// @details Original: ShowCtrl (ValidateMusicSelection, then the playing mark).
            void open();

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

            /// @brief Handles the house music packet: the house's song (sub 1) or the request for the saved choice (sub 2).
            /// @param payload Packet payload.
            /// @param nowMs Current time in ms.
            /// @details Original: RecvMusicCenterBgm, 62/59; sub 1 is the house's song (taken once a second at most), sub 2 asks for the saved choice and is answered with 62/61.
            void receive(const core::Bytes& payload, std::uint64_t nowMs);

            /// @brief Stores the current time for painting and click spacing.
            /// @param nowMs Current time in ms.
            void update(std::uint64_t nowMs) { m_nowMs = nowMs; }

        private:
            /// @brief Draws the hovered row and the page's song names; rescans the sound folder while the mouse is over the form.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @details Original: DrawControl and HandleMouseMove (sound folder read again every two seconds).
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Selects the song of the row under the mouse.
            /// @details Original: HandleMouseUp.
            void rowUp();

            /// @brief Lets a click through once in 1500 ms and not while the server's answer is awaited.
            /// @return True when the click may act.
            bool takeClick();

            /// @brief Play button: plays the selected song.
            /// @details Original: MusicPlayClick (runs on the mouse press).
            void playClick();

            /// @brief Stop button: stops the house music.
            /// @details Original: MusicStopClick.
            void stopClick();

            /// @brief Next arrow: selects and plays the next song.
            /// @details Original: MusicNextClick.
            void nextClick();

            /// @brief Previous arrow: selects and plays the previous song.
            /// @details Original: MusicPrevClick.
            void previousClick();

            /// @brief Sends the selected song as the house music and keeps the choice in the user file.
            /// @details Original: SetHouseBgm (62/61; choice kept in User/id+MC.Txt).
            void setHouseBgm();

            /// @brief Finds the list row under the mouse.
            /// @param mouse Mouse position on screen.
            /// @return Row 1..10 of the page, or 0 for none.
            int rowAt(core::Point mouse) const;

            GameServices& m_services;
            ui::Screen& m_screen;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::ImageView& m_playIcon;
            ui::ImageButton& m_playButton;
            const ui::Spot& m_rowSpot;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_playingSpot;
            MusicSongs m_songs;
            int m_page = 0;
            int m_hovered = 0;
            int m_playing = 0;
            bool m_waiting = false;
            bool m_iconShown = false;
            bool m_playHeld = false;
            int m_houseSong = 0;
            std::int64_t m_lastClick = -1500; // time of the last click, ms; -1500 = one click gap ago, so the first click passes
            std::int64_t m_lastScan = 0;
            std::int64_t m_lastPlay = -1000; // time of the last play, ms; -1000 = one play gap ago, so the first play passes
            std::int64_t m_nowMs = 0;
    };

}

#endif
