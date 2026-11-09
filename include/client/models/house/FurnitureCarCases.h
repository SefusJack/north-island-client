#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_FURNITURE_CAR_CASES_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_FURNITURE_CAR_CASES_H

#include "client/models/house/HouseUtensils.h"
#include "protocol/FurnitureCupboardItem.h"
#include "protocol/ItemInstance.h"

#include <array>

namespace client {

    /// @brief The vehicles parked in the Garages of a house, up to four, each in a cell (1..4) of a piece that the piece's content slots point to.
    /// @details Original: DZ_Thing.pas TUtensilManage.FCarCaseItems with what HandleUtensilPacket's 62/47 and 62/48 do to them. A vehicle
    /// that arrives before its piece (the server sends the cars as the house is entered, before its furniture list) stays unlinked.
    class FurnitureCarCases {
        public:
            static constexpr int kCars = 4; // car places per case
            static constexpr int kGarageItem = 39069; // item 39069: Garage
            static constexpr int kShowPlatformItem = 39001; // item 39001: Show Platform

            /// @brief Creates the car list over the house utensils.
            /// @param utensils The house's furniture pieces.
            explicit FurnitureCarCases(
                HouseUtensils& utensils
            )
                : m_utensils(utensils) {}

            /// @brief Forgets all parked vehicles.
            void clear();

            /// @brief A vehicle parked in a cell (a piece that is not standing in a room keeps it unlinked).
            /// @details Original: server packet 62/47.
            /// @param stored The vehicle, its piece and cell.
            void put(const protocol::FurnitureCupboardItem& stored);

            /// @brief The vehicle of that number left its cell.
            /// @details Original: server packet 62/48.
            /// @param furnIndex Furniture piece index.
            /// @param cell Garage cell.
            /// @param id Number the vehicle is kept under.
            /// @return False when the piece is no Garage standing in the house.
            bool remove(int furnIndex, int cell, int id);

            /// @brief What is kept under a number, whichever cell it is in (the form's checks read it by the cell's number).
            /// @details Original: FCarCaseItems[id].Item.
            /// @param id Number (1..4).
            /// @return The vehicle, empty for none.
            const protocol::ItemInstance& keptUnder(int id) const;

            /// @brief The first number nothing is kept under.
            /// @details Original: PutCarCaseItem.
            /// @return The number, 0 for none.
            int emptyNumber() const;

        private:
            HouseUtensils& m_utensils;
            std::array<protocol::ItemInstance, kCars + 1> m_kept{};
    };

}

#endif
