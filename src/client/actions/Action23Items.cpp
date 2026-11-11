#include "client/actions/Action23Items.h"

#include "core/ByteReader.h"
#include "protocol/GroundItemRecord.h"
#include "protocol/ItemLists.h"
#include "protocol/MessageId.h"
#include "protocol/PetLists.h"
#include "protocol/PuppetPacket.h"
#include "protocol/StallLists.h"
#include "protocol/UtensilStallRequests.h"
#include "protocol/VehicleLists.h"

#include <array>
#include <vector>

namespace client {

    void handleAction23Items(const core::Bytes& payload, MessageContext& context) {
        const protocol::MessageId id = protocol::MessageId::of(payload);
        core::ByteReader reader(payload, 2); // body starts after the 2-byte action/sub header
        switch (id.sub) {
            case 5: // 23/5: bag records: the bag
                context.events.onBagItems(protocol::decodeBagItems(payload));
                return;
            case 8: // 23/8: one record added to a place (pick-up sound)
                if (const auto record = protocol::decodeBagItem(payload)) {
                    context.events.onBagItemAdded(*record);
                    return;
                }
                break;
            case 7: { // 23/7: u16 item, u8 count: taken, lowest places first
                const int itemId = reader.u16();
                const int count = reader.u8();
                if (!reader.failed()) {
                    context.events.onBagItemsRemoved(itemId, count);
                    return;
                }
                break;
            }
            case 9: // 23/9: u8 slot, u8 count: taken from a place
            case 10: { // 23/10: u8 from, u8 count, u8 to: moved
                const int slot = reader.u8();
                const int count = reader.u8();
                if (id.sub == 9 && !reader.failed()) { // sub 9 = decrease only
                    context.events.onBagSlotDecreased(slot, count);
                    return;
                }
                const int toSlot = reader.u8();
                if (!reader.failed()) {
                    context.events.onBagItemsMoved(slot, count, toSlot);
                    return;
                }
                break;
            }
            case 11: // 23/11: equipment records: what is worn
                context.events.onEquipment(protocol::decodeEquipment(payload));
                return;
            case 12: // 23/12: u8 slot: a worn item is gone
            case 19: { // 23/19: u8 slot: the same, quietly
                const int slot = reader.u8();
                if (!reader.failed()) {
                    context.events.onEquipmentRemoved(slot, id.sub == 12); // sub 12 = with sound
                    return;
                }
                break;
            }
            case 13: // 23/13: compound result records
                while (reader.remaining() >= 4) { // record size: u16 + u8 + u8
                    const int itemId = reader.u16();
                    reader.skip(1);
                    const int slot = reader.u8();
                    context.events.onCompoundResult(itemId, slot);
                }
                return;
            case 15: // 23/15: the sound of using an item
                context.events.onItemUsed();
                return;
            case 30: // 23/30: the own stall opened
                if (const auto stall = protocol::decodeOwnStall(payload)) {
                    context.events.onOwnItemStall(*stall);
                    return;
                }
                break;
            case 31: // 23/31: stalls on the map
                context.events.onItemStalls(protocol::decodeStallSigns(payload));
                return;
            case 32: { // 23/32: i32 player: a stall closed
                const std::int32_t playerId = reader.i32();
                if (!reader.failed()) {
                    context.events.onItemStallClosed(playerId);
                    return;
                }
                break;
            }
            case 33: // 23/33: what someone's stall sells
                if (const auto goods = protocol::decodeStallGoods(payload)) {
                    context.events.onStallGoods(*goods);
                    return;
                }
                break;
            case 34: // 23/34: something sold from the own stall
                if (const auto sale = protocol::decodeStallSale(payload)) {
                    context.events.onStallSale(*sale);
                    return;
                }
                break;
            case 130: // 23/130: the own utensil stall opened
                if (const auto stall = protocol::decodeOwnStall(payload)) {
                    context.events.onOwnUtensilStall(*stall);
                    return;
                }
                break;
            case 131: // 23/131: utensil stalls on the map
                context.events.onUtensilStalls(protocol::decodeStallSigns(payload));
                return;
            case 132: { // 23/132: i32 player: a utensil stall closed
                const std::int32_t playerId = reader.i32();
                if (!reader.failed()) {
                    context.events.onUtensilStallClosed(playerId);
                    return;
                }
                break;
            }
            case 133: // 23/133: what someone's utensil stall sells
                if (const auto goods = protocol::decodeUtensilStallGoods(payload)) {
                    context.events.onUtensilStallGoods(*goods);
                    return;
                }
                break;
            case 134: // 23/134: something sold from the own utensil stall
                if (const auto sale = protocol::decodeStallSale(payload)) {
                    context.events.onUtensilStallSale(*sale);
                    return;
                }
                break;
            case 79: // 23/79: remote reset
                context.events.onRemoteReset();
                return;
            case 137: // 23/137: auto-collecting player ids (list)
            case 138: { // 23/138: auto-collecting player ids (the other state)
                std::vector<std::int32_t> ids;
                while (!reader.failed() && reader.remaining() >= 4) { // 4: bytes per i32 player id
                    ids.push_back(reader.i32());
                }
                context.events.onAutoCollecting(ids, id.sub == 138); // sub 138 flag
                return;
            }
            case 136: { // 23/136: the Space Contract
                const int kind = reader.u8();
                const int flags = kind == 5 ? reader.u8() : 0; // kind 5 carries flags
                const std::int32_t hours = kind == 5 || kind == 7 ? reader.i32() : 0; // kinds 5 and 7 carry hours
                if (!reader.failed()) {
                    context.events.onSpaceContract(kind, flags, hours);
                    return;
                }
                break;
            }
            case 81: { // 23/81: u8 code, vehicle repair result
                const int code = reader.u8();
                const std::int32_t wear = code == 2 ? reader.i32() : 0; // code 2 carries the wear value
                if (!reader.failed()) {
                    context.events.onVehicleRepair(code, wear);
                    return;
                }
                break;
            }
            case 41: { // 23/41: u8 dark mode flag
                const bool dark = reader.u8() != 0;
                if (!reader.failed()) {
                    context.events.onDarkMode(dark);
                    return;
                }
                break;
            }
            case 42: // 23/42: revert item variants
                context.events.onRevertItemVariants();
                return;
            case 56: { // 23/56: u8 code: pet sublimation failed
                const int code = reader.u8();
                if (!reader.failed()) {
                    context.events.onPetSublimateFailed(code);
                    return;
                }
                break;
            }
            case 121: // 23/121: force the player to leave the water
                context.events.onForceLeaveWater();
                return;
            case 122: // 23/122: fishing stopped (RecvFishingStopped)
            case 123: { // 23/123: fishing started (RecvFishingStarted)
                const std::int32_t playerId = reader.i32();
                const int kind = id.sub == 123 ? reader.u8() : 0; // only 123 carries the rod kind
                if (!reader.failed()) {
                    if (id.sub == 123) { // 123 = started
                        context.events.onFishingStarted(playerId, kind);
                    } else {
                        context.events.onFishingStopped(playerId);
                    }
                    return;
                }
                break;
            }
            case 153: // 23/153: park list
                context.events.onParkList(protocol::decodeParkList(payload));
                return;
            case 154: // 23/154: one park slot
                if (const auto cell = protocol::decodeParkSlot(payload)) {
                    context.events.onParkSlot(*cell);
                    return;
                }
                break;
            case 155: { // 23/155: park result (ParkForm.RecvParkResult)
                const int code = reader.u8();
                const int itemId = code == 1 || code == 3 ? reader.i32() : 0; // codes 1 and 3 carry an item id
                if (!reader.failed()) {
                    context.events.onParkResult(code, itemId);
                    return;
                }
                break;
            }
            case 156: // 23/156: park cleared
                context.events.onParkClear();
                return;
            case 157: { // 23/157: u8 code: park opens
                const int code = reader.u8();
                if (!reader.failed()) {
                    context.events.onParkOpen(code);
                    return;
                }
                break;
            }
            case 102: // 23/102: vehicles in the scene
                context.events.onSceneVehicles(protocol::decodeSceneVehicles(payload));
                return;
            case 115: // 23/115: riders in the scene
                context.events.onSceneRiders(protocol::decodeSceneRiders(payload));
                return;
            case 116: // 23/116: pet rides
                context.events.onPetRides(protocol::decodePetRides(payload));
                return;
            case 69: { // 23/69: i32 player, u16 item: pet equipped (HandlePetEquipChangePacket)
                const std::int32_t playerId = reader.i32();
                const int itemId = reader.u16();
                if (!reader.failed()) {
                    context.events.onScenePetEquipped(playerId, itemId);
                    return;
                }
                break;
            }
            case 70: { // 23/70: i32 player: pet unequipped (HandlePetUnequipPacket)
                const std::int32_t playerId = reader.i32();
                if (!reader.failed()) {
                    context.events.onScenePetUnequipped(playerId);
                    return;
                }
                break;
            }
            case 110: // 23/110: the own pet stall opened
                if (const auto stall = protocol::decodeOwnStall(payload)) {
                    context.events.onOwnPetStall(*stall);
                    return;
                }
                break;
            case 111: // 23/111: pet stalls on the map
                context.events.onPetStalls(protocol::decodeStallSigns(payload));
                return;
            case 112: { // 23/112: i32 player: a pet stall closed
                const std::int32_t playerId = reader.i32();
                if (!reader.failed()) {
                    context.events.onPetStallClosed(playerId);
                    return;
                }
                break;
            }
            case 113: // 23/113: what someone's pet stall sells
                if (const auto goods = protocol::decodePetStallGoods(payload)) {
                    context.events.onPetStallGoods(*goods);
                    return;
                }
                break;
            case 114: // 23/114: something sold from the own pet stall
                if (const auto sale = protocol::decodePetStallSale(payload)) {
                    context.events.onPetStallSale(*sale);
                    return;
                }
                break;
            case 2: { // 23/2: u16 index: a ground item is gone
                const int index = reader.u16();
                if (!reader.failed()) {
                    context.events.onGroundItemRemoved(index, !reader.atEnd());
                    return;
                }
                break;
            }
            case 3: { // 23/3: u16 item, u16 x, u16 y, i32 extra: appears on the map
                const int itemId = reader.u16();
                const int x = reader.u16();
                const int y = reader.u16();
                const std::int32_t extra = reader.i32();
                const int appearMode = reader.atEnd() ? 0 : reader.u8();
                if (!reader.failed()) {
                    context.events.onGroundItemAdded(itemId, core::Point{x, y}, extra, appearMode);
                    return;
                }
                break;
            }
            case 18: { // 23/18: i32 gold, u16 x, u16 y: a gold pile appears on the map
                const std::int32_t gold = reader.i32();
                const int x = reader.u16();
                const int y = reader.u16();
                const bool dropped = !reader.atEnd();
                if (!reader.failed()) {
                    context.events.onGroundGoldAdded(gold, core::Point{x, y}, dropped);
                    return;
                }
                break;
            }
            case 4: { // 23/4: ground records: what lies on the map
                std::vector<protocol::GroundItemRecord> records;
                while (reader.remaining() >= 15) { // ground record size: u8 + u16 + i32 + u16 + u16 + i32
                    protocol::GroundItemRecord record;
                    record.kind = reader.u8();
                    record.index = reader.u16();
                    record.itemOrGold = reader.i32();
                    record.position.x = reader.u16();
                    record.position.y = reader.u16();
                    record.extra = reader.i32();
                    records.push_back(record);
                }
                context.events.onGroundItems(records);
                return;
            }
            case 6: { // 23/6: an item obtained (added to the bag)
                protocol::ItemInstance item;
                item.itemId = reader.u16();
                item.count = reader.u16();
                item.wear = reader.u8();
                item.byteA = reader.u8();
                item.byteB = reader.u8();
                item.usedAmount = reader.i32();
                item.extraAmount = reader.u16();
                item.insertItemId = reader.u16();
                item.expiry = reader.f64();
                item.fusionLevel = reader.u8();
                item.attributeAdjust = reader.u8();
                item.lookItemId = reader.u16();
                item.boxItemId = reader.u16();
                if (!reader.failed()) {
                    context.events.onItemObtained(item);
                    return;
                }
                break;
            }
            case 43: // 23/43: 7-byte records (place first): into the city warehouse
            case 44: { // 23/44: 6-byte records without the place: the client places them
                std::vector<protocol::BagItemRecord> records;
                while (reader.remaining() >= (id.sub == 43 ? 7u : 6u)) { // record sizes: 7 with place byte, 6 without
                    protocol::BagItemRecord record;
                    record.slot = id.sub == 43 ? reader.u8() : 0;
                    record.item.itemId = reader.u16();
                    record.item.count = reader.u8();
                    record.item.wear = reader.u8();
                    record.item.byteA = reader.u8();
                    record.item.byteB = reader.u8();
                    records.push_back(record);
                }
                for (const protocol::BagItemRecord& record : records) {
                    context.events.onWarehouseItem(record);
                }
                return;
            }
            case 45: { // 23/45: u8 place: warehouse place emptied
                const int slot = reader.u8();
                if (!reader.failed()) {
                    context.events.onWarehouseSlotCleared(slot);
                    return;
                }
                break;
            }
            case 46: // 23/46: the warehouse form opens
                context.events.onWarehouseOpened();
                return;
            case 60: { // 23/60: u16 item, u16 count, u8 wear, u8, u8: into the sale store
                protocol::BagItemRecord record;
                record.item.itemId = reader.u16();
                record.item.count = reader.u16();
                record.item.wear = reader.u8();
                record.item.byteA = reader.u8();
                record.item.byteB = reader.u8();
                if (!reader.failed()) {
                    context.events.onSaleStoreItem(record);
                    return;
                }
                break;
            }
            case 61: { // 23/61: 7-byte records (place first): the sale store list
                std::vector<protocol::BagItemRecord> records;
                while (reader.remaining() >= 7u) { // record size: u8 place, u16 item, u8 count, u8 wear, u8, u8
                    protocol::BagItemRecord record;
                    record.slot = reader.u8();
                    record.item.itemId = reader.u16();
                    record.item.count = reader.u8();
                    record.item.wear = reader.u8();
                    record.item.byteA = reader.u8();
                    record.item.byteB = reader.u8();
                    records.push_back(record);
                }
                for (const protocol::BagItemRecord& record : records) {
                    context.events.onSaleStoreItem(record);
                }
                return;
            }
            case 62: { // 23/62: u8 place: sale store place emptied
                const int slot = reader.u8();
                if (!reader.failed()) {
                    context.events.onSaleStoreCleared(slot);
                    return;
                }
                break;
            }
            case 63: { // 23/63: i32 value, 8-byte records: the sale net's list
                protocol::SaleNetList list;
                list.value = reader.i32();
                while (!reader.failed() && reader.remaining() >= 8u) { // record size: u8 place, u16 item, u8 count, u8 wear, u8, u8, u8 state
                    protocol::SaleNetList::Entry entry;
                    entry.place = reader.u8();
                    entry.item.itemId = reader.u16();
                    entry.item.count = reader.u8();
                    entry.item.wear = reader.u8();
                    entry.item.byteA = reader.u8();
                    entry.item.byteB = reader.u8();
                    entry.state = reader.u8();
                    list.entries.push_back(entry);
                }
                if (!reader.failed()) {
                    context.events.onSaleNetList(list);
                    return;
                }
                break;
            }
            case 64: // 23/64: the sale menu opens
                context.events.onSaleMenuOpened();
                return;
            case 65: // 23/65: the pending sale request is answered
                context.events.onSaleRequestDone();
                return;
            case 67: { // 23/67: u8 code: the sale website's error
                const int code = reader.u8();
                if (!reader.failed()) {
                    context.events.onSaleWebError(code);
                    return;
                }
                break;
            }
            case 37: { // 23/37: i32 caster, i32 target, i32 hp, u16 sp: help recover
                const std::int32_t casterId = reader.i32();
                const std::int32_t targetId = reader.i32();
                const std::int32_t hp = reader.i32();
                const int sp = reader.u16();
                if (!reader.failed()) {
                    context.events.onHelpRecover(casterId, targetId, hp, sp);
                    return;
                }
                break;
            }
            case 40: { // 23/40: u8 pet place, i32 hp, u16 sp: recover notice
                const int petPlace = reader.u8();
                const std::int32_t hp = reader.i32();
                const int sp = reader.u16();
                if (!reader.failed()) {
                    context.events.onRecoverNotice(petPlace, hp, sp);
                    return;
                }
                break;
            }
            case 75: // 23/75: i32 player, u16 piece, u8 seat: sits at a furniture piece (UseUtensil)
            case 76: { // 23/76: i32 player: leaves the furniture
                const std::int32_t playerId = reader.i32();
                const int piece = id.sub == 75 ? reader.u16() : 0; // sub 75 carries piece and seat
                const int seat = id.sub == 75 ? reader.u8() : 0; // sub 75 carries piece and seat
                if (!reader.failed()) {
                    if (id.sub == 75) { // 75 = sits down
                        context.events.onUtensilUsed(playerId, piece, seat);
                    } else {
                        context.events.onUtensilLeft(playerId);
                    }
                    return;
                }
                break;
            }
            case 161: { // 23/161: u8 note: another player struck a piano note (PlayPianoNote)
                const int note = reader.u8();
                if (!reader.failed()) {
                    if (note >= 1 && note <= 7) { // notes 1..7 map to wav 9904..9910
                        // note 7 is wav 9910; notes 1..6 are wav 9904..9909 (note + 3)
                        context.events.onSoundEffect("sound/WAV99" + std::string(note == 7 ? "10" : "0" + std::to_string(note + 3)) + ".wav");
                    }
                    return;
                }
                break;
            }
            case 53: // 23/53: i32 player, i32 code: hair and skin colours (part 1)
            case 103: { // 23/103: i32 player, i32 code: clothes and eyes colours (part 5)
                const std::int32_t playerId = reader.i32();
                const std::int32_t code = reader.i32();
                if (!reader.failed()) {
                    context.events.onPlayerColors(playerId, id.sub == 53 ? 1 : 5, code); // colour part: 1 hair/skin, 5 clothes/eyes
                    return;
                }
                break;
            }
            case 146: // 23/146: u16 item, u8 count: the Dog Bone's search found an item
            case 147: { // 23/147: u8 bone place, u8 pet place, u8 code: the search ended
                if (id.sub == 146) { // 146 = found
                    const int itemId = reader.u16();
                    const int count = reader.u8();
                    if (!reader.failed()) {
                        context.events.onForageFound(itemId, count);
                        return;
                    }
                } else {
                    const int bone = reader.u8();
                    const int pet = reader.u8();
                    const int code = reader.u8();
                    if (!reader.failed()) {
                        context.events.onForageEnded(bone, pet, code);
                        return;
                    }
                }
                break;
            }
            case 151: { // 23/151: u8 movie (1..8): play a story movie
                const int movie = reader.u8();
                if (!reader.failed()) {
                    context.events.onStoryMovie(movie);
                    return;
                }
                break;
            }
            case 152: { // 23/152: u8 bone place: the forage bone
                const int bone = reader.u8();
                if (!reader.failed()) {
                    context.events.onForageBone(bone);
                    return;
                }
                break;
            }
            case 160: // 23/160: the fish tank's packets (RecvFishBoxPacket)
                context.events.onFishTankMessage(payload);
                return;
            case 165: { // 23/165: u8 form: the ATM's forms
                const int form = reader.u8();
                if (!reader.failed()) {
                    context.events.onAtmForm(form);
                    return;
                }
                break;
            }
            case 35: { // 23/35: u8 owner, u16 item: equipment broke (RecvEquipBroken)
                const int owner = reader.u8();
                const int itemId = reader.u16();
                if (!reader.failed()) {
                    context.events.onEquipBroken(owner, itemId);
                    return;
                }
                break;
            }
            case 38: // 23/38: a timed light effect (StartEffectS10233)
                context.events.onTimedLight();
                return;
            case 55: { // 23/55: i32 player, light kinds: effect lights (RecvPlayerEffectLights)
                const std::int32_t playerId = reader.i32();
                std::vector<int> kinds;
                while (reader.remaining() > 0) {
                    kinds.push_back(reader.u8());
                }
                if (!reader.failed()) {
                    context.events.onEffectLights(playerId, kinds);
                    return;
                }
                break;
            }
            case 73: { // 23/73: i32 player, u16 piece, u8 seat: bathing (RecvBathing)
                const std::int32_t playerId = reader.i32();
                const int furnIndex = reader.u16();
                const int seat = reader.u8();
                const std::int32_t sitterId = reader.remaining() >= 4 ? reader.i32() : 0; // 4: bytes of the optional i32 sitter
                if (!reader.failed()) {
                    context.events.onBathing(playerId, furnIndex, seat, sitterId);
                    return;
                }
                break;
            }
            case 74: { // 23/74: i32 player: leaves the tub
                const std::int32_t playerId = reader.i32();
                if (!reader.failed()) {
                    context.events.onBathEnd(playerId);
                    return;
                }
                break;
            }
            case 117: { // 23/117: u8 kind, i32 player, u16 x, u16 y: throw effect (HandleThrowFxPacket)
                const int kind = reader.u8();
                const std::int32_t playerId = reader.i32();
                const int x = reader.u16();
                const int y = reader.u16();
                if (!reader.failed()) {
                    context.events.onThrowEffect(kind, playerId, x, y);
                    return;
                }
                break;
            }
            case 148: { // 23/148: i32 player, u8 kind, u8 variant: firework (HandleEffectS51016Packet)
                const std::int32_t playerId = reader.i32();
                const int kind = reader.u8();
                const int variant = reader.u8();
                if (!reader.failed()) {
                    context.events.onFirework(playerId, kind, variant);
                    return;
                }
                break;
            }
            case 212: { // 23/212: u8 reason, u8 bag place, u16 item, u8 count: confirm discard (RecvDiscardConfirm)
                const int reason = reader.u8();
                const int bagSlot = reader.u8();
                const int itemId = reader.u16();
                const int count = reader.u8();
                if (!reader.failed()) {
                    context.events.onDiscardAsked(reason, bagSlot, itemId, count);
                    return;
                }
                break;
            }
            case 77: { // 23/77: u8 bag place, u16 stone: crystal mounted (RecvCrystalMounted)
                const int bagSlot = reader.u8();
                const int stoneId = reader.u16();
                if (!reader.failed()) {
                    context.events.onCrystalMounted(bagSlot, stoneId);
                    return;
                }
                break;
            }
            case 78: // 23/78: u8 result: a pet's ability changed
                if (reader.u8() == 1 && !reader.failed()) { // result 1: success
                    context.events.onNoticeMessage("Change ability successfully", 1500); // notice display time in ms
                    return;
                }
                break;
            case 25: // 23/25: cannot pick the item up
                context.events.onNoticeMessage("Unable to pick the item up.", 1200); // notice display time in ms
                return;
            case 106: { // 23/106: training island item (HandleTrainingIslandItemPacket)
                const int kind = reader.u8();
                const int effect = kind == 1 ? reader.u8() : 0; // kind 1 carries effect, minutes, sound
                const int minutes = kind == 1 ? reader.u16() : 0; // kind 1 carries effect, minutes, sound
                const int sound = kind == 1 ? reader.u8() : 0; // kind 1 carries effect, minutes, sound
                if (!reader.failed()) {
                    context.events.onTrainingItem(kind, effect, minutes, sound == 1); // sound 1 = play the sound
                    return;
                }
                break;
            }
            case 119: { // 23/119: strengthen buff (HandleStrengthenBuffPacket)
                const int kind = reader.u8();
                if (kind == 1) { // kind 1: five minute counters
                    std::array<int, 5> minutes{}; // 5 buff timers
                    for (int& left : minutes) {
                        left = reader.u16();
                    }
                    if (!reader.failed()) {
                        context.events.onStrengthBuffTimes(minutes);
                        return;
                    }
                } else if (kind == 2) { // kind 2: u32 mask and u8 sound
                    const std::uint32_t mask = reader.u32();
                    const int sound = reader.u8();
                    if (!reader.failed()) {
                        context.events.onStrengthBuffMask(mask, sound == 1); // sound 1 = play the sound
                        return;
                    }
                }
                break;
            }
            case 204: { // 23/204: the Stopwatch (exp pause)
                const int sub = reader.u8();
                const int paused = reader.u8();
                if (!reader.failed() && (sub == 1 || sub == 2)) { // sub 1 login state, 2 used or cancelled
                    context.events.onExpPause(sub, paused != 0);
                    return;
                }
                break;
            }
            case 206: { // 23/206: u8 stat: pet stat enhanced
                const int stat = reader.u8();
                if (!reader.failed() && stat <= 5) { // stats 0..4 increase, 5 = cannot enhance anymore
                    context.events.onStatEnhanced(stat);
                    return;
                }
                break;
            }
            case 207: { // 23/207: body shift (TChangeBody)
                const int sub = reader.u8();
                const int value = sub == 2 ? reader.u16() : reader.u8(); // sub 2 carries a u16 minutes left
                if (!reader.failed() && sub >= 1 && sub <= 3) { // subs 1 state, 2 minutes left, 3 refusal
                    context.events.onBodyShift(sub, value);
                    return;
                }
                break;
            }
            case 162: { // 23/162: u8 step: the training at a utensil
                // steps: 1 over, 2 player's step, 3 pet's step, 4 and 5 reset the percent, 8 a pet is being trained
                const int step = reader.u8();
                if (step == 1) {
                    const std::int32_t playerId = reader.i32();
                    const int flag = reader.u8();
                    if (!reader.failed()) {
                        context.events.onTrainingOver(playerId, flag);
                        return;
                    }
                } else if (step == 2) { // step 2: the player's training percent
                    const int percent = reader.u8();
                    const int expGiven = reader.u8();
                    if (!reader.failed()) {
                        context.events.onTrainingPercent(false, 0, percent);
                        if (expGiven == 1) { // 1 = exp was given
                            context.events.onFloatText(false, "ExpUp");
                        }
                        return;
                    }
                } else if (step == 3) { // step 3: the pet's training percent
                    const int percent = reader.u8();
                    const int place = reader.u8();
                    const int expGiven = reader.u8();
                    if (!reader.failed()) {
                        context.events.onTrainingPercent(true, place, percent);
                        if (expGiven == 1) { // 1 = exp was given
                            context.events.onFloatText(true, "ExpUp");
                        }
                        return;
                    }
                } else if (step == 4) { // step 4: reset the player's percent
                    context.events.onTrainingReset(false, 0);
                    return;
                } else if (step == 5) { // step 5: reset the pet's percent
                    reader.skip(1);
                    const int place = reader.u8();
                    if (!reader.failed()) {
                        context.events.onTrainingReset(true, place);
                        return;
                    }
                } else if (step == 8) { // step 8: a pet is being trained
                    context.events.onTrainingBusy();
                    return;
                }
                break;
            }
            case 158: { // 23/158: u8 code: tent style refused
                const int code = reader.u8();
                if (!reader.failed()) {
                    context.events.onTentStyleRefused(code);
                    return;
                }
                break;
            }
            case 159: { // 23/159: u8 bag place, u8 set: the tent style chooser opens
                const int slot = reader.u8();
                const int set = reader.u8();
                if (!reader.failed()) {
                    context.events.onTentStyleChooser(slot, set);
                    return;
                }
                break;
            }
            case 57: { // 23/57: u8 notice: event notice (RecvEventNotice)
                const int notice = reader.u8();
                if (!reader.failed() && (notice == 93 || notice == 94)) { // notice 93 Christmas starts, 94 it ends
                    context.events.onChristmasEvent(notice == 93);
                    return;
                }
                if (!reader.failed() && (notice == 95 || notice == 96)) { // notice 95 New Year starts, 96 it ends
                    context.events.onNewYearEvent(notice == 95);
                    return;
                }
                break;
            }
            case 16: // 23/16: u8 equipSlot, u8 bagSlot: taken off into the bag
            case 17: { // 23/17: u8 bagSlot, u8 freeSlot: put on from the bag
                const int first = reader.u8();
                const int second = reader.u8();
                if (reader.failed()) {
                    break;
                }
                if (id.sub == 16) { // 16 = taken off
                    context.events.onUnequipped(first, second);
                } else {
                    context.events.onEquippedFromBag(first, second);
                }
                return;
            }
            case 216: { // 23/216: u8 code, u8 pet place: potential return result (RecvPotentialReturnResult)
                const int code = reader.u8();
                const int place = reader.u8();
                if (!reader.failed() && code == 1) { // code 1: success
                    context.events.onPotentialReturned(place);
                    return;
                }
                break;
            }
            case 250: { // 23/250: the qol server's item sort (ItemRequests.h sortRequest)
                const int op = reader.u8();
                const int grid = op == 0 ? 0 : reader.u8(); // op 0 has no grid byte
                const int result = op == 2 ? reader.u8() : 0; // op 2 carries a result byte
                if (!reader.failed()) {
                    context.events.onServerSort(op, grid, result);
                    return;
                }
                break;
            }
            case 219: { // 23/219: u8 kind, u8 row, u16 item, u8 count: store tank (RecvStoreTankPacket)
                const int kind = reader.u8();
                const int row = reader.u8();
                const int itemId = reader.u16();
                const int count = reader.u8();
                if (!reader.failed()) {
                    context.events.onStoreTank(kind, row, itemId, count);
                    return;
                }
                break;
            }
            case 213: { // 23/213: u8 code, u8 pet place, u8 potential: potential result (RecvPotentialResult)
                const int code = reader.u8();
                const int place = reader.u8();
                const int potential = reader.u8();
                if (!reader.failed() && code == 1) { // code 1: success
                    context.events.onPotentialResult(place, potential);
                    return;
                }
                break;
            }
            case 208: { // 23/208: u8 code, u8 place or Bottle kind, i32 amount: bottle fill (RecvBottleFill)
                const int code = reader.u8();
                const int place = reader.u8();
                const int amount = reader.i32();
                if (!reader.failed() && code >= 1 && code <= 3) { // known codes 1..3
                    if (code == 2) { // code 2: the bottle pool
                        context.events.onBottlePool(place, amount);
                    } else {
                        context.events.onPotionUsedAmount(place, amount, code == 1); // code 1 vs 3 selects the flag
                    }
                    return;
                }
                break;
            }
            case 21: // 23/21: pet worn item gone, with the unequip message (RecvPetEquipSlotCleared)
            case 51: { // 23/51: pet worn item gone (RecvPetEquipClear)
                const int place = reader.u8();
                const int slot = reader.u8();
                if (!reader.failed()) {
                    context.events.onPetEquipmentRemoved(place, slot, id.sub == 21);
                    return;
                }
                break;
            }
            case 22: { // 23/22: u8 pet place, u8 equipment slot, u8 bag place (RecvPetUnequipToBag)
                const int place = reader.u8();
                const int slot = reader.u8();
                const int bagSlot = reader.u8();
                if (!reader.failed()) {
                    context.events.onPetUnequipped(place, slot, bagSlot);
                    return;
                }
                break;
            }
            case 23: { // 23/23: u8 pet place, u8 bag place (RecvPetEquipFromBag)
                const int place = reader.u8();
                const int bagSlot = reader.u8();
                if (!reader.failed()) {
                    context.events.onPetEquippedFromBag(place, bagSlot);
                    return;
                }
                break;
            }
            case 24: { // 23/24: u8 pet place, a worn item's record (SetPetEquipItem)
                const int place = reader.u8();
                protocol::ItemInstance item;
                item.itemId = reader.u16();
                item.count = 1;
                item.wear = reader.u8();
                item.byteA = reader.u8();
                item.byteB = reader.u8();
                item.insertItemId = reader.u16();
                item.expiry = reader.f64();
                item.fusionLevel = reader.u8();
                item.attributeAdjust = reader.u8();
                item.lookItemId = reader.u16();
                if (!reader.failed()) {
                    context.events.onPetEquipmentSet(place, item);
                    return;
                }
                break;
            }
            case 140: // 23/140: thing wear update or expiry, with a kind byte
            case 168: { // 23/168: teleport items' wear update
                if (id.sub == 140) {
                    const int kind = reader.u8();
                    if (kind == 2) { // kind 2: an item expired
                        const int container = reader.u8();
                        const int place = reader.u8();
                        const int pet = reader.u8();
                        const int replacement = reader.u16();
                        int itemId = 0;
                        std::string owner;
                        if (container == 5 || container == 6) { // containers 5 and 6 carry an item and the owner's name
                            itemId = reader.u16();
                            owner = reader.rest();
                            if (!owner.empty()) {
                                owner.pop_back();
                            }
                        }
                        if (!reader.failed()) {
                            context.events.onThingExpired(container, place, pet, replacement, itemId, owner);
                            return;
                        }
                        break;
                    }
                    if (kind != 1) { // kind 1: wear update (RecvThingWearUpdate)
                        break;
                    }
                }
                const int container = reader.u8();
                const int slot = reader.u8();
                const std::int32_t wear = reader.i32();
                const int petPlace = reader.u8();
                if (!reader.failed()) {
                    context.events.onThingWear(container, slot, wear, petPlace);
                    return;
                }
                break;
            }
            case 217: // 23/217: picnic messages
                context.events.onPicnicMessage(payload);
                return;
            case 218: // 23/218: exp box messages
                context.events.onExpBoxMessage(payload);
                return;
            case 200: { // 23/200: i32 rate: exchange rate
                const std::int32_t rate = reader.i32();
                if (!reader.failed()) {
                    context.events.onExchangeRate(rate);
                    return;
                }
                break;
            }
            case 201: { // 23/201: i32 gold, i32 gained: exchange gold
                const std::int32_t gold = reader.i32();
                const std::int32_t gained = reader.i32();
                if (!reader.failed()) {
                    context.events.onExchangeGold(gold, gained);
                    return;
                }
                break;
            }
            case 202: { // 23/202: u8 kind, i32 player, i32 value: exchange fee
                const int kind = reader.u8();
                const std::int32_t player = reader.i32();
                const std::int32_t value = reader.i32();
                if (!reader.failed()) {
                    context.events.onExchangeFee(kind, player, value);
                    return;
                }
                break;
            }
            case 104: // 23/104: puppet packet
                if (const auto packet = protocol::PuppetPacket::decode(payload)) {
                    context.events.onPuppetPacket(*packet);
                    return;
                }
                break;
            case 107: // 23/107: u8 code: packing result
            case 205: { // 23/205: u8 code: packing result
                const int code = reader.u8();
                if (!reader.failed()) {
                    context.events.onPackingResult(code);
                    return;
                }
                break;
            }
            case 108: // 23/108: gift card messages
                context.events.onGiftCardMessage(payload);
                return;
            case 166: // 23/166: refer result
                context.events.onReferResult(payload);
                return;
            case 167: // 23/167: form the original leaves before showing
                return;
            case 105: { // 23/105: sewing kit / magic box result (RecvSewingKitResult, RecvMagicBoxResult)
                const int code = reader.u8();
                int slot = 0;
                int look = 0;
                if (code == 1) { // code 1: success, carries bag place and look
                    slot = reader.u8();
                    look = reader.u16();
                }
                if (reader.failed()) {
                    break;
                }
                if (code == 3 || code == 4) { // codes 3 and 4: magic box result
                    context.events.onMagicBoxResult(code);
                } else {
                    context.events.onSewingKitResult(code, slot, look);
                }
                return;
            }
            case 215: // 23/215: bless balloon result (RecvBlessBalloonResult)
            case 209: { // 23/209: love flower result (RecvLoveFlowerResult)
                const int outcome = reader.u8();
                std::int32_t sender = 0;
                std::int32_t target = 0;
                std::string text;
                if (id.sub == 209 && outcome == 0) { // 209 outcome 0: u8 why
                    sender = reader.u8();
                } else if (id.sub == 215 ? outcome == 4 : outcome == 1 || outcome == 2) { // outcomes that carry sender, target and text
                    sender = reader.i32();
                    target = reader.i32();
                    text = reader.rest();
                }
                if (reader.failed()) {
                    break;
                }
                if (id.sub == 215) { // 215 = bless balloon
                    context.events.onBlessBalloonResult(outcome, sender, target, text);
                } else {
                    context.events.onLoveFlowerResult(outcome, sender, target, text);
                }
                return;
            }
            case 210: { // 23/210: reinforce result (RecvReinforceResult)
                const int code = reader.u8();
                int slot = 0;
                int level = 0;
                if (code == 1) { // code 1: success, carries bag place and fusion level
                    slot = reader.u8();
                    level = reader.u8();
                }
                if (!reader.failed()) {
                    context.events.onReinforceResult(code, slot, level);
                    return;
                }
                break;
            }
            case 214: { // 23/214: confuse stone result (RecvConfuseStoneResult)
                const int outcome = reader.u8();
                const int step = reader.u8();
                const int slot = reader.u8();
                const int stone = reader.u16();
                if (!reader.failed()) {
                    if (outcome == 1) { // outcome 1: success
                        context.events.onConfuseStoneResult(step, slot, stone);
                    }
                    return;
                }
                break;
            }
            case 143: { // 23/143: dismantle result (RecvDismantleResult)
                const int container = reader.u8();
                const int slot = reader.u8();
                const std::int32_t wear = reader.i32();
                const int petPlace = reader.u8();
                if (!reader.failed()) {
                    context.events.onDismantled(container, slot, wear, petPlace);
                    return;
                }
                break;
            }
            case 80: { // 23/80: magnet request or result (RecvMagnetRequest, RecvMagnetResult)
                const int kind = reader.u8();
                if (kind == 1 || kind == 2) { // kinds 1 and 2: request with i32 player
                    const std::int32_t player = reader.i32();
                    const int place = kind == 2 ? reader.u8() : 0; // kind 2 carries a bag place
                    if (!reader.failed()) {
                        context.events.onMagnetNotice(kind, player, place);
                        return;
                    }
                } else if (kind == 3) { // kind 3: result with u8 code
                    const int code = reader.u8();
                    if (!reader.failed()) {
                        context.events.onMagnetNotice(kind, 0, code);
                        return;
                    }
                }
                break;
            }
            case 144: { // 23/144: i32 Garage cell, u8 set flag, u16 item: Show Platform exhibit
                const int cell = reader.i32();
                reader.u8();
                const int itemId = reader.u16();
                if (!reader.failed()) {
                    context.events.onExhibitState(cell, itemId);
                    return;
                }
                break;
            }
            case 169: { // 23/169: u8 slot: magic map release
                const int slot = reader.u8();
                if (!reader.failed()) {
                    context.events.onMagicMapRelease(slot);
                    return;
                }
                break;
            }
            case 27: // 23/27: wear of a worn item (RecvEquipWear)
            case 28: { // 23/28: wear of a pet's worn item, pet place first (RecvPetEquipWear)
                const int place = id.sub == 28 ? reader.u8() : 0; // sub 28 carries the pet place
                const int slot = reader.u8();
                const int wear = reader.u8();
                if (!reader.failed()) {
                    context.events.onEquipmentWear(place, slot, wear);
                    return;
                }
                break;
            }
            case 26: { // 23/26: u16 item, u8 kind: used up (RecvItemDecreaseNotice)
                const int itemId = reader.u16();
                const int kind = reader.u8();
                if (!reader.failed()) {
                    context.events.onItemDecreased(itemId, kind);
                    return;
                }
                break;
            }
            case 36: // 23/36: the sound of getting an item
                context.events.onItemSound();
                return;
            case 71: // 23/71: no eggs in the Fortune Egg machine
                context.events.onNoticeMessage("No eggs in the Fortune Egg machine.", 2000); // notice display time in ms
                return;
            case 100: { // 23/100: u8 place, u16 item, u8 count records: given into the bag (RecvBagItemsGained)
                std::vector<protocol::BagItemRecord> records;
                while (reader.remaining() >= 4u) { // record size: u8 + u16 + u8
                    protocol::BagItemRecord record;
                    record.slot = reader.u8();
                    record.item.itemId = reader.u16();
                    record.item.count = reader.u8();
                    records.push_back(record);
                }
                context.events.onBagItemsGained(records);
                return;
            }
            case 101: { // 23/101: u16 item, u8 count records: presents sent (RecvBagFullGiftNotice)
                std::vector<protocol::ItemInstance> items;
                while (reader.remaining() >= 3u) { // record size: u16 + u8
                    protocol::ItemInstance item;
                    item.itemId = reader.u16();
                    item.count = reader.u8();
                    items.push_back(item);
                }
                context.events.onBagFullGifts(items);
                return;
            }
            case 135: { // 23/135: i32 player, u8 pet place, i32 exp: exp increased
                const std::int32_t playerId = reader.i32();
                const int place = reader.u8();
                const std::int32_t exp = reader.i32();
                if (!reader.failed()) {
                    context.events.onExpIncreased(playerId, place, exp);
                    return;
                }
                break;
            }
            case 139: { // 23/139: u8 kind (1 / 2 recharge card result, 3 u16 item glows)
                const int kind = reader.u8();
                if (kind == 1 || kind == 2) { // kinds 1 and 2: purchase result
                    const std::string text = kind == 1 ? "Purchase recharge card successfully" : "Purchase recharge card unsuccessfully";
                    context.events.onChat(0, 0, text);
                    context.events.onNoticeMessage(text, 2000); // notice display time in ms
                    return;
                }
                const int itemId = reader.u16();
                if (kind == 3 && !reader.failed()) { // kind 3: an item glows
                    context.events.onItemGlows(itemId);
                    return;
                }
                break;
            }
            case 141: // 23/141: game not launched yet
                context.events.onNoticeMessage("This game hasn't been launched yet.", 2000); // notice display time in ms
                return;
            case 150: // 23/150: remove failed
                context.events.onNoticeMessage("Remove failed", 1000); // notice display time in ms
                return;
            case 163: // 23/163: Carnie Voucher in use
                context.events.onNoticeMessage("Carnie Voucher is being used", 1000); // notice display time in ms
                return;
            case 203: // 23/203: u8 kind: the birthday gift
                switch (reader.u8()) {
                    case 1: // kind 1: the greeting in the chat
                        context.events.onChat(0, 0, "Happy birthday!");
                        return;
                    case 2: // kind 2: gift already redeemed
                        context.events.onNoticeMessage("Has already redeemed a birthday gift.", 2000); // notice display time in ms
                        return;
                    case 3: // kind 3: not enough item space
                        context.events.onNoticeMessage("Please reserve enough item space", 2000); // notice display time in ms
                        return;
                    case 4: // kind 4: level too low
                        context.events.onNoticeMessage("Your level is not enough", 2000); // notice display time in ms
                        return;
                    default: // unknown kind: falls through to unhandled
                        break;
                }
                break;
            default: // any other sub id: reported as unhandled
                break;
        }
        context.events.onUnhandled(id);
    }

}
