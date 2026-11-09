#include "client/scenes/world/WorldScene.h"
#include "client/rules/FurnitureBonus.h"
#include "client/rules/FurnitureKinds.h"
#include "client/rules/HouseMovePath.h"
#include "client/rules/CharacterActions.h"
#include "client/rules/Walking.h"
#include "core/ByteReader.h"
#include "protocol/HouseRequests.h"
#include "protocol/ItemRequests.h"
#include "protocol/PlayerId.h"
#include "protocol/StoreroomLists.h"

#include <algorithm>
#include <cstdlib>

namespace client {

    namespace {

        constexpr int kTreadmillItem = 39105; // item 39105: Treadmill

        constexpr int kTitleMs = 3000; // house title display time, ms
        // background map file per floor room size 1..3 (index 0 unused): 10005 (Test), 10004 (Prison), 10000 (Residence)
        constexpr int kBackgroundOfSize[] = {0, 10005, 10004, 10000};
        constexpr int kFloorRoomMap = 10000; // map 10000: Residence (the floor rooms)
        constexpr int kMailboxItem = 39084; // item 39084: Mailbox
        constexpr int kHandcartItem = 39065; // item 39065: Handcart
        // sub ids of the furniture message 62 (62/sub)
        constexpr int kPortraitSub = 62; // 62/62: portrait frame data
        constexpr int kStoreroomItem = 39030; // item 39030: Storeroom
        constexpr int kBoxUpdateSub = 57; // 62/57: one place of a storage furniture box (Fridge, cabinets, Locker) changes
        constexpr int kMusicSub = 59; // 62/59: music center data
        constexpr int kSandglassSub = 53; // 62/53: sandglass data

    }

    void WorldScene::onHouseRoom(std::int32_t playerId, int room, bool sizesGiven, const std::array<int, 5>& sizes) {
        if (static_cast<std::uint32_t>(protocol::stripServerOffset(playerId)) != m_services.session.loginId) {
            if (const auto other = m_others.find(playerId); other != m_others.end()) {
                other->second.houseRoom = room;
            }
            return;
        }
        m_houseRoom = room;
        if (sizesGiven) {
            m_house.setFloorRoomSizes(sizes);
        }
    }

    int WorldScene::houseBackground(int mapId) const {
        if (!isHouseMap(mapId) || m_houseRoom < 0 || m_houseRoom > 1) {
            return 0;
        }
        const int size = m_house.floorRoomSize(m_houseRoom);
        return size >= 1 && size <= 3 ? kBackgroundOfSize[size] : 0; // floor room sizes 1 to 3 have a picture
    }

    int WorldScene::houseMapFile(int room) {
        switch (room) {
            case 2: return 10081; // room 2 (Cherry Grassplot): map file 10081
            case 3: return 10083; // room 3 (Summer Beach): map file 10083
            case 4: return 10084; // room 4 (Love cabin): map file 10084
            default: return kFloorRoomMap; // the floor rooms 0 and 1 share map 10000
        }
    }

    void WorldScene::showHouseTitle() {
        m_screenLight.stop();
        const auto scene = m_map && m_scenes ? m_scenes->find(m_map->id()) : std::nullopt;
        if (scene && scene->titleLight != 0) {
            m_houseTitle.clear();
            const auto previous = m_scenes->find(m_prevMapId);
            constexpr int kTown = 2; // scene type 2: town
            constexpr int kHouse = 3; // scene type 3: house
            if ((previous ? previous->titleLight : 0) == scene->titleLight || (scene->type == kTown && previous && previous->type == kHouse)) {
                return;
            }
            const auto picture = m_services.pictures.find("T" + std::to_string(scene->titleLight));
            if (picture) {
                constexpr int kFrames = 7; // frames in the title light picture
                LightEffect::Setup setup;
                setup.picture = *picture;
                setup.frameMs = 250; // frame time in ms
                setup.firstFrame = 1;
                setup.lastFrame = kFrames;
                setup.frameCount = kFrames;
                setup.motion = LightEffect::Motion::Once;
                setup.alpha = 50; // light alpha (the 50 of PlayScreenLight)
                m_screenLight
                    .start(setup, {Camera::kScreenWidth / 2, (m_services.pictures.picture(*picture).height() / kFrames + Camera::kScreenHeight) / 2}, m_nowMs);
            }
            return;
        }
        if (!m_map || !isHouseMap(m_map->id())) {
            m_houseTitle.clear();
            return;
        }
        const std::int32_t ownerId = m_house.furniture().ownerId();
        std::string owner = isOwn(ownerId) ? m_player.name : playerName(ownerId);
        if (owner.empty()) {
            owner = "Someone";
        }
        m_houseTitle = owner + "'s home ";
        switch (m_houseRoom) {
            case 0: m_houseTitle += "First Floor"; break; // room 0: first floor
            case 1: m_houseTitle += "Second Floor"; break; // room 1: second floor
            case 2: m_houseTitle = owner + "'s Cherry Grassplot"; break; // room 2: Cherry Grassplot
            case 3: m_houseTitle = owner + "'s Summer Beach"; break; // room 3: Summer Beach
            case 4: m_houseTitle = owner + "'sLove cabin"; break; // room 4: Love cabin
            default: break; // other rooms keep the first title
        }
        m_messages.show(m_houseTitle, kTitleMs);
    }

