#include "client/models/house/HouseInterior.h"

#include "core/ByteReader.h"
#include "protocol/FurnitureBedMessages.h"
#include "protocol/FurnitureCaseMessages.h"
#include "protocol/FurnitureCupboardMessages.h"
#include "protocol/FurnitureStallItem.h"

namespace client {

    namespace {

        constexpr int kLift = 39014; // item 39014: Elevator
        constexpr int kCracker = 39149; // item 39149: New Year Cracker
        constexpr int kClock = 39152; // item 39152: Gugu Clock
        constexpr int kCrackerAnimating = 4; // animation state 4: the cracker plays
        constexpr int kClockStriking = 1; // animation state 1: the clock chimes
        constexpr int kLiftStill = 0; // animation state 0: the elevator stands still
        constexpr std::size_t kEntrySize = 2 + FurnitureRecord::kSize; // list entry: u16 piece index + 57-byte record

    }

    HouseInterior::HouseInterior(
        gfx::PictureLibrary& pictures,
        const formats::ItemTable& items,
        std::int32_t ownId,
        Hooks hooks
    )
        : m_items(items),
          m_hooks(std::move(hooks)),
          m_furniture(items),
          m_utensils(pictures, items, m_furniture),
          m_surfaces(pictures, items),
          m_pillars(m_furniture, items, m_hooks.notice),
          m_stalls(
              pictures,
              items,
              m_furniture,
              m_utensils,
              {m_hooks.markBag, m_hooks.reclaimed, m_hooks.notice, m_hooks.ownId, m_hooks.priceChanged, m_hooks.caseChanged}
          ),
          m_cupboards(items, m_utensils),
          m_carCases(m_utensils),
          m_beds(items, m_utensils) {
        m_furniture.setOwnerId(ownId);
    }

    void HouseInterior::setPlace(bool houseMap, int room) {
        m_houseMap = houseMap;
        m_room = room;
    }

    int HouseInterior::addPiece(const FurnitureRecord& record, int index, int slot, std::uint64_t nowMs) {
        if (index < 1 || index > HomeFurniture::kMost) {
            return 0;
        }
        m_furniture.place(index, record, false);
        const int placed = m_utensils.add(record, index, slot, nowMs);
        if (placed != 0 && record.itemId == FurnitureCarCases::kShowPlatformItem) {
            m_exhibit.clear();
        }
        return placed;
    }

    void HouseInterior::removePiece(int index) {
        if (index < 1 || index > HomeFurniture::kMost) {
            return;
        }
        m_furniture.remove(index);
        m_utensils.remove(index);
    }

    void HouseInterior::retainPiece(int index, int flag) {
        FurnitureRecord record = m_furniture.piece(index);
        if (record.itemId == 0 || record.retained == flag) {
            return;
        }
        removePiece(index);
        if (flag != 0) {
            record.cellX = 0;
            record.cellY = 0;
            record.cellZ = 0;
            record.retained = flag;
            m_furniture.place(index, record, false);
        }
    }

    void HouseInterior::placeRetained(const core::Bytes& payload, std::uint64_t nowMs) {
        core::ByteReader reader(payload, 1);
        reader.u8();
        const int index = reader.u16();
        FurnitureRecord record = m_furniture.piece(index);
        record.cellX = reader.i32();
        record.cellY = reader.i32();
        record.cellZ = reader.i32();
        record.rotation = reader.u8();
        record.room = reader.u8();
        record.retained = 0;
        if (reader.failed() || record.itemId == 0) {
            return;
        }
        removePiece(index);
        const int slot = addPiece(record, index, 0, nowMs);
        if (slot != 0) {
            m_utensils.applyStackOffset(slot);
            m_stalls.relink(index);
            m_cupboards.relink(index);
            m_beds.relink(index);
        }
    }

    void HouseInterior::deliverPiece(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skips the first 2 payload bytes (action and sub ids)
        const int index = reader.u16();
        FurnitureRecord record;
        record.retained = reader.u8();
        record.itemId = reader.u16();
        const int flag = reader.u8();
        if (reader.failed()) {
            return;
        }
        if (const auto item = m_items.find(record.itemId)) {
            m_hooks.notice(item->name + " It is deliverd to the tent and furniture pick-up area");
        }
        if (m_furniture.piece(index).itemId != 0) {
            bool any = false;
            for (int piece = 1; piece <= HomeFurniture::kMost; ++piece) {
                any = any || (m_furniture.piece(piece).itemId != 0 && m_furniture.piece(piece).retained == 1);
            }
            if (m_furniture.ownerId() == m_hooks.ownId() && m_houseMap && !any) {
                m_furniture.remove(index);
                m_furniture.place(index, record, false);
            }
        } else if (flag == 1) {
            m_furniture.place(index, record, false);
        }
    }

