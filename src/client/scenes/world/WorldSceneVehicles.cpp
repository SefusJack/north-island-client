#include "client/scenes/world/WorldScene.h"

#include "client/data/VehicleKinds.h"
#include "client/rules/VehiclePlacing.h"
#include "client/rules/CharacterActions.h"
#include "client/rules/Walking.h"
#include "protocol/VehicleRequests.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr std::int32_t kLastPlayerId = 9000000; // highest id treated as a player id
        constexpr int kMessageMs = 2000; // message duration, ms
        constexpr double kFlightSpeed = 0.24; // speed of the vehicle's flight to the player (arc effect)
        constexpr int kWaterReach = 6; // radius around the player (in tiles) searched for water

    }

    Vehicles::Hooks WorldScene::vehiclesHooks() {
        Vehicles::Hooks hooks;
        hooks.placed = [this](Vehicle& vehicle, bool fly) {
            if (!fly || !isOwn(vehicle.ownerId)) {
                return;
            }
            const auto record = m_services.items.find(vehicle.itemId);
            LightEffect::Setup setup;
            setup.picture = record ? m_services.pictures.find(std::to_string(record->iconId)).value_or(gfx::PictureId{}) : gfx::PictureId{};
            setup.frameMs = 1;
            setup.motion = LightEffect::Motion::Arc;
            setup.loop = false;
            setup.target = m_camera.toScreen(vehicle.position);
            setup.speed = kFlightSpeed;
            vehicle.flight.start(setup, m_camera.toScreen(m_player.position), m_nowMs);
        };
        return hooks;
    }

    bool WorldScene::isQuizZoneMap(int mapId) { return mapId == 10079 || mapId == 10095; } // map ids of the quiz zones (both named Quiz Zone)

    bool WorldScene::nearWater() const {
        if (!m_map) {
            return false;
        }
        const int cell = formats::MapFile::kCellSize;
        const int tileX = m_player.position.x / cell;
        const int tileY = m_player.position.y / cell;
        for (int x = tileX - kWaterReach; x <= tileX + kWaterReach; ++x) {
            for (int y = tileY - kWaterReach; y <= tileY + kWaterReach; ++y) {
                const int attribute = m_map->tileAttribute({x - 1, y - 1});
                if (attribute == formats::MapFile::kDeepWater || attribute == formats::MapFile::kShallowWater) {
                    return true;
                }
            }
        }
        return false;
    }

    void WorldScene::useVehicleItem(int bagSlot) {
        const protocol::ItemInstance& stack = m_inventory.bagItem(bagSlot);
        const auto record = m_services.items.find(stack.itemId);
        if (!record) {
            return;
        }
        if (m_remote.isOn()) {
            m_messages.show("You are using a remote control", 1500); // message duration in ms
            return;
        }
        if (const auto notice = m_bagMarks.lockNotice(bagSlot, 0)) {
            m_messages.show(notice->text, notice->ms);
            return;
        }
        if (!m_bagMarks.isAvailable(bagSlot)) {
            m_messages.show("Unable to operate when the item is locked", kMessageMs);
            return;
        }
        const int mapId = m_map ? m_map->id() : 0;
        PlacingSituation where;
        where.mapId = mapId;
        where.sceneType = sceneType();
        where.houseMap = isHouseMap(mapId);
        where.quizZone = isQuizZoneMap(mapId);
        where.trojanWarMap = onTrojanWarMap();
        if (VehicleKinds::classOf(VehicleKinds::kindOfItem(stack.itemId)) == VehicleKinds::kWaterClass) {
            m_ownTile = {m_player.position.x / formats::MapFile::kCellSize, m_player.position.y / formats::MapFile::kCellSize};
        }
        where.nearWater = nearWater();
        where.onBlockedCell = m_map && m_map->tileAttribute(m_ownTile) != 0;
        where.fighting = m_fightState != 0;
        where.placedCount = m_vehicles.count();
        where.samePlaced = m_vehicles.isPlaced(m_player.id, stack.itemId);
        where.ridingItemId = m_player.vehicle ? m_player.vehicle->itemId : 0;
        where.placedState = m_bagMarks.vehicleState(bagSlot);
        if (const auto refusal = placingRefusal(*record, stack, where)) {
            if (!refusal->text.empty()) {
                m_messages.show(refusal->text, refusal->ms);
            }
            return;
        }
        if (m_player.riding) {
            dismountOwnPet();
        }
        m_services.connection.send(protocol::placeVehicleRequest(bagSlot, stack.itemId));
    }

    void WorldScene::onVehiclePlaced(
        int bagSlot,
        std::int32_t ownerId,
        int itemId,
        core::Point position,
        bool board
    ) {
        const int slot = m_vehicles.freeSlot();
        if (slot <= 0 || slot > Vehicles::kMaxPlaced || bagSlot <= 0 || bagSlot > Inventory::kBagSlots) {
            return;
        }
        const auto record = m_services.items.find(itemId);
        constexpr int kVehicleType = 39; // item type 39: vehicles (Boat, Airship, Cabriolet, ...)
        if ((ownerId <= 0 || ownerId > kLastPlayerId) && (!record || record->type != kVehicleType)) {
            return;
        }
        if (!m_vehicles.place(slot, ownerId, itemId, position, bagSlot, true) || !isOwn(ownerId)) {
            return;
        }
        m_bagMarks.setVehicleState(bagSlot, 1);
        if (!board) {
            m_itemWindow.close();
            return;
        }
        if (m_boardCell) {
            placeCharacter(m_player, {m_boardCell->x * formats::MapFile::kCellSize, m_boardCell->y * formats::MapFile::kCellSize});
        }
        m_boardCell.reset();
        tryBoardOwnVehicle(slot);
    }

    bool WorldScene::vehicleSupplyRefused(const formats::ItemRecord& item) {
        constexpr int kFuelSubType = 121; // item sub type 121: vehicle fuel items
        if (item.subType == kFuelSubType) {
            if (m_fightState != 0) {
                m_messages.show("Unable to use in combat", 1000); // message duration in ms
                return true;
            }
            if (!m_player.vehicle) {
                m_messages.show("Unable to use without using a vehicle", 1000); // message duration in ms
                return true;
            }
            const auto vehicle = m_services.items.find(m_player.vehicle->itemId);
            if (!vehicle || (vehicle->attributeValues[0] != item.id && vehicle->attributeValues[1] != item.id)) {
                m_messages.show("The vehicle cannot use this fuel", 1400); // message duration in ms
                return true;
            }
            return false;
        }
        if (VehicleKinds::isVoucher(item.id)) {
            if (m_fightState != 0) {
                m_messages.show("Unable to exchange for vehicle during a combat", kMessageMs);
                return true;
            }
            const int vehicle = VehicleKinds::voucherVehicle(item.id);
            if (vehicle == 0) {
                m_messages.show("Unable to exchange for vehicle at present", kMessageMs);
                return true;
            }
            if (m_inventory.freeSlotFor(vehicle) == 0) {
                m_messages.show("Your body space is not enough", kMessageMs);
                return true;
            }
        }
        return false;
    }

    std::optional<VehicleGauge> WorldScene::vehicleGauge() const {
        if (!m_player.vehicle || m_player.vehicle->seat != 1 || m_vehicleBagSlot < 1 || m_vehicleBagSlot > Inventory::kBagSlots) {
            return std::nullopt;
        }
        const auto record = m_services.items.find(m_player.vehicle->itemId);
        if (!record) {
            return std::nullopt;
        }
        VehicleGauge gauge;
        gauge.icon = std::to_string(record->iconId);
        gauge.capsule = VehicleKinds::isCapsule(record->id);
        const auto shape = gauge.capsule ? m_services.items.find(record->aliasItemId).value_or(*record) : *record;
        gauge.cellWidth = shape.cellWidth;
        gauge.cellHeight = shape.cellHeight;
        const protocol::ItemInstance& stack = m_inventory.bagItem(m_vehicleBagSlot);
        const int limit = vehicleWearLimit(*record);
        gauge.durability = limit > 0 ? static_cast<double>(limit - stack.usedAmount) / limit : 0.0;
        gauge.hasFuel = stack.extraAmount > 0;
        gauge.fuel = limit > 0 ? static_cast<double>(stack.extraAmount) / limit : 0.0;
        return gauge;
    }

    void WorldScene::mountVehicle(MapCharacter& rider, RiddenVehicle vehicle) {
        constexpr double kWalkSpeed = 0.16;
        stopAction(rider);
        rider.vehicle = std::move(vehicle);
        rider.pixelsPerMs = kWalkSpeed + VehicleKinds::speedBonus(rider.vehicle->kind);
        if (m_teams.isLeader(rider.id)) {
            loadPassengers(rider);
        }
    }

    void WorldScene::dismountVehicle(MapCharacter& rider) {
        constexpr double kWalkSpeed = 0.16;
        if (m_teams.isLeader(rider.id)) {
            unloadPassengers(rider);
        }
        rider.vehicle.reset();
        rider.pixelsPerMs = kWalkSpeed;
        startWalking(rider, {}, m_nowMs);
    }

    void WorldScene::loadPassengers(MapCharacter& driver) {
        constexpr double kWalkSpeed = 0.16;
        if (!driver.vehicle) {
            return;
        }
        driver.vehicle->passengers = m_teams.members(driver.id);
        const std::vector<std::int32_t> members = driver.vehicle->passengers;
        for (std::size_t i = 0; i < members.size(); ++i) {
            const auto found = characterOf(members[i]);
            if (!found) {
                continue;
            }
            MapCharacter& member = found->get();
            if (static_cast<int>(i) + 1 <= VehicleKinds::seatCount(driver.vehicle->kind) - 1) {
                RiddenVehicle seat = *driver.vehicle;
                seat.passengers.clear();
                seat.seat = static_cast<int>(i) + 2; // seat 1 is the driver's, passengers start at seat 2
                member.vehicle = std::move(seat);
                member.pixelsPerMs = kWalkSpeed + VehicleKinds::speedBonus(driver.vehicle->kind);
            } else {
                member.vehicle.reset();
                member.pixelsPerMs = kWalkSpeed;
            }
            stopAction(member);
        }
    }

    void WorldScene::unloadPassengers(MapCharacter& driver) {
        constexpr double kWalkSpeed = 0.16;
        if (!driver.vehicle) {
            return;
        }
        for (const std::int32_t id : driver.vehicle->passengers) {
            if (const auto found = characterOf(id)) {
                found->get().vehicle.reset();
                found->get().pixelsPerMs = kWalkSpeed;
            }
        }
        driver.vehicle->passengers.clear();
    }

    void WorldScene::onVehicleBoarded(int bagSlot, std::int32_t playerId, int itemId) {
        const auto record = m_services.items.find(itemId);
        constexpr int kVehicleType = 39; // item type 39: vehicles (Boat, Airship, Cabriolet, ...)
        const auto rider = characterOf(playerId);
        if (!record || record->type != kVehicleType || !rider) {
            return;
        }
        RiddenVehicle vehicle;
        vehicle.itemId = itemId;
        vehicle.kind = VehicleKinds::kindOfItem(itemId);
        vehicle.lookId = record->lookIds[0];
        vehicle.ownerId = playerId;
        vehicle.seat = 1;
        mountVehicle(rider->get(), std::move(vehicle));
        if (isOwn(playerId)) {
            for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
                const auto item = m_services.items.find(m_inventory.bagItem(slot).itemId);
                if (!item || item->type != kVehicleType) {
                    continue;
                }
                if (slot == bagSlot) {
                    m_vehicleBagSlot = bagSlot;
                    m_bagMarks.setVehicleState(bagSlot, 2); // vehicle state 2: ridden
                    m_walkSent = false;
                } else if (m_bagMarks.vehicleState(slot) == 2) { // vehicle state 2: ridden
                    m_bagMarks.setVehicleState(slot, 0);
                }
            }
        }
        m_vehicles.remove(playerId, itemId);
    }

    void WorldScene::onVehicleDismounted(int bagSlot, std::int32_t playerId) {
        const auto rider = characterOf(playerId);
        if (!rider) {
            return;
        }
        if (rider->get().vehicle) {
            dismountVehicle(rider->get());
        }
        if (isOwn(playerId) && bagSlot >= 1 && bagSlot <= Inventory::kBagSlots) {
            m_bagMarks.setVehicleState(bagSlot, 0);
        }
    }

    void WorldScene::onVehicleWear(int bagSlot, std::int32_t ownerId, std::int32_t wear, int fuel) {
        constexpr int kVehicleType = 39; // item type 39: vehicles (Boat, Airship, Cabriolet, ...)
        if (bagSlot < 1 || bagSlot > Inventory::kBagSlots || !isOwn(ownerId) || !m_player.vehicle ||
            m_bagMarks.vehicleState(bagSlot) != 2) { // vehicle state 2: ridden
            return;
        }
        const auto record = m_services.items.find(m_inventory.bagItem(bagSlot).itemId);
        if (!record || record->type != kVehicleType) {
            return;
        }
        m_inventory.setUsedAmount(bagSlot, std::min(wear, vehicleWearLimit(*record)));
        m_inventory.setExtraAmount(bagSlot, fuel);
    }

    CapsuleWindow::Hooks WorldScene::capsuleHooks() {
        CapsuleWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& bytes) { m_services.connection.send(bytes); };
        hooks.fighting = [this] { return m_fightState != 0; };
        hooks.itemWindowBounds = [this] { return m_itemWindow.bounds(); };
        hooks.moveItemWindow = [this](int left) { m_itemWindow.moveLeft(left); };
        hooks.marksChanged = [this] { bagMarksChanged(); };
        return hooks;
    }

    TransportWindow::Hooks WorldScene::transportHooks() {
        TransportWindow::Hooks hooks;
        hooks.vehicleItem = [this] { return m_player.vehicle ? m_player.vehicle->itemId : 0; };
        hooks.vehicleBagItem = [this]() -> std::optional<protocol::ItemInstance> {
            if (m_vehicleBagSlot < 1 || m_vehicleBagSlot > Inventory::kBagSlots) {
                return std::nullopt;
            }
            return m_inventory.bagItem(m_vehicleBagSlot);
        };
        hooks.sceneType = [this] { return sceneType(); };
        hooks.setOff = [this](int place) { m_transmit.begin(place, m_camera.toScreen(m_player.position), m_nowMs); };
        return hooks;
    }

    void WorldScene::onVehicleRepair(int code, std::int32_t wear) { m_vehicleRepair.result(code, wear); }

    void WorldScene::onParkList(const std::vector<protocol::ParkedVehicle>& cells) { m_parking.setList(cells); }

    void WorldScene::onParkSlot(const protocol::ParkedVehicle& cell) { m_parking.setCell(cell); }

    void WorldScene::onParkResult(int code, int itemId) { m_parking.result(code, itemId); }

    void WorldScene::onParkClear() { m_parking.clear(); }

    void WorldScene::onParkOpen(int code) { m_parking.opened(code); }

    void WorldScene::onVehicleNoFuel() { m_messages.show("Not enough fuel", kMessageMs); }

    void WorldScene::onSceneRiders(const std::vector<protocol::SceneRider>& riders) {
        constexpr int kVehicleType = 39; // item type 39: vehicles (Boat, Airship, Cabriolet, ...)
        for (const protocol::SceneRider& entry : riders) {
            const auto rider = characterOf(entry.playerId);
            const auto record = m_services.items.find(entry.itemId);
            if (!rider || !record || record->type != kVehicleType) {
                continue;
            }
            RiddenVehicle vehicle;
            vehicle.itemId = entry.itemId;
            vehicle.kind = VehicleKinds::kindOfItem(entry.itemId);
            vehicle.lookId = record->lookIds[0];
            vehicle.ownerId = entry.playerId;
            mountVehicle(rider->get(), std::move(vehicle));
        }
    }

    void WorldScene::onVehicleTakenBack(int bagSlot) {
        if (bagSlot < 1 || bagSlot > Inventory::kBagSlots) {
            return;
        }
        m_bagMarks.setVehicleState(bagSlot, 0);
        m_messages.show("The vehicle has been taken back", kMessageMs);
        m_vehicles.remove(m_player.id, m_inventory.bagItem(bagSlot).itemId);
    }

    void WorldScene::onVehicleRemoved(std::int32_t ownerId, int itemId) {
        if (ownerId > 0 && ownerId <= kLastPlayerId) {
            m_vehicles.remove(ownerId, itemId);
        }
    }

    void WorldScene::onVehicleDestroyed(std::int32_t ownerId, int itemId) {
        const auto owner = characterOf(ownerId);
        if (!owner) {
            return;
        }
        const core::Point where = owner->get().position;
        m_vehicleWrecks.add(itemId, ownerId, where, m_camera.toScreen(where), m_nowMs);
        if (isOwn(ownerId)) {
            m_messages.show("The vehicle has been destroyed", kMessageMs);
        }
    }

    void WorldScene::onSceneVehicles(const std::vector<protocol::SceneVehicle>& vehicles) {
        m_vehicles.receiveList(vehicles);
        m_vehicleWrecks.clear();
    }

    std::optional<int> WorldScene::vehicleAt(core::Point screenPoint) {
        std::optional<int> found;
        int foundY = 0;
        for (const auto& [slot, vehicle] : m_vehicles.placedVehicles()) {
            if (vehicle.flight.isActive()) {
                continue;
            }
            const int vehicleSlot = vehicle.kind == 17 ? 3 : 11; // vehicle kind 17 is drawn in sprite slot 3, the others in slot 11
            const core::Point feet = m_camera.toScreen(vehicle.position);
            const core::Point at{feet.x, feet.y + VehicleKinds::spriteDrop(vehicle.kind)};
            if (m_services.characters.vehicleContains(vehicle.lookId, at, vehicleSlot, vehicle.shownFrame, screenPoint) &&
                (!found || vehicle.position.y >= foundY)) {
                found = slot;
                foundY = vehicle.position.y;
            }
        }
        return found;
    }

    void WorldScene::clickVehicle(int slot) {
        if (m_player.riding) {
            m_messages.show("Unable to use vehicle while riding a pet", 1500); // message duration in ms
            return;
        }
        if (m_teams.isLeader(m_player.id)) {
            for (const std::int32_t id : m_teams.members(m_player.id)) {
                if (const auto member = characterOf(id); member && member->get().riding) {
                    m_messages.show("Unable to use a vehicle while riding a pet", 1500); // message duration in ms
                    return;
                }
            }
        }
        tryBoardVehicle(slot);
    }

    void WorldScene::tryBoardVehicle(int slot) {
        const auto found = m_vehicles.find(slot);
        if (!found) {
            return;
        }
        const Vehicle& vehicle = found->get();
        if (m_remote.isOn()) {
            m_messages.show("You are using a remote control", 1500); // message duration in ms
            return;
        }
        if (m_player.stall) {
            m_messages.show("Unable to take a transportation vehicle while keeping a stall.", 1500); // message duration in ms
            return;
        }
        if (m_player.fishing) {
            m_messages.show("Fishing, cancel the action", kMessageMs);
            return;
        }
        const char* const kNoVehicleScene = "Vehicles are prohibited in this scene";
        switch (sceneType()) {
            case 3:
            case 4:
            case 9:
            case 11: // scene types 3, 4, 9, 11: only robots and magic carpets may be boarded
                if (!VehicleKinds::isRobot(vehicle.itemId) && !VehicleKinds::isMagicCarpet(vehicle.itemId)) {
                    m_messages.show(kNoVehicleScene, kMessageMs);
                    return;
                }
                break;
            case 8:
            case 14: // scene types 8 and 14: vehicles are prohibited
                m_messages.show(kNoVehicleScene, kMessageMs);
                return;
            default: // any other scene type: vehicles allowed
                break;
        }
        if (m_map && m_map->tileAttribute(m_ownTile) != 0) {
            m_messages.show("Cannot use transportation vehicle here", kMessageMs);
            // boarding reach in pixels around the vehicle: 100 left/right, 150 above, 100 below
        } else if (
            m_player.position.x <= vehicle.position.x - 100 || m_player.position.x >= vehicle.position.x + 100 ||
            m_player.position.y <= vehicle.position.y - 150 || m_player.position.y >= vehicle.position.y + 100
        ) {
            m_messages.show("too far away", kMessageMs);
        } else if (m_player.vehicle) {
            m_messages.show("You've used a vehicle", kMessageMs);
        } else if (!isOwn(vehicle.ownerId)) {
            m_messages.show("Invalid ID, unable to use", kMessageMs);
        } else if (m_teams.isLeader(m_player.id) && static_cast<int>(m_teams.members(m_player.id).size()) + 1 > VehicleKinds::seatCount(vehicle.kind)) {
            m_messages.show("The loading number reaches the upper limit", kMessageMs);
        } else {
            m_services.connection.send(protocol::boardVehicleRequest(vehicle.bagSlot, vehicle.itemId));
        }
    }

    void WorldScene::tryBoardOwnVehicle(int slot) {
        const auto found = m_vehicles.find(slot);
        if (!found || m_player.vehicle) {
            return;
        }
        if (m_player.fishing) {
            m_messages.show("Fishing, cancel the action", kMessageMs);
            return;
        }
        if (!isOwn(found->get().ownerId)) {
            m_messages.show("Invalid ID, unable to use", kMessageMs);
            return;
        }
        m_services.connection.send(protocol::boardVehicleRequest(found->get().bagSlot, found->get().itemId));
    }

    void WorldScene::parkRiddenVehicle() {
        if (!m_player.vehicle || m_bagMarks.vehicleState(m_vehicleBagSlot) != 2) { // vehicle state 2: ridden
            return;
        }
        if (m_vehicles.count() >= Vehicles::kMaxPlaced) {
            m_messages.show("The scene is full", kMessageMs);
        } else if (m_vehicles.isPlaced(m_player.id, m_player.vehicle->itemId)) {
            m_messages.show("The same vehicle has been placed", kMessageMs);
        } else {
            m_services.connection.send(protocol::parkVehicleRequest(m_vehicleBagSlot, m_player.vehicle->itemId));
        }
    }

    void WorldScene::rideVehicleItem(int bagSlot) {
        constexpr int kVehicleType = 39; // item type 39: vehicles (Boat, Airship, Cabriolet, ...)
        const auto record = m_services.items.find(m_inventory.bagItem(bagSlot).itemId);
        if (record && record->type == kVehicleType && m_bagMarks.vehicleState(bagSlot) != 0) {
            m_services.connection.send(protocol::useVehicleRequest(bagSlot, m_inventory.bagItem(bagSlot).itemId));
        }
    }

    void WorldScene::walkIntoWaterOrOut(core::Point clicked) {
        constexpr int kVehicleType = 39; // item type 39: vehicles (Boat, Airship, Cabriolet, ...)
        constexpr int kReach = 3;
        if (!m_map || m_walkSent) {
            return;
        }
        const int cell = formats::MapFile::kCellSize;
        const core::Point clickedCell{clicked.x / cell, clicked.y / cell};
        const core::Point ownCell{m_player.position.x / cell, m_player.position.y / cell};
        const auto attribute = [this](int x, int y) { return m_map->tileAttribute({x - 1, y - 1}); };
        const auto around = [&](int wanted) -> std::optional<core::Point> {
            for (int x = ownCell.x - 2; x <= ownCell.x + kReach; ++x) {
                for (int y = ownCell.y - 2; y <= ownCell.y + kReach; ++y) {
                    if (attribute(x, y) == wanted) {
                        return core::Point{x, y};
                    }
                }
            }
            return std::nullopt;
        };
        constexpr int kDeep = formats::MapFile::kDeepWater;
        constexpr int kShallow = formats::MapFile::kShallowWater;
        if (!m_player.vehicle) {
            if (!((attribute(clickedCell.x + 1, clickedCell.y + 1) == kDeep || attribute(clickedCell.x + 1, clickedCell.x + 1) == kShallow) &&
                  attribute(ownCell.x + 1, ownCell.y + 1) == 0)) {
                return;
            }
            const auto water = around(kDeep);
            if (!water) {
                return;
            }
            m_boardCell = water;
            const int partners = static_cast<int>(m_teams.members(m_player.id).size());
            for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
                const protocol::ItemInstance& stack = m_inventory.bagItem(slot);
                const auto record = m_services.items.find(stack.itemId);
                if (!record || record->type != kVehicleType) {
                    continue;
                }
                const int kind = VehicleKinds::kindOfItem(stack.itemId);
                if (VehicleKinds::classOf(kind) == VehicleKinds::kWaterClass && !(partners > 0 && VehicleKinds::seatCount(kind) < partners + 1) &&
                    vehicleWearLimit(*record) > stack.usedAmount && !m_player.riding && m_bagMarks.vehicleState(slot) == 0 &&
                    m_vehicles.count() < Vehicles::kMaxPlaced && !m_vehicles.isPlaced(m_player.id, stack.itemId)) {
                    m_walkSent = true;
                    m_services.connection.send(protocol::enterWaterVehicleRequest(slot, stack.itemId));
                    return;
                }
            }
        } else if (VehicleKinds::classOf(m_player.vehicle->kind) == VehicleKinds::kWaterClass) {
            const int own = attribute(ownCell.x + 1, ownCell.y + 1);
            if (attribute(clickedCell.x, clickedCell.y) != 0 || (own != kDeep && own != kShallow)) {
                return;
            }
            const auto land = around(0);
            if (!land) {
                return;
            }
            m_boardCell = land;
            for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
                if (m_inventory.bagItem(slot).itemId == m_player.vehicle->itemId) {
                    placeCharacter(m_player, {land->x * cell, land->y * cell});
                    rideVehicleItem(slot);
                    return;
                }
            }
        }
    }

}
