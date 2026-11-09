#include "client/rules/FurnitureBonus.h"
#include "client/rules/FurnitureKinds.h"
#include "client/scenes/world/WorldScene.h"
#include "client/rules/CharacterActions.h"
#include "client/rules/Walking.h"
#include "core/ByteReader.h"
#include "protocol/MessageId.h"

#include <string>

namespace client {

    namespace {

        constexpr int kLinkedTools = 38049; // item 38049: Work Platform (links other workbenches)
        constexpr int kFortuneEgg = 0x9891; // item 39057: Fortune Egg
        constexpr int kBathtubSubType = 112; // item sub type 112: bathtub furniture
        constexpr int kWorkingAction = 46; // character action group 46: sitting/working pose
        constexpr int kNoticeMs = 1500; // message display time, ms
        constexpr int kNoWorkRoom = 250; // value 250: no room left to work in (sentinel)
        constexpr int kFinishedStoreMs = 2000; // message display time, ms
        // sub ids of the manufacture message 64 (64/sub)
        constexpr int kGasStateSub = 12; // 64/12: big gas storage fuel state
        constexpr int kGasRefusedSub = 13; // 64/13: big gas storage refused
        constexpr int kSmallGasStateSub = 14; // 64/14: small gas storage fuel state
        constexpr int kSmallGasRefusedSub = 15; // 64/15: small gas storage refused
        constexpr int kLongNoticeMs = 2500; // message display time, ms
        constexpr core::Point kBarPlace{-25, 20}; // progress bar offset from the character's feet, px
        constexpr core::Point kBarFill{1, 1}; // progress bar fill offset inside the frame, px
        constexpr int kBarHeight = 3; // progress bar fill height, px
        constexpr int kMinuteMs = 60000; // milliseconds per minute

    }

    bool WorldScene::inHouseFloorRoom() const { return m_house.inFloorRoom(); }

    std::string WorldScene::makeRefusal() const {
        const auto leader = m_teams.leaderOf(m_player.id);
        return leader && *leader != m_player.id ? "Cannot manufacture" : "";
    }

    MakeQuestions::Hooks WorldScene::makeQuestionHooks() {
        MakeQuestions::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.closeWindows = [this] {
            m_makeThing.close();
            m_makeInfo.close();
        };
        hooks.stopped = [this] { m_computer.stopped(); };
        hooks.refusal = [this] { return makeRefusal(); };
        hooks.hasMaid = [this] { return hasMaidFurniture(); };
        hooks.choosePartner = [this](std::function<bool(int, int)> picked) { m_npcMake.open(std::move(picked)); };
        return hooks;
    }

    MakeThingInfoWindow::Hooks WorldScene::makeInfoHooks() {
        MakeThingInfoWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.refusal = [this] { return makeRefusal(); };
        hooks.bathing = [this] { return m_player.bathing; };
        hooks.hasMaid = [this] { return hasMaidFurniture(); };
        hooks.choosePartner = [this](std::function<bool(int, int)> picked) { m_npcMake.open(std::move(picked)); };
        hooks.partnerOrdered = [this] { m_npcMaking = true; };
        return hooks;
    }

    MakeThingWindow::Hooks WorldScene::makeThingHooks() {
        MakeThingWindow::Hooks hooks;
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.askContinue = [this](int job, const std::string& product, int piece, int source, int slot) {
            m_makeQuestions.askContinue(job, product, piece, source, slot);
        };
        return hooks;
    }

    void WorldScene::openMakeTree(int piece) {
        const FurnitureRecord& record = m_house.furniture().piece(piece);
        const auto item = m_services.items.find(record.itemId);
        if (!item || record.itemId == kFortuneEgg || (item->subType == kBathtubSubType && item->rank != 0)) {
            return;
        }
        std::vector<int> facilities{record.itemId};
        if (record.itemId == kLinkedTools) {
            for (std::size_t place = 1; place < record.links.size(); ++place) {
                if (record.links[place] > 0) {
                    facilities.push_back(m_house.furniture().piece(record.links[place]).itemId);
                }
            }
        }
        m_makePiece = piece;
        m_makeThing.show(piece, facilities);
    }

