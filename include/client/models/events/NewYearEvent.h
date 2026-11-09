#ifndef NORTH_ISLAND_CLIENT_MODELS_EVENTS_NEW_YEAR_EVENT_H
#define NORTH_ISLAND_CLIENT_MODELS_EVENTS_NEW_YEAR_EVENT_H

namespace client {

    /// @brief State of the New Year event as the server tells it (event on/off and the Lucky 7 play count).
    /// @details Original: Player.FNewYearEventOn (event notice 95 on, 96 off) and Player.FLucky7Count (27/5).
    class NewYearEvent {
        public:
            /// @brief Switches the event on or off; off resets the Lucky 7 count.
            /// @details Original: RecvEventNotice 95 (on) / 96 (off).
            /// @param on True when the event is on.
            void setOn(bool on);

            /// @brief Stores the Lucky 7 play count.
            /// @details Original: server packet 27/5.
            /// @param count Number of plays so far.
            void setLucky7Count(int count);

            /// @brief Whether the event is on.
            /// @return True while the event is on.
            bool on() const { return m_on; }

            /// @brief The Lucky 7 play count.
            /// @return Plays so far.
            int lucky7Count() const { return m_lucky7Count; }

            /// @brief Whether a Lucky 7 machine plays free (event on and count >= 7).
            /// @details Original: FNewYearEventOn <> 0 and FLucky7Count >= 7.
            /// @return True when the play is free.
            bool freePlay() const;

        private:
            bool m_on = false;
            int m_lucky7Count = 0;
    };

}

#endif
