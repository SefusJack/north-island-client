#include "client/scenes/world/WorldScene.h"

#include "core/FileSystem.h"
#include "client/rules/SoldierTime.h"
#include "formats/ServerList.h"
#include "protocol/PlayerId.h"

namespace client {

    NoticeBoard::Hooks WorldScene::noticeBoardHooks() {
        return {
            [this](const core::Bytes& bytes) { m_services.connection.send(bytes); },
            [this](const std::string& text, int durationMs) { m_messages.show(text, durationMs); }
        };
    }

    void WorldScene::onNoticeRows(bool guild, const std::vector<protocol::NoticeRow>& rows) { (guild ? m_guildBoard : m_publicBoard).setRows(rows); }

    void WorldScene::onNoticePages(bool guild, const protocol::NoticePageInfo& info, bool restoreAsked) {
        NoticeBoard& board = guild ? m_guildBoard : m_publicBoard;
        if (guild && restoreAsked) {
            board.clearRows();
        }
        board.setPageInfo(info);
        if (restoreAsked) {
            board.restoreAskedPage();
        }
    }

    void WorldScene::onNoticeContent(bool guild, const protocol::NoticeContent& content) { m_noticeRead.show(guild, content); }

    void WorldScene::onNoticeWriteForm(bool guild) { m_noticeSend.open(guild); }

    void WorldScene::onNoticeBoardOpened() { m_noticeBoardWindow.open(); }

    void WorldScene::onNoticeMessage(const std::string& text, int durationMs) { m_messages.show(text, durationMs); }

    namespace {

        /// @brief Looks up an item's name in the item table (GetItemTableName).
        /// @param services The game services holding the item table.
        /// @param itemId The item id.
        /// @return The item name, or an empty string for an item the table does not hold.
        std::string itemTableName(GameServices& services, int itemId) {
            const auto record = services.items.find(itemId);
            return record ? record->name : std::string();
        }

    }

    void WorldScene::onItemDecreased(int itemId, int kind) {
        if (kind == 1) { // notice kind 1: the item count dropped (other kinds show nothing)
            m_messages.show(itemTableName(m_services, itemId) + " Decrease ", 1200); // message display time in ms
        }
    }

    void WorldScene::onItemSound() { m_services.playSound("sound/wav0152.wav"); }

    void WorldScene::onBagItemsGained(const std::vector<protocol::BagItemRecord>& records) {
        std::string list;
        for (const protocol::BagItemRecord& record : records) {
            if (record.slot >= 1 && record.slot <= Inventory::kBagSlots) {
                m_inventory.addToBag(record.slot, record.item);
            }
            list += itemTableName(m_services, record.item.itemId) + std::to_string(record.item.count) + "Quantity" + " ";
        }
        inventoryChanged();
        m_messages.show("Obtain " + list, 1200); // message display time in ms
        m_services.playSound("sound/wav0152.wav");
    }

    void WorldScene::onBagFullGifts(const std::vector<protocol::ItemInstance>& items) {
        std::string list;
        for (const protocol::ItemInstance& item : items) {
            list += itemTableName(m_services, item.itemId) + std::to_string(item.count) + "Quantity" + " ";
        }
        m_messages.show("Send presents sometimes because your inventory is full." + list, 2000); // message display time in ms
    }

    void WorldScene::onExpIncreased(std::int32_t playerId, int petPlace, std::int32_t exp) {
        if (!isOwn(playerId)) {
            return;
        }
        std::string name = m_player.name;
        if (petPlace != 0) {
            if (petPlace < 1 || petPlace > PetTeam::kPlaces || !m_pets.at(petPlace)) { // pet places start at 1
                return;
            }
            name = m_pets.at(petPlace)->name;
        }
        m_messages.show(name + " Increases EXP " + std::to_string(exp), 3000); // message display time in ms
    }

    void WorldScene::onRecommendResult(int result) {
        // chat channel 10; result 1 = recommendation succeeded
        m_chat.add(
            10,
            result == 1 ? "Succeed to recommend!" : "Fail to recommend! Please ensure the recommender and the recommended player are qualified.",
            m_player.id
        );
    }

    void WorldScene::onRecommendPoints(std::int32_t points) {
        m_messages.show("obtain points because of being recommended." + std::to_string(points), 10000); // message display time in ms
    }

    void WorldScene::onCarnieTeamRefused(std::int32_t playerId) {
        if (isOwn(playerId)) {
            m_messages.show("Players who are telepoted to carnie can't team up", 4000); // message display time in ms
        }
    }

