#include "client/models/mall/MallForge.h"

#include "client/rules/ItemInfo.h"
#include "client/data/PackingTable.h"
#include "protocol/MallMessages.h"

namespace client {

    namespace {

        constexpr int kHintMs = 1000; // short hint message time, ms
        constexpr int kResultMs = 3000; // result message time, ms
        constexpr int kBusyForge = 4; // bag cell busy kind 4: the item is being forged
        constexpr int kCantForgeItem = 18071; // item 18071: Lover's Bouquet (can never be forged)
        constexpr int kNoForgeFlag = 512; // item flags bit 512: the item can't be forged
        constexpr int kUpperLimit = 100; // highest fusion level
        constexpr int kWeakAttribute = 250; // attribute kind 250: none worth forging (counts as no attribute)
        // wrap width 32 characters, no timeout, no timeout action, not exclusive
        const TalkDialog::Timing kQuestion{32, 0, {}, false};

        /// @brief Tells whether a bag place number is valid.
        /// @param slot Place number.
        /// @return True when in 1..50.
        bool validSlot(int slot) { return slot >= 1 && slot <= BagMarks::kSlots; }

        /// @brief The item's name with its fusion level.
        /// @details Original: GetItemTableName and the level.
        /// @param items Item table.
        /// @param item The item.
        /// @return The text, like "[ Name+3 ] ".
        std::string forgedName(const formats::ItemTable& items, const protocol::ItemInstance& item) {
            const auto record = items.find(item.itemId);
            const std::string name = record ? record->name : std::string();
            return "[ " + name + (item.fusionLevel == 0 ? std::string() : "+" + std::to_string(item.fusionLevel)) + " ] ";
        }

        /// @brief The checks of gear that is forged with points.
        /// @param record Item record.
        /// @return The message the game gives, empty when fine.
        std::string unforgeable(const formats::ItemRecord& record) {
            // only equipment slots 1..5 can be forged (6 is special)
            if (record.equipSlot < 1 || record.equipSlot > 5) {
                return "The item can't be forged";
            }
            if (record.petNpcId != 0) {
                return "Professional equipment can't be forged.";
            }
            if ((record.flags & kNoForgeFlag) == kNoForgeFlag) {
                return "The item can't be forged";
            }
            const auto weak = [](int kind) { return kind == 0 || kind == kWeakAttribute; };
            if (weak(record.attributeKinds[0]) && weak(record.attributeKinds[1])) {
                return "The item can't be forged";
            }
            return {};
        }

    }

    MallForge::MallForge(
        GameServices& services,
        Inventory& inventory,
        BagMarks& marks,
        const MallShop& shop,
        PlayerMessages& messages,
        TalkDialog& talk,
        std::function<void()> changed
    )
        : m_services(services),
          m_inventory(inventory),
          m_marks(marks),
          m_shop(shop),
          m_messages(messages),
          m_talk(talk),
          m_changed(std::move(changed)) {}

    std::string MallForge::levelText() const {
        if (m_bagSlot == 0) {
            return "0";
        }
        return m_costText.empty() ? "+" + std::to_string(m_item.fusionLevel) : m_costText;
    }

    void MallForge::select(int bagSlot) {
        m_requestSent = false;
        if (m_sent || !validSlot(bagSlot)) {
            return;
        }
        if (m_bagSlot == bagSlot && m_marks.useMark(bagSlot) == BagMarks::kForge) {
            m_messages.show("Located", kHintMs);
            return;
        }
        m_strengthen = false;
        m_costText.clear();
        if (m_bagSlot != 0) {
            clear();
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        if (item.itemId == 0 || item.count != 1) {
            return;
        }
        const auto record = m_services.items.find(item.itemId);
        if (!record) {
            return;
        }
        if (record->id == kCantForgeItem) {
            m_messages.show(record->name + "Cannot be forged", kResultMs);
            return;
        }
        if (isMallEquipment(*record)) {
            m_strengthen = true;
        } else if (const std::string refused = unforgeable(*record); !refused.empty()) {
            m_messages.show(refused, kHintMs);
            return;
        }
        if (usePeriod(*record) != 0) {
            m_messages.show("Temporary items can't be forged", kHintMs);
            return;
        }
        if (const auto notice = m_marks.lockNotice(bagSlot, 0)) {
            m_messages.show(notice->text, notice->ms);
            return;
        }
        if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show("Unable to operate when the item is locked", kHintMs);
            return;
        }
        m_bagSlot = bagSlot;
        m_marks.setUseMark(bagSlot, BagMarks::kForge);
        m_item = item;
        if (!m_strengthen) {
            m_cost = 3; // IM points a forge costs
        } else if (const auto step = PackingTable::find(record->shopPicId, record->id)) {
            m_costText = "+" + std::to_string(step->level);
        }
        m_changed();
    }

    void MallForge::release(int bagSlot) {
        if (m_sent || !validSlot(bagSlot) || m_bagSlot != bagSlot || m_marks.useMark(m_bagSlot) != BagMarks::kForge) {
            return;
        }
        clear();
        m_changed();
    }

    void MallForge::cancel() {
        if (!m_sent) {
            clear();
            m_changed();
        }
    }

