#include "protocol/VehicleLists.h"

#include "core/ByteReader.h"

namespace protocol {

    namespace {

        constexpr std::size_t kHeader = 2;
        constexpr std::size_t kVehicleSize = 16; // bytes per scene vehicle: u8 slot, i32 owner, u16 item, i32 x, i32 y
        constexpr std::size_t kRiderSize = 6; // bytes per rider: i32 player, u16 item
        constexpr std::size_t kParkedSize = 11; // bytes per parked vehicle: i32 item, u8 cell, i32 wear, u16 fuel
        constexpr int kParkCells = 4; // parking cells in a garage

        /// @brief Reads one 11-byte parked vehicle record (i32 item, u8 cell, i32 wear, u16 fuel).
        /// @param reader Reader positioned at the record.
        /// @return The parked vehicle.
        ParkedVehicle readParked(core::ByteReader& reader) {
            ParkedVehicle parked;
            parked.itemId = reader.i32();
            parked.cell = reader.u8();
            parked.wear = reader.i32();
            parked.fuel = reader.u16();
            return parked;
        }

    }

    std::vector<SceneVehicle> decodeSceneVehicles(const core::Bytes& payload) {
        std::vector<SceneVehicle> vehicles;
        core::ByteReader reader(payload, kHeader);
        while (reader.remaining() >= kVehicleSize) {
            SceneVehicle vehicle;
            vehicle.slot = reader.u8();
            vehicle.ownerId = reader.i32();
            vehicle.itemId = reader.u16();
            vehicle.x = reader.i32();
            vehicle.y = reader.i32();
            vehicle.bagSlot = reader.u8();
            vehicles.push_back(vehicle);
        }
        return vehicles;
    }

    std::vector<SceneRider> decodeSceneRiders(const core::Bytes& payload) {
        std::vector<SceneRider> riders;
        core::ByteReader reader(payload, kHeader);
        while (reader.remaining() >= kRiderSize) {
            SceneRider rider;
            rider.playerId = reader.i32();
            rider.itemId = reader.u16();
            riders.push_back(rider);
        }
        return riders;
    }

    std::vector<ParkedVehicle> decodeParkList(const core::Bytes& payload) {
        std::vector<ParkedVehicle> cells;
        core::ByteReader reader(payload, kHeader);
        for (int cell = 1; cell <= kParkCells && reader.remaining() >= kParkedSize; ++cell) {
            ParkedVehicle parked = readParked(reader);
            parked.cell = cell;
            cells.push_back(parked);
        }
        return cells;
    }

    std::optional<ParkedVehicle> decodeParkSlot(const core::Bytes& payload) {
        core::ByteReader reader(payload, kHeader);
        if (reader.remaining() < kParkedSize) {
            return std::nullopt;
        }
        return readParked(reader);
    }

}
