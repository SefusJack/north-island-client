#include "client/rules/BagRequests.h"

#include "client/rules/ItemInfo.h"
#include "client/rules/Equipping.h"
#include "client/rules/Stalling.h"
#include "client/data/StarterRoles.h"
#include "protocol/ItemRequests.h"

namespace client {

    namespace {

        constexpr int kInstrument = 29; // item type 29: placeable machines / instruments (Assembly Basin, Auto Loom, Carpenter's Lathe, ...)
        constexpr int kFurniture = 30; // item type 30: furniture (ATM, Air Condition, Ancient Bathtub, ...)
        constexpr int kCannotDiscard = 0x80; // item flag bit 0x80: the item cannot be discarded
        constexpr int kAskDiscard = 1; // item flag bit 0x01: ask for confirmation before discarding
        constexpr std::size_t kMostThings = 255; // most items allowed on the ground of a map
        constexpr int kBirthdayRoom = 10085; // map 10085: Wedding Hall (items cannot be dropped there)
        constexpr int kMostStack = 50; // most items in one stack
        constexpr int kMinLevel = 10; // below this level a discard asks the "Less than level 10" question
        constexpr int kMostBottleFill = 2000000; // most points a bottle (HP / SP Bottle) pool can hold
        constexpr int kShortMs = 1500; // short message display time, ms
        constexpr int kLongMs = 2000; // long message display time, ms
        const char* const kDiscardQuestion = "Discard the item to destroy it, Okay? ";

    }

    BagRequests::BagRequests(
        GameServices& services,
        const Inventory& bag,
        const BagMarks& marks,
        PlayerMessages& messages,
        CountBox& countBox,
        Hooks hooks
    )
        : m_services(services),
          m_bag(bag),
          m_marks(marks),
          m_messages(messages),
          m_countBox(countBox),
          m_hooks(std::move(hooks)) {}

    void BagRequests::moveItem(int fromSlot, int count, int toSlot) {
        if (const auto notice = m_marks.lockNotice(fromSlot, 0)) {
            m_messages.show(notice->text, notice->ms);
            return;
        }
        if (!m_marks.isAvailable(fromSlot)) {
            m_messages.show("Unable to operate when the item is locked", kLongMs);
            return;
        }
        const auto from = m_services.items.find(m_bag.bagItem(fromSlot).itemId);
        const auto to = m_services.items.find(m_bag.bagItem(toSlot).itemId);
        const int fromKind = from ? bottleKind(*from) : 0;
        const int toKind = to ? bottleKind(*to) : 0;
        const bool pour =
            (fromKind == 1 && toKind == 3) || (fromKind == 2 && toKind == 4); // bottle kinds: potion 1 into HP Bottle 3, potion 2 into SP Bottle 4
        if (pour) {
            if (m_hooks.bottlePool(toKind) >= kMostBottleFill) {
                m_messages.show("The bottle is full", kLongMs);
                return;
            }
        } else if (m_bag.fitCount(toSlot, m_bag.bagItem(fromSlot).itemId, count, fromSlot) != count) {
            return;
        }
        m_services.connection.send(protocol::moveItemRequest(fromSlot, count, toSlot));
    }

    void BagRequests::dropItem(int slot, int count) {
        if (m_hooks.swimming()) {
            m_messages.show("Don't litter, help keep our world green!", kLongMs);
            return;
        }
        if (const auto notice = m_marks.lockNotice(slot, 0)) {
            m_messages.show(notice->text, notice->ms);
            return;
        }
        if (!m_marks.isAvailable(slot)) {
            m_messages.show("Unable to operate when the item is locked", kLongMs);
            return;
        }
        if (m_hooks.groundCount() + static_cast<std::size_t>(count) > kMostThings) {
            m_messages.show("Items on the floor have reached the upper limit. Please discard later.", kShortMs);
            return;
        }
        const auto record = m_services.items.find(m_bag.bagItem(slot).itemId);
        if (!m_hooks.inHouseMap() && record && (record->type == kInstrument || record->type == kFurniture)) {
            m_messages.show("Instruments cannot be discarded on the ground", kShortMs);
            return;
        }
        if (record && (record->flags & kCannotDiscard) != 0) {
            m_messages.show("This item cannot be discarded", kShortMs);
            return;
        }
        if (m_bag.bagItem(slot).itemId == m_hooks.campItemId()) {
            m_messages.show("The tent is being used, cannot be discarded", kShortMs);
            return;
        }
        m_services.connection.send(protocol::dropItemRequest(slot, count));
    }

