#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_INTERIOR_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_INTERIOR_H

#include "client/models/house/Doorplates.h"
#include "client/models/house/HomeFurniture.h"
#include "client/models/house/FurnitureBeds.h"
#include "client/models/house/FurnitureCarCases.h"
#include "client/models/house/FurnitureCupboards.h"
#include "client/models/house/FurnitureExhibit.h"
#include "client/models/house/FurnitureStalls.h"
#include "client/models/house/HousePillars.h"
#include "client/models/house/HouseTraining.h"
#include "client/models/house/HouseSurfaces.h"
#include "client/models/house/HouseUtensils.h"
#include "client/models/house/StallCaseChange.h"
#include "core/Bytes.h"
#include "formats/ItemTable.h"
#include "gfx/PictureLibrary.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief The inside of the house the player is in: its furniture and how it is drawn, the pillars, the floor and wall items, and what 62/x, 65/11, 65/12 and 65/13 do to them.
    /// @details Original: DZ_Thing.pas TUtensilManage and TPillarManage with RE_Furniture.pas TRE_FurnitureMng.
    class HouseInterior {
        public:
            static constexpr int kFloorRooms = 5;

            /// @brief Callbacks into the rest of the client.
            struct Hooks {
                std::function<void(const std::string&)> notice;
                std::function<void(const std::string&)> sound;
                std::function<void(int, bool)> markBag;
                std::function<void(int, int)> reclaimed;
                std::function<std::int32_t()> ownId;
                std::function<void(int, std::int32_t)> priceChanged;
                std::function<void(const StallCaseChange&)> caseChanged;
                std::function<void(const protocol::PetRecord&, int)> petReturned;
                std::function<void()> petSlept;
                std::function<void(int)> garageOpened;
                std::function<void(const protocol::FurnitureCupboardItem&)> carParked;
                std::function<void(int)> carLeft;
                std::function<void(int, bool)> platformOpened;
                std::function<void(bool)> exhibitChosen;
            };

            /// @brief Creates an empty house interior.
            /// @param pictures Picture library.
            /// @param items Item table.
            /// @param ownId The player's own id (the house is his until a list names the owner).
            /// @param hooks Callbacks into the rest of the client.
            HouseInterior(
                gfx::PictureLibrary& pictures,
                const formats::ItemTable& items,
                std::int32_t ownId,
                Hooks hooks
            );

            /// @brief The furniture of the house.
            /// @return The furniture.
            HomeFurniture& furniture() { return m_furniture; }

            /// @brief The furniture of the house, read-only.
            /// @return The furniture.
            const HomeFurniture& furniture() const { return m_furniture; }

            /// @brief The drawn pieces.
            /// @return The utensils.
            HouseUtensils& utensils() { return m_utensils; }

            /// @brief The drawn pieces, read-only.
            /// @return The utensils.
            const HouseUtensils& utensils() const { return m_utensils; }

            /// @brief The pillars.
            /// @return The pillars.
            HousePillars& pillars() { return m_pillars; }

            /// @brief The pillars, read-only.
            /// @return The pillars.
            const HousePillars& pillars() const { return m_pillars; }

            /// @brief The shelf, vending, case and egg machine items.
            /// @return The stalls.
            FurnitureStalls& stalls() { return m_stalls; }

            /// @brief The shelf, vending, case and egg machine items, read-only.
            /// @return The stalls.
            const FurnitureStalls& stalls() const { return m_stalls; }

            /// @brief The cupboards' contents.
            /// @return The cupboards.
            const FurnitureCupboards& cupboards() const { return m_cupboards; }

            /// @brief The Garage's parked vehicles.
            /// @return The car cases.
            const FurnitureCarCases& carCases() const { return m_carCases; }

            /// @brief The beds' sleeping pets.
            /// @return The beds.
            const FurnitureBeds& beds() const { return m_beds; }

            /// @brief The Show Platform's exhibit, read-only.
            /// @return The exhibit.
            const FurnitureExhibit& exhibit() const { return m_exhibit; }

            /// @brief The Show Platform's exhibit.
            /// @return The exhibit.
            FurnitureExhibit& exhibit() { return m_exhibit; }

            /// @brief The training machines' state, read-only.
            /// @return The training state.
            const HouseTraining& training() const { return m_training; }

            /// @brief The training machines' state.
            /// @return The training state.
            HouseTraining& training() { return m_training; }

            /// @brief The doorplates.
            /// @return The doorplates.
            const Doorplates& doorplates() const { return m_doorplates; }

            /// @brief The floor, wall and pillar pictures.
            /// @return The surfaces.
            HouseSurfaces& surfaces() { return m_surfaces; }

            /// @brief The floor, wall and pillar pictures, read-only.
            /// @return The surfaces.
            const HouseSurfaces& surfaces() const { return m_surfaces; }

            /// @brief Sets whether the map is a house and which room the player is in.
            /// @details Original: Player.FHouseRoom. A floor room (0 or 1 of a house) is the one with furniture.
            /// @param houseMap True when the map is a house.
            /// @param room The room index.
            void setPlace(bool houseMap, int room);

            /// @brief The room the player is in.
            /// @return The room index.
            int room() const { return m_room; }

            /// @brief Tells whether the player is in a floor room (room 0 or 1 of a house).
            /// @return True when in a floor room.
            bool inFloorRoom() const { return m_houseMap && m_room >= 0 && m_room <= 1; }

            /// @brief Sets the size of each floor room the last map change named.
            /// @details Original: EventTable.FloorRoomSizes.
            /// @param sizes The sizes, 1..3 each.
            void setFloorRoomSizes(const std::array<int, kFloorRooms>& sizes) { m_floorRoomSizes = sizes; }

            /// @brief Applies a floor room size message (the room's box and the size kept).
            /// @details Original: server action 65/12, HandleRoomSizePacket.
            /// @param payload The whole message payload.
            /// @return True when it is the room shown (its picture then changes with it).
            bool receiveRoomSize(const core::Bytes& payload);

            /// @brief The size of a floor room.
            /// @param floor The floor room index.
            /// @return The size, 1..3.
            int floorRoomSize(int floor) const { return m_floorRoomSizes[static_cast<std::size_t>(floor)]; }

            /// @brief Applies a furniture message of the 62 action to the house.
            /// @details Original: HandleUtensilPacket's furniture messages (62/1..5, 14, 15, 16, 17, 21, 22, 24, 25, 31..34, 41..48, 51, 52, 54, 55, 58, 60, 61, 255).
            /// @param payload The whole payload.
            /// @param nowMs Current time in milliseconds.
            void receiveFurniture(const core::Bytes& payload, std::uint64_t nowMs);

            /// @brief Applies a pillar list message.
            /// @details Original: server action 65/11.
            /// @param payload The whole payload.
            void receivePillarList(const core::Bytes& payload) { m_pillars.applyList(payload); }

            /// @brief Applies a pillar diff message.
            /// @details Original: server action 65/13.
            /// @param payload The whole payload.
            void receivePillarDiff(const core::Bytes& payload) { m_pillars.applyDiff(payload); }

        private:
            /// @brief Puts a piece into the furniture and, when it stands in a room, gives it its picture.
            /// @details Original: AddHomeUtensil.
            /// @param record The piece.
            /// @param index The piece's number.
            /// @param slot The utensil slot to use, 0 for the first free one.
            /// @param nowMs Current time in milliseconds.
            /// @return The utensil slot, 0 for none.
            int addPiece(const FurnitureRecord& record, int index, int slot, std::uint64_t nowMs);

            /// @brief Takes a piece out of the furniture and the utensils.
            /// @details Original: RemoveHomeUtensil.
            /// @param index The piece's number.
            void removePiece(int index);

            /// @brief A standing piece goes to the pick-up area (flag 0 takes it out of the house).
            /// @details Original: HandleFurnRetainPacket (62/16).
            /// @param index The piece's number.
            /// @param flag The retained flag.
            void retainPiece(int index, int flag);

            /// @brief A piece of the pick-up area now stands in the house.
            /// @details Original: HandleFurnPlacedPacket (62/17).
            /// @param payload The whole payload.
            /// @param nowMs Current time in milliseconds.
            void placeRetained(const core::Bytes& payload, std::uint64_t nowMs);

            /// @brief A bought or rented piece waits in the pick-up area.
            /// @details Original: HandleFurnDeliveredPacket (62/52).
            /// @param payload The whole payload.
            void deliverPiece(const core::Bytes& payload);

            /// @brief Reads everything again: the owner first, then the pieces.
            /// @details Original: server action 62/4.
            /// @param payload The whole payload.
            /// @param nowMs Current time in milliseconds.
            void receiveList(const core::Bytes& payload, std::uint64_t nowMs);

            const formats::ItemTable& m_items;
            Hooks m_hooks;
            HomeFurniture m_furniture;
            HouseUtensils m_utensils;
            HouseSurfaces m_surfaces;
            HousePillars m_pillars;
            FurnitureStalls m_stalls;
            FurnitureCupboards m_cupboards;
            FurnitureCarCases m_carCases;
            FurnitureBeds m_beds;
            FurnitureExhibit m_exhibit;
            HouseTraining m_training;
            Doorplates m_doorplates;
            bool m_houseMap = false;
            int m_room = 0;
            std::array<int, kFloorRooms> m_floorRoomSizes{};
    };

}

#endif
