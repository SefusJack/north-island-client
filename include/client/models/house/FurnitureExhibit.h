#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_FURNITURE_EXHIBIT_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_FURNITURE_EXHIBIT_H

namespace client {

    /// @brief The vehicle on show on the Show Platforms of the house; one vehicle is kept for the whole house, so every Show Platform draws the same one.
    /// @details Original: DZ_Thing.pas TUtensilManage.FExhibitItem's FSportPics: ExhibitPos holds the item, ExhibitItemId the Garage cell the owner took it from.
    class FurnitureExhibit {
        public:
            /// @brief Nothing is on show.
            /// @details Original: AddHomeUtensil for a Show Platform, and 62/55 with no vehicle.
            void clear() { *this = FurnitureExhibit{}; }

            /// @brief Sets the item on show.
            /// @details Original: server packets 62/55 and 23/144.
            /// @param itemId Item id (0: none).
            void setItem(int itemId) { m_itemId = itemId; }

            /// @brief Sets the Garage cell the owner took the vehicle from.
            /// @param cell Garage cell.
            void setCell(int cell) { m_cell = cell; }

            /// @brief The vehicle's item.
            /// @details Original: ExhibitPos.
            /// @return Item id, 0 for none.
            int itemId() const { return m_itemId; }

            /// @brief The owner's Garage cell of the vehicle.
            /// @details Original: ExhibitItemId.
            /// @return Garage cell, 0 for a visitor, who did not choose it.
            int cell() const { return m_cell; }

        private:
            int m_itemId = 0;
            int m_cell = 0;
    };

}

#endif