    void WorldScene::workAtPiece() {
        const int slot = m_house.utensils().slotOfPiece(m_makePiece);
        if (slot != 0) {
            m_player.direction = directionTowards(m_player.position, m_house.utensils().utensil(slot).world, m_player.direction);
        }
        startAction(m_player, kWorkingAction);
    }

    void WorldScene::onManufactureMessage(const core::Bytes& payload) {
        const int sub = protocol::MessageId::of(payload).sub;
        if (sub == 1) { // 64/1: the list of the make jobs
            m_makeJobs.applyList(payload);
            return;
        }
        if (sub >= kGasStateSub && sub <= kSmallGasRefusedSub) {
            core::ByteReader reader(payload, 2); // the body starts after the 2 id bytes
            const int fuel = reader.u16();
            const int finished = reader.u8();
            const int tank = reader.u8();
            if (sub == kGasStateSub && !reader.failed()) {
                (tank == 2 ? m_smallGas : m_gasCan).setFuel(fuel); // tank 2 is the small gas storage
            } else if (sub == kSmallGasStateSub && !reader.failed()) {
                m_smallGas.setFuel(fuel);
            } else if (sub == kGasRefusedSub) {
                m_gasCan.unmarkAll();
            } else if (sub == kSmallGasRefusedSub) {
                m_smallGas.unmarkAll();
            }
            if ((sub == kGasStateSub || sub == kSmallGasStateSub) && finished == 1) { // finished flag set: the store is full
                m_messages.show("Finished store", kFinishedStoreMs);
            }
            return;
        }
        const int slot = payload.size() > 2 ? payload[2] : 0; // job slot byte at offset 2
        const MakeSlot job = m_makeJobs.slot(slot);
        const auto nameOf = [this](int itemId) {
            const auto item = m_services.items.find(itemId);
            return item ? item->name : std::string();
        };
        const auto standUp = [this] {
            if (m_player.action == kWorkingAction) {
                stopAction(m_player);
            }
        };
        switch (sub) {
            case 2: { // 64/2: a job starts (u8 slot)
                m_makeJobs.start(slot, m_nowMs);
                const int minutes = m_services.compounds.at(m_services.compounds.indexOfProduct(job.productId)).makeMinutes;
                if (m_makeJobs.petSlot() == 0) {
                    workAtPiece();
                } else if (slot == 1) { // job slot 1 is the one the partner works on
                    const int utensil = m_house.utensils().slotOfPiece(m_makePiece);
                    m_makeJobs.setWork(minutes > 0 ? m_houseRoom : kNoWorkRoom, utensil != 0 ? m_house.utensils().utensil(utensil).world : core::Point{});
                }
                m_messages.show("Start Manufacture " + nameOf(job.productId), kNoticeMs);
                if (minutes > 0) {
                    m_makeThing.close();
                    m_makeInfo.close();
                }
                m_npcMaking = false;
                break;
            }
            case 3: // 64/3: a job stops (u8 slot)
                if (m_makeJobs.petSlot() == 0) {
                    standUp();
                }
                m_messages.show("Stop Manufacture " + nameOf(job.productId), kNoticeMs);
                m_makeJobs.stop(slot);
                m_computer.stopped();
                makerLeaves();
                break;
            case 4: // 64/4: a job is finished (u8 slot)
                if (m_makeJobs.petSlot() == 0) {
                    standUp();
                }
                if (job.batches == 1) { // a single batch: no quantity in the message
                    m_messages.show(" Manufacture " + nameOf(job.productId) + " Finish ", kNoticeMs);
                } else {
                    const int each = m_services.compounds.at(m_services.compounds.indexOfProduct(job.productId)).productCount +
                                     FurnitureBonus::extraProducts(m_house.furniture(), m_services.compounds, job.productId, 0);
                    m_messages.show(" Manufacture " + nameOf(job.productId) + std::to_string(each * job.batches) + " Quantity " + " Finish ", kNoticeMs);
                }
                m_makeJobs.release(slot);
                queueEnds(slot);
                break;
            case 5: { // 64/5: a job is given up (u8 slot)
                const int working = m_makeJobs.playerSlot();
                if (working == slot) {
                    standUp();
                }
                m_messages.show("Give up Manufacture " + nameOf(job.productId), kNoticeMs);
                m_makeJobs.release(slot);
                if (m_computer.state() != ComputerWindow::State::Running && working == slot) {
                    makerLeaves();
                }
                m_computer.ended(slot);
                break;
            }
            case 6: // 64/6: the job cannot finish, the body space is not enough
            case 7: // 64/7: the job cannot finish, the house space is not enough
            case 8: { // 64/8: the job cannot finish, the instrument reached its upper limit
                stopAction(m_player);
                const char* why = sub == 6   ? "Your body space is not enough"
                                  : sub == 7 ? "The house space is not enough"
                                             : "Reached the upper limit of the instrument";
                m_messages.show(std::string(why) + ", " + " Manufacture " + nameOf(job.productId) + " Unable to finish", kLongNoticeMs);
                m_makeJobs.fail(
                    slot,
                    static_cast<std::uint32_t>(m_services.compounds.at(m_services.compounds.indexOfProduct(job.productId)).makeMinutes * kMinuteMs)
                );
                m_makeThing.close();
                m_makeInfo.close();
                queueEnds(slot);
                break;
            }
            case 9: // 64/9: the partner leaves
                makerLeaves();
                m_computer.endQueue();
                break;
            case 10: { // 64/10: a partner comes out to make it (u8 petSlot, i32 petId)
                core::ByteReader reader(payload, 2); // the body starts after the 2 id bytes
                const int petSlot = reader.u8();
                const int petId = reader.i32();
                m_mapPets.hideMaker(m_player.id);
                m_makeJobs.setPet(petSlot, petId);
                if (!reader.failed() && m_map && isHouseMap(m_map->id()) && isOwn(m_house.furniture().ownerId())) {
                    showMaker(petId);
                }
                break;
            }
            case 11: // 64/11: the partner order is over
                m_npcMaking = false;
                break;
            default: // other sub ids: ignored
                break;
        }
    }

