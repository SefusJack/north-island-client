#include "client/actions/Action15Pets.h"

#include "core/ByteReader.h"
#include "protocol/MessageId.h"
#include "protocol/PetLists.h"

namespace client {

    void handleAction15Pets(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        switch (id.sub) {
            case 1: // 15/1: a pet joined (PetLists)
                if (const auto joined = protocol::decodePetJoined(payload)) {
                    context.events.onPetJoined(*joined);
                    return;
                }
                break;
            case 2: { // 15/2: i32 owner, u8 place, a pet left
                core::ByteReader reader(payload, 2);
                const std::int32_t ownerId = reader.i32();
                const int place = reader.u8();
                if (!reader.failed()) {
                    context.events.onPetLeft(ownerId, place);
                    return;
                }
                break;
            }
            case 4: // 15/4: other players' fighting pets (PetLists)
                context.events.onScenePets(protocol::decodeScenePets(payload));
                return;
            case 7: // 15/7: pets that are mounted (PetLists)
                context.events.onPetsMounted(protocol::decodePetsMounted(payload));
                return;
            case 8: // 15/8: the own pets (PetLists)
                context.events.onPetList(protocol::decodePetList(payload));
                return;
            case 9: { // 15/9: i32 owner, u8 place, name: a pet renamed
                core::ByteReader reader(payload, 2);
                const std::int32_t ownerId = reader.i32();
                const int place = reader.u8();
                const std::string name = reader.rest();
                if (!reader.failed()) {
                    context.events.onPetRenamed(ownerId, place, name);
                    return;
                }
                break;
            }
            case 19: // 15/19: the stars owned
                context.events.onStarsOwned(payload);
                return;
            case 20: { // 15/20: u8 star, u8 npc: a star acquired
                core::ByteReader reader(payload, 2);
                const int star = reader.u8();
                const int npc = reader.u8();
                if (!reader.failed()) {
                    context.events.onStarAcquired(star, npc);
                    return;
                }
                break;
            }
            case 16: { // 15/16: u8 place, i32 player, i32 npc: a pet mounted
                core::ByteReader reader(payload, 2);
                const int place = reader.u8();
                const std::int32_t playerId = reader.i32();
                const std::int32_t npcId = reader.i32();
                if (!reader.failed()) {
                    context.events.onPetMounted(playerId, place, npcId);
                    return;
                }
                break;
            }
            case 17: { // 15/17: i32 player, a pet dismounted
                core::ByteReader reader(payload, 2);
                const std::int32_t playerId = reader.i32();
                if (!reader.failed()) {
                    context.events.onPetDismounted(playerId);
                    return;
                }
                break;
            }
            case 10: { // 15/10: u8 bag slot, i32 player, u16 item: a vehicle boarded
                core::ByteReader vehicle(payload, 2);
                const int bagSlot = vehicle.u8();
                const std::int32_t playerId = vehicle.i32();
                const int itemId = vehicle.u16();
                if (!vehicle.failed()) {
                    context.events.onVehicleBoarded(bagSlot, playerId, itemId);
                    return;
                }
                break;
            }
            case 11: { // 15/11: u8 bag slot, i32 player: a vehicle dismounted
                core::ByteReader vehicle(payload, 2);
                const int bagSlot = vehicle.u8();
                const std::int32_t playerId = vehicle.i32();
                if (!vehicle.failed()) {
                    context.events.onVehicleDismounted(bagSlot, playerId);
                    return;
                }
                break;
            }
            case 12: // 15/12: u8 bag slot, i32 owner, u16 item, i32 x, i32 y: a vehicle placed
            case 18: { // 15/18: the same record as 15/12 (flagged on the event)
                core::ByteReader vehicle(payload, 2);
                const int bagSlot = vehicle.u8();
                const std::int32_t ownerId = vehicle.i32();
                const int itemId = vehicle.u16();
                const std::int32_t x = vehicle.i32();
                const std::int32_t y = vehicle.i32();
                if (!vehicle.failed()) {
                    context.events.onVehiclePlaced(bagSlot, ownerId, itemId, {x, y}, id.sub == 18); // 15/18 sets the flag
                    return;
                }
                break;
            }
            case 13: { // 15/13: u8 bag slot: a vehicle taken back
                core::ByteReader vehicle(payload, 2);
                const int bagSlot = vehicle.u8();
                if (!vehicle.failed()) {
                    context.events.onVehicleTakenBack(bagSlot);
                    return;
                }
                break;
            }
            case 14: { // 15/14: u8 bag slot, i32 owner, i32 wear, u16 fuel: vehicle wear
                core::ByteReader vehicle(payload, 2);
                const int bagSlot = vehicle.u8();
                const std::int32_t ownerId = vehicle.i32();
                const std::int32_t wear = vehicle.i32();
                const int fuel = vehicle.u16();
                if (!vehicle.failed()) {
                    context.events.onVehicleWear(bagSlot, ownerId, wear, fuel);
                    return;
                }
                break;
            }
            case 15: // 15/15: i32 owner, u16 item: a vehicle destroyed
            case 22: { // 15/22: i32 owner, u16 item: a vehicle removed
                core::ByteReader vehicle(payload, 2);
                const std::int32_t ownerId = vehicle.i32();
                const int itemId = vehicle.u16();
                if (!vehicle.failed()) {
                    if (id.sub == 15) { // 15/15 destroyed, else 15/22 removed
                        context.events.onVehicleDestroyed(ownerId, itemId);
                    } else {
                        context.events.onVehicleRemoved(ownerId, itemId);
                    }
                    return;
                }
                break;
            }
            case 23: // 15/23: the vehicle has no fuel
                context.events.onVehicleNoFuel();
                return;
            case 24: { // 15/24: u16 minutes a soldier is available
                core::ByteReader reader(payload, 2);
                const int minutes = reader.u16();
                if (!reader.failed()) {
                    context.events.onSoldierTime(minutes);
                    return;
                }
                break;
            }
            case 25: { // 15/25: i32 player, u16 pet kind, u16 new kind (RecvScenePetNpcChange)
                core::ByteReader reader(payload, 2);
                const std::int32_t playerId = reader.i32();
                const int petKind = reader.u16();
                const int newKind = reader.u16();
                if (!reader.failed()) {
                    context.events.onScenePetChanged(playerId, petKind, newKind);
                    return;
                }
                break;
            }
            case 21: { // 15/21: u16 the pet's kind: pet gear back home
                core::ByteReader reader(payload, 2);
                const int npcId = reader.u16();
                if (!reader.failed()) {
                    context.events.onPetGearBackHome(npcId);
                    return;
                }
                break;
            }
            default: // any other sub id: unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
