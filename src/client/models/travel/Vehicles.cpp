#include "client/models/travel/Vehicles.h"

#include "client/data/VehicleKinds.h"

#include <utility>

namespace client {

    namespace {

        constexpr std::int32_t kLastPlayerId = 9000000; // highest id treated as a player id (player ids run up to 9000000 including the second server branch)

    }

    Vehicles::Vehicles(
        const formats::ItemTable& items,
        Hooks hooks
    )
        : m_items(items),
          m_hooks(std::move(hooks)) {}

    int Vehicles::freeSlot() const {
        for (int slot = 1; slot <= kMaxPlaced; ++slot) {
            if (m_placed.count(slot) == 0) {
                return slot;
            }
        }
        return 0;
    }

    int Vehicles::slotOf(std::int32_t ownerId, int itemId) const {
        if (ownerId <= 0 || ownerId > kLastPlayerId) {
            return 0;
        }
        for (const auto& [slot, vehicle] : m_placed) {
            if (vehicle.itemId == itemId && vehicle.ownerId == ownerId) {
                return slot;
            }
        }
        return 0;
    }

    bool Vehicles::isPlaced(std::int32_t ownerId, int itemId) const {
        const auto record = m_items.find(itemId);
        const int alias = record ? record->aliasItemId : 0;
        for (const auto& [slot, vehicle] : m_placed) {
            if (vehicle.ownerId == ownerId && (vehicle.itemId == itemId || vehicle.itemId == alias)) {
                return true;
            }
        }
        return false;
    }

    std::optional<std::reference_wrapper<Vehicle>> Vehicles::find(int slot) {
        const auto found = m_placed.find(slot);
        if (found == m_placed.end()) {
            return std::nullopt;
        }
        return std::ref(found->second);
    }

    bool Vehicles::place(
        int slot,
        std::int32_t ownerId,
        int itemId,
        core::Point position,
        int bagSlot,
        bool fly
    ) {
        if (m_count >= kMaxPlaced) {
            return false;
        }
        ++m_count;
        Vehicle vehicle;
        vehicle.slot = slot;
        vehicle.ownerId = ownerId;
        vehicle.itemId = itemId;
        vehicle.kind = VehicleKinds::kindOfItem(itemId);
        const auto record = m_items.find(itemId);
        vehicle.lookId = record ? record->lookIds[0] : 0;
        vehicle.position = position;
        vehicle.bagSlot = bagSlot;
        Vehicle& placed = m_placed[slot] = std::move(vehicle);
        if (m_hooks.placed) {
            m_hooks.placed(placed, fly);
        }
        return true;
    }

    void Vehicles::receiveList(const std::vector<protocol::SceneVehicle>& vehicles) {
        m_count = 0;
        for (const protocol::SceneVehicle& vehicle : vehicles) {
            if (vehicle.slot > 0 && vehicle.slot <= kMaxPlaced) {
                place(vehicle.slot, vehicle.ownerId, vehicle.itemId, {vehicle.x, vehicle.y}, vehicle.bagSlot, false);
            }
        }
    }

    void Vehicles::drawFlights(gfx::Canvas& canvas, std::uint64_t nowMs) {
        for (auto& [slot, vehicle] : m_placed) {
            if (vehicle.flight.isActive()) {
                vehicle.flight.update(nowMs);
                vehicle.flight.draw(canvas, nowMs);
            }
        }
    }

    void Vehicles::remove(std::int32_t ownerId, int itemId) {
        const int slot = slotOf(ownerId, itemId);
        if (slot > 0 && slot <= kMaxPlaced && m_count > 0) {
            m_placed.erase(slot);
            --m_count;
        }
    }

    void Vehicles::clear() {
        m_placed.clear();
        m_count = 0;
    }

}