    void MallForge::clear() {
        m_item = {};
        if (validSlot(m_bagSlot)) {
            if (m_marks.useMark(m_bagSlot) == BagMarks::kForge) {
                m_marks.setUseMark(m_bagSlot, 0);
            }
            if (m_marks.busyKind(m_bagSlot) == kBusyForge) {
                m_marks.setBusyKind(m_bagSlot, 0);
            }
        }
        m_bagSlot = 0;
        m_cost = 0;
        m_strengthen = false;
        m_costText.clear();
    }

    void MallForge::confirm() {
        if (m_cost == 0 && !m_strengthen) {
            return;
        }
        if (m_strengthen) {
            const auto record = m_services.items.find(m_inventory.bagItem(m_bagSlot).itemId);
            const auto step = record ? PackingTable::find(record->shopPicId, record->id) : std::nullopt;
            if (!step) {
                return;
            }
            if (!step->canStrengthen) {
                m_messages.show("Has reached the strong upper limit", kHintMs);
                return;
            }
            if (m_inventory.freeSlotFor(11101) == 0) { // item id 11101, the scroll's result item
                m_messages.show("It's full", kHintMs);
                return;
            }
            m_talk.ask(
                "[ " + record->name + " ] Enhance\rStrong Scroll is needed. #R " + std::to_string(step->level + 1) + " /#R Pieces\rAre you sure?",
                std::nullopt,
                [this] { send(); },
                {},
                kQuestion
            );
            return;
        }
        if (!m_requestSent) {
            m_talk.ask(
                "IM Points :" + std::to_string(m_shop.points) + "\rThe forging will cost " + std::to_string(m_cost) + " points\rAre you sure?",
                std::nullopt,
                [this] { send(); },
                {},
                kQuestion
            );
        } else {
            send();
        }
    }

    void MallForge::send() {
        if (m_sent || !validSlot(m_bagSlot)) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(m_bagSlot);
        if (item.itemId == 0 || item.count != 1) {
            return;
        }
        const auto record = m_services.items.find(item.itemId);
        if (!record) {
            return;
        }
        if (!isMallEquipment(*record)) {
            if (const std::string refused = unforgeable(*record); !refused.empty()) {
                m_messages.show(refused, kHintMs);
                return;
            }
            if (item.fusionLevel + 1 > kUpperLimit) {
                m_messages.show("Reached the forge upper limit", kHintMs);
                return;
            }
            if (m_shop.points < m_cost) {
                m_messages.show("Not enough points", kHintMs);
                return;
            }
        }
        if (const auto notice = m_marks.lockNotice(m_bagSlot, BagMarks::kForge)) {
            m_messages.show(notice->text, notice->ms);
            return;
        }
        if (!m_marks.isAvailable(m_bagSlot)) {
            m_messages.show("Unable to operate when the item is locked", kHintMs);
            return;
        }
        m_sent = true;
        m_services.connection.send(protocol::mallForgeRequest(m_bagSlot));
    }

    void MallForge::open() {
        if (m_sent) {
            return;
        }
        if (m_bagSlot != 0) {
            clear();
            m_changed();
        }
        m_cost = 0;
    }

    void MallForge::close() {
        if (!m_sent) {
            clear();
            m_changed();
        }
    }

    void MallForge::receive(int outcome, int bagSlot, int fusionLevel) {
        const auto reselect = [this](int slot) {
            m_sent = false;
            clear();
            select(slot);
        };
        switch (outcome) {
            case 1: // 75/6 outcome 1: fusion succeeded (new fusion level)
                if (!validSlot(bagSlot)) {
                    return;
                }
                m_messages.show(forgedName(m_services.items, m_inventory.bagItem(bagSlot)) + "Fusion succeeded", kResultMs);
                m_inventory.setFusionLevel(bagSlot, fusionLevel);
                m_services.playSound("sound/SEB0300.wav");
                reselect(bagSlot);
                m_requestSent = true;
                break;
            case 2: // outcome 2: fusion failed
                if (!validSlot(m_bagSlot)) {
                    return;
                }
                m_messages.show(forgedName(m_services.items, m_inventory.bagItem(m_bagSlot)) + "Fusion failed", kResultMs);
                reselect(m_bagSlot);
                m_requestSent = true;
                break;
            case 3: // outcome 3: the server is working on the item
                if (validSlot(m_bagSlot) && m_marks.useMark(m_bagSlot) == BagMarks::kForge && m_marks.busyKind(m_bagSlot) == 0) {
                    m_marks.setBusyKind(m_bagSlot, kBusyForge);
                }
                break;
            case 4: // outcome 4: not enough points
                m_messages.show("Not enough points,Fusion failed", kResultMs);
                reselect(m_bagSlot);
                break;
            case 5: // outcome 5: not enough Strong Scrolls
                m_messages.show("Insufficient Strong Scroll", kResultMs);
                reselect(m_bagSlot);
                break;
            case 6: // outcome 6: strengthening succeeded
                m_messages.show("Enhaced successfully", kResultMs);
                m_sent = false;
                m_services.playSound("sound/SEB0300.wav");
                clear();
                break;
            case 7: // outcome 7: strengthening failed
            case 8: // outcome 8: strengthening failed (same handling as 7)
                m_messages.show("Enchancd failed ", kResultMs);
                m_sent = false;
                clear();
                break;
            default: // unknown outcome is ignored
                return;
        }
        m_changed();
    }

}