    void BagRequests::dropOnGround(int slot, bool ctrl) {
        if (m_hooks.mapId() == kBirthdayRoom) {
            m_messages.show("The scene can't be discarded", kLongMs);
            return;
        }
        if (ctrl) {
            askDropCount(slot);
        } else if (!m_hooks.fighting()) {
            constexpr int kVehicleType = 39; // item type 39: vehicles and their capsules (Airship, Boat, Cabriolet, ...)
            const auto record = m_services.items.find(m_bag.bagItem(slot).itemId);
            if (record && record->type == kVehicleType && m_marks.vehicleState(slot) != 0) {
                m_messages.show("You are unable to discard the item.", kShortMs);
                return;
            }
            dropItem(slot, 1);
        }
    }

    void BagRequests::askMoveCount(int fromSlot, int toSlot) {
        if (const auto notice = m_marks.lockNotice(fromSlot, 0)) {
            m_messages.show(notice->text, notice->ms);
            return;
        }
        if (!m_marks.isAvailable(fromSlot)) {
            m_messages.show("Unable to operate when the item is locked", kLongMs);
            return;
        }
        if (m_marks.vehicleState(fromSlot) != 0) {
            m_messages.show("Put item away before preserving.", kShortMs);
            return;
        }
        const protocol::ItemInstance& from = m_bag.bagItem(fromSlot);
        const protocol::ItemInstance& to = m_bag.bagItem(toSlot);
        const auto record = m_services.items.find(from.itemId);
        int count = from.count;
        if (to.count != 0) {
            if (record && isEquipment(*record)) {
                count = 0;
            } else if (from.count + to.count >= kMostStack) {
                count = kMostStack - to.count;
            }
        }
        m_countBox.ask(count, [this, fromSlot, toSlot](int asked) {
            if (asked > m_bag.bagItem(fromSlot).count) {
                return false;
            }
            if (asked > 0) {
                moveItem(fromSlot, asked, toSlot);
            }
            return true;
        });
    }

    void BagRequests::askDropCount(int slot) {
        m_countBox.ask(m_bag.bagItem(slot).count, [this, slot](int asked) {
            if (asked > m_bag.bagItem(slot).count) {
                return false;
            }
            if (asked > 0) {
                dropItem(slot, asked);
            }
            return true;
        });
    }

    void BagRequests::askDiscardEquip(int equipSlot, int petPlace) {
        if (m_hooks.groundCount() + 1 > kMostThings) {
            m_messages.show("Items on the floor have reached the upper limit. Please discard later.", kShortMs);
            return;
        }
        if (m_hooks.inHouseMap()) {
            m_messages.show("Common items cannot be discarded on the ground at home", kShortMs);
            return;
        }
        if (m_hooks.fighting()) {
            return;
        }
        const protocol::ItemInstance worn = petPlace != 0 ? m_hooks.petWorn(petPlace, equipSlot) : m_bag.equipped(equipSlot);
        const auto record = m_services.items.find(worn.itemId);
        const auto discard = [this, equipSlot, petPlace] {
            if (petPlace != 0) {
                m_services.connection.send(protocol::petDiscardEquipRequest(petPlace, equipSlot));
            } else {
                confirmDiscard(equipSlot);
            }
        };
        const auto ask = [this, &discard](const std::string& text) { m_hooks.confirm(text, discard); };
        if (record && (record->flags & kCannotDiscard) != 0) {
            m_messages.show("This item cannot be discarded", kShortMs);
        } else if (m_hooks.level() < kMinLevel) {
            ask(std::string("Less than level 10\r") + kDiscardQuestion);
        } else if (!m_hooks.enoughOnlineTime()) {
            ask(std::string("The online time is not enough\xA1I\r") + kDiscardQuestion);
        } else if ((record && (record->flags & kAskDiscard) != 0) || worn.lookItemId != 0 || isNonTransferable(worn.insertItemId)) {
            ask(kDiscardQuestion);
        } else if (petPlace != 0) {
            m_services.connection.send(protocol::petDiscardEquipRequest(petPlace, equipSlot));
        } else {
            m_services.connection.send(protocol::discardEquipRequest(equipSlot));
        }
    }

    void BagRequests::confirmDiscard(int equipSlot) {
        constexpr int kClothes = 1; // index 1 of the starter equipment list (headgear, clothes, ...): the newbie clothes
        if (m_bag.equipped(equipSlot).itemId == starterEquipment(m_hooks.roleIndex())[kClothes]) {
            m_hooks.confirm("Discarding the newbie set will cause #R#B cannot use /#R/#B newbie skill, are you sure? ", [this, equipSlot] {
                m_services.connection.send(protocol::discardEquipRequest(equipSlot));
            });
            return;
        }
        m_services.connection.send(protocol::discardEquipRequest(equipSlot));
    }

}