    void WorldScene::queueEnds(int slot) {
        const bool queued = m_computer.state() == ComputerWindow::State::Running;
        m_computer.ended(slot);
        if (!queued) {
            makerLeaves();
        }
    }

    bool WorldScene::hasMaidFurniture() const {
        for (int piece = 1; piece <= HomeFurniture::kMost; ++piece) {
            if (FurnitureKinds::maidKind(m_services.items, m_house.furniture().piece(piece).itemId) != 0) {
                return true;
            }
        }
        return false;
    }

    void WorldScene::showMaker(int petId) {
        const int place = m_pets.placeOf(petId);
        if (m_fightState != 0 || petId == 0 || place == 0) {
            return;
        }
        if (const std::optional<Pet>& pet = m_pets.at(place)) {
            m_mapPets.showMaker(m_player.id, pet->info, pet->name, pet->tradable, m_nowMs);
        }
    }

    void WorldScene::makerLeaves() {
        m_mapPets.hideMaker(m_player.id);
        m_makeJobs.clearPet();
    }

    void WorldScene::driveMaker() { m_mapPets.driveMaker(m_player.id, m_makeJobs.workRoom() == m_houseRoom && inHouseFloorRoom(), m_makeJobs.workAt()); }

    void WorldScene::paintMakeBar(gfx::Canvas& canvas, core::Point feet) {
        const int slot = m_makeJobs.playerSlot();
        if (slot == 0 || m_makeJobs.slot(slot).productId == 0 || !m_makeJobs.slot(slot).active) {
            return;
        }
        const auto frame = m_services.pictures.find("Panel_makeThingTimeForm_1");
        const auto bar = m_services.pictures.find("Panel_makeThingTime_1");
        // constants kBarPlace/kBarFill/kBarHeight position the frame, the fill and its height
        const int minutes = m_services.compounds.at(m_services.compounds.indexOfProduct(m_makeJobs.slot(slot).productId)).makeMinutes;
        const int total = m_makeJobs.totalSeconds(slot, minutes, FurnitureBonus::flags(m_house.furniture()), m_sandglass.slotActive(slot));
        if (frame) {
            canvas.drawPicture(*frame, {feet.x + kBarPlace.x, feet.y + kBarPlace.y});
        }
        if (const int length = m_makeJobs.barLength(total, m_nowMs); bar && length > 0) {
            canvas.drawPicture(*bar, {feet.x + kBarPlace.x + kBarFill.x, feet.y + kBarPlace.y + kBarFill.y}, core::Rect{0, 0, length, kBarHeight});
        }
    }

}
