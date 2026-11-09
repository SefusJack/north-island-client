#include "client/models/pets/RebornWish.h"

#include "client/rules/RebornWishPrice.h"
#include "core/ByteReader.h"
#include "protocol/MessageId.h"
#include "protocol/RebornWishRequests.h"

#include <vector>

namespace client {

    namespace {

        constexpr int kWishingGod = 14108; // NPC 14108: Abba (the wishing god)
        constexpr int kStarItem = 30025; // item 30025: Star
        constexpr int kNoPetsTalk = 31314; // talk text id said when the player has no pets
        constexpr int kMaxStarSlots = 50; // bag cells to search for Stars (the whole bag)

    }

    RebornWish::RebornWish(
        GameServices& services,
        const Inventory& inventory,
        TalkDialog& talk,
        Hooks hooks
    )
        : m_services(services),
          m_inventory(inventory),
          m_talk(talk),
          m_hooks(std::move(hooks)) {}

    std::optional<TalkDialog::Speaker> RebornWish::speaker() const {
        const auto god = m_services.npcs.find(kWishingGod);
        if (!god) {
            return std::nullopt;
        }
        return TalkDialog::Speaker{*god, god->name, true};
    }

    void RebornWish::receive(const core::Bytes& payload) {
        switch (protocol::MessageId::of(payload).sub) {
            case 11: // 66/11: wish list with extra bytes per entry (RecvRebornWishList)
                receiveList(payload, true);
                return;
            case 12: // 66/12: opens the form (ShowCtrl)
                m_open = true;
                return;
            case 13: // 66/13: a pet was wished for, remove it from the list (RecvRebornWishRemoved)
                receiveRemoved(payload);
                return;
            case 14: // 66/14: wish list of grow keys only (RecvRebornWishIds)
                receiveList(payload, false);
                return;
            case 15: // 66/15: show the pick box "Make a wish for who?" (ShowRebornWishChoice)
                showChoice();
                return;
            case 16: // 66/16: greeting talk (ShowRebornWishGreeting)
                say("God! #n/#n want to make a wish");
                return;
            default: // other sub ids are ignored
                return;
        }
    }

    void RebornWish::receiveList(const core::Bytes& payload, bool extra) {
        m_wishes.fill(0);
        core::ByteReader reader(payload, 2); // skips the 2 message id bytes
        const int count = reader.u8();
        for (int i = 1; i <= count && i <= kListSize && !reader.failed(); ++i) {
            const int growKey = reader.u8();
            if (extra) {
                reader.skip(2 + 4 + 4); // two bytes and two 32-bit integers
            }
            const auto npc = m_services.npcs.findByGrowKey(growKey);
            m_wishes[static_cast<std::size_t>(i)] = npc ? npc->npcId : 0;
        }
    }

    void RebornWish::receiveRemoved(const core::Bytes& payload) {
        if (!m_open) {
            return;
        }
        core::ByteReader reader(payload, 2); // skips the 2 message id bytes
        const int npcId = reader.u16();
        if (reader.failed()) {
            return;
        }
        int found = 0;
        for (int i = 1; i <= kListSize; ++i) {
            if (m_wishes[static_cast<std::size_t>(i)] == npcId) {
                found = i;
                break;
            }
        }
        if (found == 0) {
            return;
        }
        m_wishes[static_cast<std::size_t>(found)] = 0;
        for (int i = found; i < kListSize; ++i) {
            if (m_wishes[static_cast<std::size_t>(i + 1)] == 0) {
                break;
            }
            m_wishes[static_cast<std::size_t>(i)] = m_wishes[static_cast<std::size_t>(i + 1)];
            m_wishes[static_cast<std::size_t>(i + 1)] = 0;
        }
    }

    void RebornWish::say(const std::string& text) {
        TalkDialog::Timing timing;
        timing.modal = true;
        m_talk.say(text, speaker(), {}, std::move(timing));
    }

    void RebornWish::showChoice() {
        m_open = true;
        std::vector<std::string> names;
        for (int i = 1; i <= kListSize; ++i) {
            if (const auto npc = m_services.npcs.find(m_wishes[static_cast<std::size_t>(i)]); m_wishes[static_cast<std::size_t>(i)] != 0 && npc) {
                names.push_back(npc->name);
            }
        }
        if (names.empty()) {
            say(m_services.talks.text(kNoPetsTalk));
            close();
            return;
        }
        TalkDialog::Timing timing;
        timing.wrapChars = 32; // characters per line of the request box
        m_talk.choose("Make a wish for who\xA1H", speaker(), names, [this](int row) { pick(row); }, [this] { close(); }, std::move(timing));
    }

    void RebornWish::pick(int row) {
        m_wishIndex = row + 1;
        const auto npc = m_wishIndex <= kListSize ? m_services.npcs.find(m_wishes[static_cast<std::size_t>(m_wishIndex)]) : std::nullopt;
        if (!npc || npc->growKey == 0) {
            close();
            return;
        }
        if (!m_hooks.petRoom()) {
            close();
            m_hooks.message("Pet NPC full", 2000); // message time in ms
            return;
        }
        m_talk.choose(
            "The cost of making wish" + std::to_string(cost(*npc)) + "Stars, do you really want to\xA1H",
            speaker(),
            {"Yes", "No"},
            [this](int choice) { confirm(choice); },
            [this] { close(); }
        );
    }

    int RebornWish::cost(const formats::NpcInfo& npc) const {
        int stars = rebornStarCost(npc.growKey);
        if (const int markId = petCareerMarkId(npc.npcId); markId != 0) {
            const auto mark = m_services.marks.find(markId);
            if (mark && mark->get().flagBit != 0 && m_hooks.hasMarkFlag(mark->get().flagBit)) {
                stars = (stars + 1) / 2;
            }
        }
        return stars;
    }

    void RebornWish::confirm(int choice) {
        if (choice != 0) {
            close();
            return;
        }
        const auto npc = m_wishIndex <= kListSize ? m_services.npcs.find(m_wishes[static_cast<std::size_t>(m_wishIndex)]) : std::nullopt;
        int stars = npc ? cost(*npc) : 0;
        if (stars == 0) {
            close();
            return;
        }
        int slot = m_inventory.nextSlotWith(kStarItem, 0);
        if (slot == 0) {
            say("Not enough stars");
            m_wishIndex = 0;
            close();
            return;
        }
        int tries = 0;
        do {
            ++tries;
            if (m_inventory.bagItem(slot).count > stars) {
                stars = 0;
            } else {
                stars -= m_inventory.bagItem(slot).count;
            }
            if (stars > 0) {
                slot = m_inventory.nextSlotWith(kStarItem, slot);
            }
        } while (tries < kMaxStarSlots && stars > 0 && slot != 0);
        if (stars != 0) {
            say("Not enough stars");
            m_wishIndex = 0;
            close();
            return;
        }
        m_services.connection.send(protocol::rebornWishRequest(m_wishes[static_cast<std::size_t>(m_wishIndex)]));
        close();
    }

    void RebornWish::close() {
        m_open = false;
        m_hooks.closed();
    }

}