    void WorldScene::onAutoHealPotion(int times) {
        m_messages.show("Use auto-healing potion" + std::to_string(times) + "times", 2000);
    } // message display time in ms

    void WorldScene::onSoldierTime(int minutes) {
        const std::string text = soldierTimeText(minutes);
        if (!text.empty()) {
            m_messages.show("Available time for soldiers " + text, 2000); // message display time in ms
        }
    }

    void WorldScene::onScenePetChanged(std::int32_t playerId, int petKind, int newKind) {
        if (const auto npc = m_services.npcs.find(newKind)) {
            m_mapPets.changeKind(playerId, petKind, *npc);
        }
    }

    void WorldScene::onAutoPillUsed(int kind, int count) {
        constexpr int kMostPills = 255; // the pill counter is capped at 255 (one byte)
        m_autoPillCount = std::min(m_autoPillCount + count, kMostPills);
        if (kind == 1) { // pill kind 1: auto-attack
            m_messages.show("use auto-attack pill" + std::to_string(m_autoPillCount) + "times", 1500); // message display time in ms
        } else if (kind == 2) { // pill kind 2: auto F1 potion
            m_messages.show("Use auto F1 potion" + std::to_string(m_autoPillCount) + "times", 1500); // message display time in ms
        }
    }

    void WorldScene::onFighterAbstains(std::int32_t playerId) {
        if (playerId == m_player.id || static_cast<std::uint32_t>(protocol::stripServerOffset(playerId)) == m_services.session.loginId) {
            m_messages.show(m_player.name + "Abstain", 2000); // message display time in ms
        } else if (const auto other = m_others.find(playerId); other != m_others.end()) {
            m_messages.show(other->second.name + "Abstain", 2000); // message display time in ms
        }
    }

    void WorldScene::onPetGearBackHome(int npcId) {
        const int place = m_pets.placeOf(npcId);
        if (place != 0 && m_pets.at(place)) {
            m_chat.add(0, "(System Prompt):" + m_pets.at(place)->name + " The gear has been taken back into the warehouse at home");
        }
    }

    void WorldScene::onFightHelp(int mode, std::int32_t helperId, int kind) {
        constexpr int kPlayerHelper = 2; // helper kind 2: the helper is a player
        if (helperId <= 0) {
            m_messages.show("It is not easy to win", 2000); // message display time in ms
            return;
        }
        std::string name;
        if (mode == 1 && kind != kPlayerHelper) { // mode 1: the helper is an NPC unless the kind says player
            const auto npc = m_services.npcs.find(helperId);
            name = npc ? npc->name : std::string();
        } else {
            name = playerName(helperId);
            if (name.empty()) {
                name = m_player.name;
            }
        }
        m_messages.show(name + ",Let me help you", 2000); // message display time in ms
    }

    void WorldScene::onFighterMoved(int fromRow, int fromCol, int toRow, int toCol) { m_battle.moveFighter(fromRow, fromCol, toRow, toCol); }

    void WorldScene::onItemGlows(int itemId) {
        m_messages.show(itemTableName(m_services, itemId) + "Give off a mysterious light", 2000);
    } // message display time in ms

    Top100::Hooks WorldScene::top100Hooks() {
        return {
            [this](const core::Bytes& bytes) { m_services.connection.send(bytes); },
            [this](int code) {
                if (const auto lines = core::readLines(m_services.game.data("SERVER.INI"))) {
                    const formats::ServerList servers = formats::ServerList::parse(*lines);
                    for (const formats::ServerGroup& group : servers.groups()) {
                        if (group.code == code) {
                            return group.name;
                        }
                    }
                }
                return std::string();
            }
        };
    }

    void WorldScene::onTop100Opened() { m_top100Window.open(); }

    void WorldScene::onTop100List(const core::Bytes& payload) { m_top100.receive(payload); }

    void WorldScene::closeNoticeForms() {
        m_noticeSend.close();
        m_noticeRead.close();
        if (m_sendMail.isOpen()) {
            m_sendMail.hide();
        }
    }

    void WorldScene::mailNoticeAuthor(bool guild, std::int32_t authorId, const std::string& name) {
        if (guild) {
            m_sendMail.openToGuildMember(authorId);
        } else {
            m_sendMail.openToNoticeAuthor(authorId, name);
        }
    }

}
