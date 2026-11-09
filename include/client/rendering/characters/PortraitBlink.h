#ifndef NORTH_ISLAND_CLIENT_RENDERING_CHARACTERS_PORTRAIT_BLINK_H
#define NORTH_ISLAND_CLIENT_RENDERING_CHARACTERS_PORTRAIT_BLINK_H

#include <cstdint>
#include <random>

namespace client {

    /// @brief Times when a portrait blinks: its eyes close (frame 1) for 50 ms every 6 to 8 seconds.
    class PortraitBlink {
        public:
            /// @brief Gives the portrait frame to show now.
            /// @param nowMs Current time in ms.
            /// @return 1 while the eyes are closed, else 0.
            int frame(std::uint64_t nowMs);

            /// @brief Restarts the blink timing.
            void restart();

        private:
            /// @brief Picks a random time the eyes stay open.
            /// @return Milliseconds, 6000 to 7999.
            std::uint64_t openTimeMs();

            bool m_closed = false;
            std::uint64_t m_changeMs = 0;
            std::mt19937 m_random{std::random_device{}()};
    };

}

#endif
