#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_BOOK_SHELVES_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_BOOK_SHELVES_H

#include <array>

namespace client {

    /// @brief The blueprints in the Bookcase and the Desk of the player's house, ten places each (an item id or 0).
    /// @details Original: Player.FBookCaseItems and Player.FBookDeskItems.
    class BookShelves {
        public:
            static constexpr int kPlaces = 10; // book places per bookcase / desk

            /// @brief A place now holds this item; a place out of 1..10 is ignored.
            /// @details Original: HandleBookCaseSlotPacket and HandleBookDeskSlotPacket.
            /// @param desk True for the Desk, false for the Bookcase.
            /// @param place Place (1..10).
            /// @param itemId Item id (0: none).
            void set(bool desk, int place, int itemId);

            /// @brief The item in a place.
            /// @param desk True for the Desk, false for the Bookcase.
            /// @param place Place (1..10).
            /// @return Item id, 0 for none or a place out of range.
            int item(bool desk, int place) const;

            /// @brief The first empty place.
            /// @details Original: FindEmptyBookCaseSlot and FindEmptyBookDeskSlot.
            /// @param desk True for the Desk, false for the Bookcase.
            /// @return Place number, 0 when all are full.
            int firstEmpty(bool desk) const;

            /// @brief The first place that holds the item.
            /// @param desk True for the Desk, false for the Bookcase.
            /// @param itemId Item id to look for.
            /// @return Place number, 0 for none.
            int placeOf(bool desk, int itemId) const;

        private:
            std::array<int, kPlaces> m_case{};
            std::array<int, kPlaces> m_desk{};
    };

}

#endif
