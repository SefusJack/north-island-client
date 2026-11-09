#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_ROLE_CAROUSEL_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_ROLE_CAROUSEL_H

#include "ui/ImageButton.h"
#include "ui/Screen.h"

#include <cstdint>
#include <functional>
#include <vector>

namespace client {

    /// @brief Scrolling row of role pictures on the creation wizard's first page; pictures wrap round and only those between the arrows show.
    /// @details Original: CreateRole1..14 in CreateCharacterForm.xml.
    class RoleCarousel {
        public:
            /// @brief Finds the role pictures and places them on their spots.
            /// @param screen Screen holding the wizard layout.
            explicit RoleCarousel(
                ui::Screen& screen
            );

            /// @brief Scrolls to the previous role.
            void back();

            /// @brief Scrolls to the next role.
            void forward();

            /// @brief Moves the pictures; call every frame while the page shows.
            /// @param nowMs Current time in milliseconds.
            void update(std::uint64_t nowMs);

        private:
            /// @brief Scroll direction.
            enum class Direction { None, Back, Forward };

            /// @brief Works out how far to scroll to bring the current spot's picture to its stop.
            /// @param direction Back or Forward.
            void startScroll(Direction direction);

            /// @brief Gives the picture standing on a spot.
            /// @param index Spot number, 1-based, left to right.
            /// @return The role picture button.
            ui::ImageButton& pictureAt(int index) const;

            std::vector<std::reference_wrapper<ui::ImageButton>> m_pictures;
            int m_index = 1;
            int m_distance = 0;
            std::uint64_t m_scrollStartMs = 0;
            bool m_scrollStarted = false;
            int m_wrapWidth = 0;
            int m_visibleLeft = 0;
            int m_visibleRight = 0;
    };

}

#endif