    void HouseInterior::receiveList(const core::Bytes& payload, std::uint64_t nowMs) {
        m_utensils.clear();
        m_stalls.clear();
        m_cupboards.clear();
        m_carCases.clear();
        m_beds.clear();
        m_surfaces.clear();
        m_furniture.clear();
        m_pillars.clear();
        if (inFloorRoom()) {
            m_furniture.setRoomArea(HouseRoomRect::ofSize(m_floorRoomSizes[0]), 0);
            m_furniture.setRoomArea(HouseRoomRect::ofSize(m_floorRoomSizes[1]), 1);
        }
        core::ByteReader reader(payload, 2); // skips the first 2 payload bytes (action and sub ids)
        m_furniture.setOwnerId(reader.i32());
        while (!reader.failed() && reader.remaining() >= kEntrySize) {
            const int index = reader.u16();
            const FurnitureRecord record = FurnitureRecord::read(reader);
            addPiece(record, index, 0, nowMs);
            m_cupboards.relink(index);
            m_beds.relink(index);
        }
    }

    bool HouseInterior::receiveRoomSize(const core::Bytes& payload) {
        if (!inFloorRoom()) {
            return false;
        }
        core::ByteReader reader(payload, 2); // skips the first 2 payload bytes (action and sub ids)
        const int size = reader.u8();
        const int floor = reader.u8();
        if (reader.failed() || floor < 0 || floor >= kFloorRooms) {
            return false;
        }
        m_furniture.setRoomArea(HouseRoomRect::ofSize(size), floor);
        m_floorRoomSizes[static_cast<std::size_t>(floor)] = size;
        return floor == m_room;
    }

