#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_SCREEN_MODE_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MOVIE_MOVIE_SCREEN_MODE_H

namespace client {

    /// @brief The screen mode (FScreenMode) and frozen flag (FMovieFrozen) of the movie.
    /// @details Modes: 1 an event or queued movie, 3 the story movie of 23/151, 4 an event movie whose last picture stays; the movie branch runs in 1, 3 and 4.
    /// A mode 4 movie ends in mode 2 with FMovieFrozen set until the server's next event message (DZ_Connect.pas, 20/x).
    class MovieScreenMode {
        public:
            static constexpr int kEvent = 1;
            static constexpr int kStory = 3;
            static constexpr int kHeld = 4;

            /// @brief Tells whether the movie branch is on (FScreenMode in 1, 3, 4).
            /// @return True while a movie mode is set.
            bool playing() const { return m_mode == kEvent || m_mode == kStory || m_mode == kHeld; }

            /// @brief Tells whether the last picture stays on the screen (FScreenMode 2 and FMovieFrozen set).
            /// @return True when frozen.
            bool frozen() const { return m_frozen; }

            /// @brief Gives the screen mode.
            /// @return FScreenMode.
            int mode() const { return m_mode; }

            /// @brief Sets the mode from an event result's option: 1 and 3 mode 1, 2 and 4 mode 4, any other leaves it.
            /// @details Original: BK_EventManage.pas, result kind 5.
            /// @param option Event option.
            void eventOption(int option);

            /// @brief Sets the screen mode.
            /// @param mode New mode.
            void set(int mode) { m_mode = mode; }

            /// @brief Leaves the movie modes when a movie ends: mode 4 becomes 2 (frozen), any other 0.
            /// @details Original: UpdateMovie's end; the frame loop then freezes a mode 2 (DZ_Game.pas, TForm1.DXTimer1Timer).
            /// @return True when the screen freezes (the frame loop copies the picture then).
            bool movieEnded();

            /// @brief Lets the frozen picture go on a server 20/x message (FMovieFrozen := 0).
            void serverMessage() { m_frozen = false; }

            /// @brief Goes back to mode 0 and unfreezes (20/8 and FinishMovie).
            void reset() {
                m_mode = 0;
                m_frozen = false;
            }

        private:
            int m_mode = 0;
            bool m_frozen = false;
    };

}

#endif