    void WorldScene::clickCrackerOrClock(int piece) {
        constexpr int kReach = 170; // max distance to the furniture in x and y, px
        constexpr int kMessageMs = 2000; // message display time, ms
        const int slot = m_house.utensils().slotOfFurniture(piece);
        const HouseUtensil& utensil = m_house.utensils().utensil(slot);
        const bool cracker = FurnitureKinds::isNewYearCracker(utensil.itemId);
        if (slot == 0 || (!cracker && !FurnitureKinds::isGuguClock(utensil.itemId))) {
            return;
        }
        if (std::abs(utensil.world.x - m_player.position.x) > kReach || std::abs(utensil.world.y - m_player.position.y) > kReach) {
            return;
        }
        if (utensil.animState != (cracker ? 3 : 0) || m_player.bathing) { // animation state 3: cracker waiting to go off; 0: clock at rest
            return;
        }
        if (m_teams.isMember(m_player.id)) {
            m_messages.show("Cannot use", kMessageMs);
            return;
        }
        if (m_player.usedUtensil != 0 || m_makeJobs.playerSlot() != 0) {
            m_messages.show("Busy, cannot use", kMessageMs);
            return;
        }
        m_services.playSound(cracker ? "sound\\SEB0017.wav" : "sound\\wav1510.wav");
        m_house.utensils().setAnimState(slot, cracker ? 4 : 1); // next animation state: 4 for the cracker, 1 for the clock
        m_services.connection.send(cracker ? protocol::newYearCrackerRequest(piece) : protocol::guguClockRequest(piece));
    }

    void WorldScene::clickStairs(int piece) {
        const int slot = m_house.utensils().slotOfFurniture(piece);
        if (slot == 0 || m_house.room() != 0 || !FurnitureKinds::isStairs(m_services.items, m_house.utensils().utensil(slot).itemId)) {
            return;
        }
        if (FurnitureKinds::isElevator(m_house.utensils().utensil(slot).itemId)) {
            m_services.playSound("sound\\wav1610.wav");
            m_house.utensils().setAnimState(slot, 3); // the Elevator's animation state 3
        }
        m_services.connection.send(protocol::stairsRequest(piece));
    }

    bool WorldScene::receiveStoreroomMessage(const core::Bytes& payload) {
        switch (protocol::MessageId::of(payload).sub) {
            case 7: // 62/7: the storeroom list
                if (const auto list = protocol::decodeStoreroomList(payload)) {
                    m_storeroom.setList(*list);
                }
                return true;
            case 10: // 62/10: an item comes into the storeroom
            case 11: // 62/11: an item comes into the storeroom (the flagged form)
                if (const auto add = protocol::decodeStoreroomAdd(payload, protocol::MessageId::of(payload).sub == 11)) {
                    m_storeroom.add(*add);
                }
                return true;
            case 12: // 62/12: an item goes out of the storeroom (by item)
                if (const auto remove = protocol::decodeStoreroomRemoveItem(payload)) {
                    m_storeroom.remove(*remove);
                }
                return true;
            case 13: // 62/13: an item goes out of the storeroom (by cell)
                if (const auto remove = protocol::decodeStoreroomRemoveCell(payload)) {
                    m_storeroom.remove(*remove);
                }
                return true;
            case 50: // 62/50: the storeroom form opens
                m_storeroom.open();
                return true;
            default: // not a storeroom message
                return false;
        }
    }