    void HouseInterior::receiveFurniture(const core::Bytes& payload, std::uint64_t nowMs) {
        core::ByteReader reader(payload, 1);
        const int sub = reader.u8();
        switch (sub) {
            case 1: { // 62/1 server->client: a piece put in the house (u16 index, record)
                const int index = reader.u16();
                const FurnitureRecord record = FurnitureRecord::read(reader);
                if (!reader.failed() && m_furniture.spanOf(record.itemId, 0)) {
                    const int slot = addPiece(record, index, 0, nowMs);
                    if (slot != 0) {
                        m_utensils.applyStackOffset(slot);
                    }
                }
                break;
            }
            case 2: { // 62/2 server->client: a piece taken away (u16 index)
                const int index = reader.u16();
                if (!reader.failed()) {
                    removePiece(index);
                }
                break;
            }
            case 3: { // 62/3 server->client: a piece moved or turned (u16 index, 3 x i32 cell, u8 facing)
                const int index = reader.u16();
                FurnitureRecord record = m_furniture.piece(index);
                record.cellX = reader.i32();
                record.cellY = reader.i32();
                record.cellZ = reader.i32();
                record.rotation = reader.u8();
                const int slot = m_utensils.slotOfPiece(index);
                if (reader.failed() || slot == 0 || m_utensils.utensil(slot).itemId == 0) {
                    break;
                }
                removePiece(index);
                addPiece(record, index, slot, nowMs);
                m_utensils.applyStackOffset(m_utensils.slotOfFurniture(index));
                m_stalls.relink(index);
                break;
            }
            case 4: // 62/4 server->client: the whole furniture list (owner id, then the pieces)
                receiveList(payload, nowMs);
                break;
            case 5: { // 62/5 server->client: a tool put on a Work Platform (u16 index, u16 item, u16 platform piece)
                const int index = reader.u16();
                FurnitureRecord record;
                record.itemId = reader.u16();
                const int platform = reader.u16();
                if (!reader.failed()) {
                    m_furniture.place(index, record, true);
                    m_furniture.link(index, platform);
                }
                break;
            }
            case 16: { // 62/16 server->client: a piece goes to or leaves the pick-up area (u16 index, u8 flag)
                const int index = reader.u16();
                const int flag = reader.u8();
                if (!reader.failed()) {
                    retainPiece(index, flag);
                }
                break;
            }
            case 17: // 62/17 server->client: a piece of the pick-up area now stands in the house
                placeRetained(payload, nowMs);
                break;
            case 52: // 62/52 server->client: a bought or rented piece is delivered to the pick-up area
                deliverPiece(payload);
                break;
            case 14: { // 62/14 server->client: the floor item (u16 item)
                const int item = reader.u16();
                m_surfaces.setFloorItem(item);
                break;
            }
            case 15: { // 62/15 server->client: the wall item (u16 item)
                const int item = reader.u16();
                m_surfaces.setWallItem(item);
                break;
            }
            case 60: { // 62/60 server->client: New Year Cracker plays (u8 floor, u8 piece)
                const int floor = reader.u8();
                const int index = reader.u8();
                const int slot = m_utensils.slotOfFurniture(index);
                if (floor == m_room && m_utensils.utensil(slot).itemId == kCracker) {
                    m_utensils.setAnimState(slot, kCrackerAnimating);
                    m_hooks.sound("sound\\SEB0017.wav");
                }
                break;
            }
            case 61: { // 62/61 server->client: option 1 is the Gugu clock chime (u8 floor, u8 piece)
                if (reader.u8() == 1) {
                    const int floor = reader.u8();
                    const int index = reader.u8();
                    const int slot = m_utensils.slotOfFurniture(index);
                    if (floor == m_room && m_utensils.utensil(slot).itemId == kClock) {
                        m_utensils.setAnimState(slot, kClockStriking);
                        m_hooks.sound("sound\\wav1510.wav");
                    }
                }
                break;
            }
            case 21: // 62/21 server->client: an item put on a shelf or into a vending machine
                if (const auto stalled = protocol::decodeFurnitureStallItem(payload)) {
                    m_stalls.put(*stalled);
                }
                break;
            case 22: { // 62/22 server->client: a stalled item taken back (i32 seller, u16 number, u16 piece)
                const std::int32_t owner = reader.i32();
                const int number = reader.u16();
                const int piece = reader.u16();
                if (!reader.failed()) {
                    m_stalls.remove(owner, number, piece);
                }
                break;
            }
            case 24: { // 62/24 server->client: some of a stalled item sold (i32 seller, u16 number, u8 count, u16 piece)
                const std::int32_t owner = reader.i32();
                const int number = reader.u16();
                const int count = reader.u8();
                const int piece = reader.u16();
                if (!reader.failed()) {
                    m_stalls.take(owner, number, count, piece);
                }
                break;
            }
            case 25: // 62/25 server->client: why a shelf, bed or egg machine refused (u8 code)
                m_stalls.refuse(reader.u8());
                break;
            case 31: { // 62/31 server->client: a stack put in a Fortune Egg machine (i32 seller, u16 piece, u8 place, u16 item, u8 count, u8 number)
                const std::int32_t seller = reader.i32();
                const int piece = reader.u16();
                const int place = reader.u8();
                const int itemId = reader.u16();
                const int count = reader.u8();
                const int number = reader.u8();
                if (!reader.failed() && inFloorRoom()) {
                    m_stalls.putContent(seller, piece, place, itemId, count, number);
                }
                break;
            }
            case 32: { // 62/32 server->client: what a play of an egg machine costs (u16 piece, i32 price)
                const int piece = reader.u16();
                const std::int32_t price = reader.i32();
                if (!reader.failed()) {
                    m_stalls.setSalePrice(piece, price);
                }
                break;
            }
            case 33: { // 62/33 server->client: a play sold one egg (u16 piece, u8 place)
                const int piece = reader.u16();
                const int place = reader.u8();
                if (!reader.failed()) {
                    m_stalls.eggSold(piece, place);
                }
                break;
            }
            case 34: { // 62/34 server->client: how many eggs a machine holds (u16 piece, u8 count)
                const int piece = reader.u16();
                const int count = reader.u8();
                if (!reader.failed()) {
                    m_stalls.setEggCount(piece, count);
                }
                break;
            }
            case 41: // 62/41 server->client: an item put in a place of a cupboard
                if (const auto stored = protocol::decodeFurnitureCupboardItem(payload)) {
                    m_cupboards.put(*stored);
                }
                break;
            case 42: { // 62/42 server->client: an item left its cupboard place (u16 piece, u8 place, u8 item number)
                const int piece = reader.u16();
                const int place = reader.u8();
                const int id = reader.u8();
                if (!reader.failed()) {
                    m_cupboards.remove(piece, place, id);
                }
                break;
            }
            case 47: // 62/47 server->client: a vehicle parked in a Garage cell
                if (const auto stored = protocol::decodeFurnitureCupboardItem(payload)) {
                    m_carCases.put(*stored);
                    if (m_hooks.carParked) {
                        m_hooks.carParked(*stored);
                    }
                }
                break;
            case 48: { // 62/48 server->client: a vehicle left its Garage cell (u16 piece, u8 cell, u8 vehicle number)
                const int piece = reader.u16();
                const int cell = reader.u8();
                const int id = reader.u8();
                if (!reader.failed() && m_carCases.remove(piece, cell, id) && m_hooks.carLeft) {
                    m_hooks.carLeft(cell);
                }
                break;
            }
            case 54: { // 62/54 server->client: the Show Platform answers its click (u8 restrained state)
                const int restrained = reader.u8();
                if (!reader.failed()) {
                    const bool garage = m_utensils.standsIn(FurnitureCarCases::kGarageItem);
                    if (!garage) {
                        m_hooks.notice("Using the show platform with the garage");
                    }
                    if (m_hooks.platformOpened) {
                        m_hooks.platformOpened(restrained, garage);
                    }
                }
                break;
            }
            case 55: { // 62/55 server->client: a vehicle goes on show on a platform or comes down (i32 owner, i32 cell, u16, u16 item)
                const std::int32_t owner = reader.i32();
                const int cell = reader.i32();
                reader.u16();
                const int itemId = reader.u16();
                if (!reader.failed()) {
                    m_exhibit.setItem(itemId);
                    if (owner == m_hooks.ownId()) {
                        m_exhibit.setCell(cell);
                        m_hooks.notice(cell != 0 ? "Set the item on the show platform" : "Get the item back on the show platform");
                        if (m_hooks.exhibitChosen) {
                            m_hooks.exhibitChosen(cell != 0);
                        }
                    }
                }
                break;
            }
            case 51: { // 62/51 server->client: the Garage answers its click (u8 restrained state)
                const int restrained = reader.u8();
                if (!reader.failed() && m_hooks.garageOpened) {
                    m_hooks.garageOpened(restrained);
                }
                break;
            }
            case 43: // 62/43 server->client: a pet put to sleep in a bed
                if (const auto stored = protocol::decodeFurnitureBedPet(payload)) {
                    m_beds.put(*stored);
                    if (m_hooks.petSlept) {
                        m_hooks.petSlept();
                    }
                }
                break;
            case 44: { // 62/44 server->client: a pet woke (u16 piece, u8 bed, u8 pet number, u8 team place)
                const int piece = reader.u16();
                const int bed = reader.u8();
                const int id = reader.u8();
                const int petPlace = reader.u8();
                if (!reader.failed()) {
                    if (const auto pet = m_beds.release(piece, bed, id); pet && m_hooks.petReturned) {
                        m_hooks.petReturned(*pet, petPlace);
                    }
                }
                break;
            }
            case 58: { // 62/58 server->client: display case, option 1 a case changed, option 2 so many sold
                const int option = reader.u8();
                if (option == 1) {
                    if (const auto update = protocol::decodeFurnitureCaseUpdate(payload)) {
                        m_stalls.updateCase(*update, m_houseMap);
                    }
                } else if (option == 2) { // option 2: so many of a case sold
                    if (const auto sale = protocol::decodeFurnitureCaseSale(payload)) {
                        m_stalls.sellCase(*sale, m_houseMap);
                    }
                }
                break;
            }
            case 45: // 62/45 server->client: the player's doorplate is gone (i32 player id)
                m_doorplates.clear(reader.i32());
                break;
            case 46: // 62/46 server->client: the list of doorplates
                m_doorplates.receiveList(payload);
                break;
            case 255: { // 62/255 server->client: option 1 the lift stops (u16 piece)
                if (reader.u8() == 1) {
                    const int slot = m_utensils.slotOfFurniture(reader.u16());
                    if (m_utensils.utensil(slot).itemId == kLift) {
                        m_utensils.setAnimState(slot, kLiftStill);
                    }
                }
                break;
            }
            default: // any other 62 sub id: ignored
                break;
        }
    }

}