    StoreroomWindow::Hooks WorldScene::storeroomHooks() {
        StoreroomWindow::Hooks hooks;
        hooks.pieceElsewhere = [this](int piece) { return m_house.furniture().piece(piece).room != m_house.room(); };
        hooks.dropOnWorld = [this](int cell, int itemId, int count, int piece) {
            return m_house.inFloorRoom() && m_fightState == 0 && m_houseEditor.dropStoreroomItem(cell, itemId, count, piece, m_housePainter.hoveredSlot());
        };
        return hooks;
    }

    RetainFurWindow::Hooks WorldScene::retainFurHooks() {
        RetainFurWindow::Hooks hooks;
        hooks.ownHouse = [this] { return m_map && isHouseMap(m_map->id()) && isOwn(m_house.furniture().ownerId()); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.inOtherRoom = [this] { return m_map && isHouseMap(m_map->id()) && !m_house.inFloorRoom(); };
        hooks.notice = [this](const std::string& text, int ms) { m_messages.show(text, ms); };
        hooks.take = [this](int piece) { m_houseEditor.pressRetained(piece, worldMouse(), m_camera); };
        hooks.carry = [this] { m_houseEditor.dragTo(worldMouse(), m_camera); };
        hooks.drop = [this] { m_houseEditor.release(worldMouse(), m_housePainter.hoveredSlot(), m_camera); };
        return hooks;
    }

    HouseEditor::Hooks WorldScene::houseEditorHooks() {
        HouseEditor::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.notice = [this](const std::string& text, int ms) { m_messages.show(text, ms); };
        hooks.confirm = [this](const std::string& text, std::function<void()> onOk) { m_talk.ask(text, std::nullopt, std::move(onOk), [] {}); };
        hooks.choose = [this](const std::string& text, bool canRotate, std::function<void()> onRotate, std::function<void()> onGather) {
            TalkDialog::Timing timing;
            if (canRotate) {
                timing.okPicture = "btn_Rotate_1";
                timing.closePicture = "btn_Gather_1";
                m_talk.askOrClose(text, std::nullopt, std::move(onRotate), [] {}, std::move(onGather), std::move(timing));
            } else {
                timing.okPicture = "btn_Gather_1";
                m_talk.ask(text, std::nullopt, std::move(onGather), [] {}, std::move(timing));
            }
        };
        hooks.ownId = [this] { return m_player.id; };
        hooks.playerFeet = [this] { return m_player.position; };
        hooks.locked = [this] { return m_moveLocked; };
        hooks.queueBlocks = [this](int itemId) {
            return m_computer.state() == ComputerWindow::State::Running && itemId != ComputerWindow::kComputerItem && m_makeJobs.playerSlot() != 0 &&
                   m_makeJobs.petSlot() == 0;
        };
        hooks.roomInBag = [this](const std::vector<int>& itemIds) { return roomInBagFor(itemIds); };
        hooks.used = [this](int piece) {
            const int itemId = m_house.furniture().piece(piece).itemId;
            if (IceBoxWindow::boxOf(itemId) != 0) {
                m_iceBox.open(piece);
                return;
            }
            if (itemId == PledgeBoxWindow::kLockerItem) {
                m_pledgeBox.open(piece);
                return;
            }
            if (itemId == MusicCenterWindow::kMusicCenterItem && m_player.id == m_house.furniture().ownerId()) {
                m_musicCenter.open();
            }
            if (m_player.id == m_house.furniture().ownerId()) {
                if (itemId == GasStorageWindow::kBigTankItem) {
                    m_gasCan.open();
                } else if (itemId == GasStorageWindow::kSmallTankItem) {
                    m_smallGas.open();
                }
            }
            if (itemId == GlobeWindow::kGlobeItem) {
                m_globe.open();
            }
            if (FurnitureKinds::isVendingMachine(m_services.items, itemId)) {
                m_vending.open(piece);
            }
            if (itemId == kMailboxItem) {
                m_furMail.show();
            }
            if (FurnitureKinds::isFortuneEgg(itemId) && m_player.id == m_house.furniture().ownerId()) {
                m_eggSell.open(piece);
            }
            if (FurnitureKinds::isTvGameConsole(itemId)) {
                openTvConsole(piece);
            }
            if (FurnitureKinds::cupboardKind(m_services.items, itemId) != 0) {
                m_cupboard.open(piece);
            }
            if (FurnitureKinds::isBookcase(m_services.items, itemId)) {
                m_bookShelf.open(false);
            }
            if (FurnitureKinds::isBookDesk(m_services.items, itemId)) {
                m_bookShelf.open(true);
            }
            if (itemId == kHandcartItem) {
                m_homeAdorn.toggle(piece);
            }
            if (itemId == ComputerWindow::kComputerItem && m_services.items.find(itemId)) {
                m_computer.open();
            }
            if (FurnitureKinds::isPictureFrame(itemId)) {
                m_portrait.clicked(piece, itemId);
            }
            if (FurnitureKinds::isDoorplate(m_services.items, itemId)) {
                m_doorplate.open();
            }
            if (FurnitureKinds::isPiano(m_services.items, itemId)) {
                usePiano(piece, true);
            }
            clickSeat(piece);
            clickTraining(piece);
            clickCrackerOrClock(piece);
            clickStairs(piece);
            if (isBathtub(itemId)) {
                useBathtub(piece);
            }
            if (FurnitureKinds::isBlackboard(m_services.items, itemId)) {
                m_blackBoard.open();
            }
            if (FurnitureKinds::maidKind(m_services.items, itemId) > 1 && !m_teams.isMember(m_player.id)) {
                m_maid.open(piece);
            }
            if (FurnitureKinds::isFishBox(m_services.items, itemId)) {
                m_services.connection.send(protocol::openFishTankRequest());
            }
            if (itemId == kStoreroomItem) {
                m_storeroom.open(piece);
            }
            if (itemId == FurnitureCarCases::kGarageItem) {
                m_carCase.request(piece);
            }
            if (itemId == FurnitureCarCases::kShowPlatformItem) {
                m_showPlatform.request(piece);
            }
            if (FurnitureKinds::bedKind(m_services.items, itemId) != 0) {
                m_bed.open(piece);
            }
            if (Sandglass::isSandglassItem(m_services.items, itemId)) {
                askSandglass(piece);
                return;
            }
            openMakeTree(piece);
        };
        hooks.shelfDrop = [this](int slot, int piece) { return dropOnShelf(slot, piece); };
        return hooks;
    }

    void WorldScene::changeMaidPiece(int furnIndex, int itemId) {
        const int slot = m_house.utensils().slotOfPiece(furnIndex);
        m_house.furniture().setItemId(furnIndex, itemId);
        if (slot != 0) {
            HouseUtensil& utensil = m_house.utensils().utensil(slot);
            utensil.itemId = itemId;
            utensil.picture = m_house.utensils().facingPicture(itemId, m_house.furniture().piece(furnIndex).rotation);
        }
    }

    void WorldScene::onBookShelfSlot(bool desk, int place, int itemId) {
        constexpr int kBlueprintSubType = 103; // item sub type 103: furniture blueprints
        const auto record = itemId != 0 ? m_services.items.find(itemId) : std::nullopt;
        if (itemId == 0 || (record && record->subType == kBlueprintSubType)) {
            m_bookShelves.set(desk, place, itemId);
        }
    }

    BookShelfWindow::Hooks WorldScene::bookShelfHooks() {
        BookShelfWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        return hooks;
    }

    ComputerWindow::Hooks WorldScene::computerHooks() {
        ComputerWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.inOwnHouse = [this] { return m_map && isHouseMap(m_map->id()) && isOwn(m_house.furniture().ownerId()); };
        hooks.bathing = [this] { return m_player.bathing; };
        hooks.furnitureFlags = [this] { return FurnitureBonus::flags(m_house.furniture()); };
        hooks.nowMs = [this] { return m_nowMs; };
        hooks.hasMaid = [this] { return hasMaidFurniture(); };
        hooks.choosePartner = [this](std::function<bool(int, int)> picked) { m_npcMake.open(std::move(picked)); };
        hooks.partnerLeft = [this] { makerLeaves(); };
        return hooks;
    }

    PortraitWindow::Hooks WorldScene::portraitHooks() {
        PortraitWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.houseOwnerId = [this] { return m_house.furniture().ownerId(); };
        hooks.ownId = [this] { return m_player.id; };
        hooks.ownLook = [this] { return m_player.look; };
        hooks.worldLook = [this](std::int32_t id) -> std::optional<CharacterLook> {
            const auto found = m_otherRecords.find(id);
            if (found == m_otherRecords.end()) {
                return std::nullopt;
            }
            return CharacterLook::fromAppearance(found->second.appearance, m_services.items);
        };
        hooks.pieceItem = [this](int piece) { return piece >= 1 && piece <= HomeFurniture::kMost ? m_house.furniture().piece(piece).itemId : 0; };
        return hooks;
    }

    HomeAdornWindow::Hooks WorldScene::homeAdornHooks() {
        HomeAdornWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.floor = [this] { return m_houseRoom; };
        hooks.ask = [this](const std::string& text, std::function<void()> onOk, std::function<void()> onCancel) {
            m_talk.ask(
                text,
                std::nullopt,
                std::move(onOk),
                std::move(onCancel),
                {32, 0, {}, false}
            ); // 32 characters per line, no timeout, windows stay (not exclusive)
        };
        hooks.locked = [this] { return m_moveLocked; };
        return hooks;
    }

    DoorplateWindow::Hooks WorldScene::doorplateHooks() {
        DoorplateWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.ownId = [this] { return m_player.id; };
        return hooks;
    }

    bool WorldScene::roomInBagFor(const std::vector<int>& itemIds) const {
        Inventory copy = m_inventory;
        for (const int itemId : itemIds) {
            const int slot = copy.freeSlotFor(itemId);
            protocol::ItemInstance item;
            item.itemId = itemId;
            item.count = 1;
            if (slot == 0 || copy.addToBag(slot, item) == 0) {
                return false;
            }
        }
        return true;
    }

    std::vector<core::Point> WorldScene::houseWalkPath(core::Point from, core::Point to) const {
        const core::Point origin = m_camera.origin();
        HouseMovePath path(m_house.furniture(), m_house.room(), {origin.x, origin.y, Camera::kScreenWidth, Camera::kScreenHeight});
        return path.find(from, to);
    }

    void WorldScene::jumpToWaymark(core::Point door) {
        m_player.position = door;
        stopAction(m_player);
        startWalking(m_player, {}, m_nowMs);
        checkDoors();
    }

    void WorldScene::onFurnitureMessage(const core::Bytes& payload) {
        if (protocol::MessageId::of(payload).sub == kPortraitSub) {
            m_portrait.receive(payload);
            return;
        }
        if (protocol::MessageId::of(payload).sub == kBoxUpdateSub) {
            m_houseBoxes.receive(payload);
            return;
        }
        if (protocol::MessageId::of(payload).sub == kMusicSub) {
            m_musicCenter.receive(payload, m_nowMs);
            return;
        }
        if (receiveStoreroomMessage(payload)) {
            return;
        }
        if (protocol::MessageId::of(payload).sub == kSandglassSub) {
            receiveSandglass(payload);
            return;
        }
        m_house.receiveFurniture(payload, m_nowMs);
    }

    void WorldScene::receiveHouseRoomMessage(const core::Bytes& payload) {
        switch (protocol::MessageId::of(payload).sub) {
            case 11: m_house.receivePillarList(payload); break; // 65/11: the pillar list
            case 12: // 65/12: the size of a floor room
                if (m_house.receiveRoomSize(payload) && m_map) {
                    const int size = m_house.floorRoomSize(m_houseRoom);
                    if (size >= 1 && size <= 3) { // floor room sizes 1 to 3 have a picture
                        m_map->changeBackground(kBackgroundOfSize[size], m_services.pictures);
                    }
                }
                break;
            case 13: m_house.receivePillarDiff(payload); break; // 65/13: a pillar change
            default: break; // other sub ids are ignored
        }
    }

    void WorldScene::setHouseDisplay(WorldPainter::Display& display, std::uint64_t nowMs) {
        if (!m_map || !isHouseMap(m_map->id()) || !m_house.inFloorRoom()) {
            return;
        }
        display.houseRoom = m_houseRoom;
        display.houseFloor = [this](gfx::Canvas& canvas) { m_housePainter.paintFloorAndWalls(canvas, m_camera); };
        display.bathCell = [this](const MapCharacter& player) { return bathSortCell(player); };
        display.petsHidden = [this](const MapCharacter& player) {
            const int slot = player.usedUtensil != 0 ? m_house.utensils().slotOfPiece(player.usedUtensil) : 0;
            return slot != 0 && m_house.utensils().utensil(slot).itemId == kTreadmillItem;
        };
        display.bathExtras = [this](gfx::Canvas& canvas, const MapCharacter& player, bool afterFigure) { drawSprayExtras(canvas, player, afterFigure); };
        m_housePainter.setSprayBath(sprayBathPiece());
        updateTrainingDisplay(nowMs);
        m_housePainter.setTrainingPetPainter([this](gfx::Canvas& canvas, const Camera& camera, int furnIndex) { paintTrainingPet(canvas, camera, furnIndex); });
        m_houseEditor.update(m_itemWindow.draggedBagItem(), worldMouse(), m_camera);
        m_housePainter.setDrag(m_houseEditor.drag());
        m_housePainter.setWallLight(m_houseEditor.wallLight());
        display.houseCells = [this, nowMs](gfx::Canvas& canvas, const std::vector<HouseEntry>& entries) {
            const std::optional<core::Point> mouse = m_inWorld && m_screen.hoveredId() == m_view.id() ? std::optional<core::Point>(worldMouse()) : std::nullopt;
            m_housePainter.updateHover(canvas, m_camera, mouse);
            m_housePainter.updatePillarEdit(canvas, m_camera, mouse);
            m_housePainter.paintCells(canvas, m_camera, entries, nowMs);
            m_housePainter.paintLabels(canvas, m_camera);
            showShelfInfo();
        };
    }

    bool WorldScene::ownHouse() const { return m_map && isHouseMap(m_map->id()) && isOwn(m_house.furniture().ownerId()); }

    bool WorldScene::specFloorShown() const {
        int roomMap = 0;
        switch (m_houseRoom) {
            case 2: roomMap = 10081; break; // room 2: map file 10081
            case 3: roomMap = 10083; break; // room 3: map file 10083
            case 4: roomMap = 10084; break; // room 4: map file 10084
            default: break; // the floor rooms have no special-floor map
        }
        return m_map && isHouseMap(m_map->id()) && roomMap != 0 && m_specFloor.isShown(ownHouse(), roomMap);
    }

    bool WorldScene::specFloorHovered() const {
        return m_inWorld && specFloorShown() && m_screen.hoveredId() == m_view.id() && m_specFloor.contains(ownHouse(), worldMouse(), m_camera);
    }

    void WorldScene::pickSpecFloorThing() {
        constexpr int kNear = 170; // max distance to the thing in x and y, px
        constexpr int kBodyHeight = 90; // character body height, px
        constexpr int kMessageMs = 1500; // message display time, ms
        if (!ownHouse()) {
            m_messages.show("Only host of the house can pick", kMessageMs);
            return;
        }
        const int itemId = m_specFloor.itemId(true);
        if (m_inventory.placeFor(itemId, 1) == 0) {
            m_messages.show("Not enough Item cases", kMessageMs);
            return;
        }
        const core::Point place = m_specFloor.place(true);
        if (std::abs(place.x - m_player.position.x) > kNear || std::abs(place.y - m_player.position.y) > kNear) {
            m_messages.show("too far away", kMessageMs);
            return;
        }
        const core::Point feet = m_camera.toScreen(m_player.position);
        m_flights.pickUp(
            m_specFloor.flightPicture(itemId),
            m_camera.toScreen(place),
            {feet.x, feet.y - kBodyHeight / 5},
            m_nowMs
        ); // the icon flies to a fifth of the body height above the feet
        m_services.connection.send(protocol::pickSpecFloorThingRequest());
    }

    void WorldScene::onSpecFloorMessage(const core::Bytes& payload) {
        core::ByteReader reader(payload, 1);
        const int sub = reader.u8();
        if (sub == 1) { // 81/1: a special-floor thing appears
            const int kind = reader.u8();
            const int itemId = reader.u16();
            const int place = reader.u8();
            const std::int32_t ownerId = reader.i32();
            if (!reader.failed()) {
                m_specFloor.receive(kind, itemId, place, ownerId, m_player.id, m_houseRoom);
            }
        } else if (sub == 2) { // 81/2: the thing of a player is gone
            const std::int32_t playerId = reader.i32();
            if (!reader.failed()) {
                if (playerId != m_player.id) {
                    m_specFloor.clearVisitor();
                } else {
                    m_specFloor.clearOwner();
                }
            }
        } else if (sub == 4) { // 81/4: the owner's thing is cleared
            m_specFloor.clearOwner();
        }
    }

}
