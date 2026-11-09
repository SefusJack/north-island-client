#include "client/rules/FurnitureKinds.h"
#include "client/rules/UtensilSeat.h"
#include "client/rules/ItemInfo.h"
#include "client/scenes/world/WorldScene.h"
#include "protocol/FurnitureStallRequests.h"

#include <cstdlib>
#include <string>

namespace client {

    namespace {

        constexpr int kStopSellingMs = 5000; // ms
        constexpr int kShelfPlacesChecked = 2; // a shelf has places 1 and 2; place 2 is checked for "full"
        constexpr int kVendingPlaces = 5; // vending machine has 5 places

    }

    StallPriceWindow::Hooks WorldScene::stallPriceHooks() {
        StallPriceWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.storable = [this](int bagSlot) {
            const auto record = m_services.items.find(m_inventory.bagItem(bagSlot).itemId);
            return record && (record->type == 23 || record->type == 32); // item record types 23 and 32: the kinds a stall accepts
        };
        hooks.caseFree = [this] {
            for (int number = 1; number <= FurnitureStalls::kCases; ++number) {
                if (m_house.stalls().ownCase(number).itemId == 0) {
                    return true;
                }
            }
            return false;
        };
        hooks.putInCase = [this](int furnIndex, int bagSlot, std::int32_t price) {
            m_stallCase.openForPiece(furnIndex);
            m_stallCase.putItem(bagSlot, true, price);
        };
        hooks.repriceCase = [this](std::int32_t price) { m_stallCase.sendPrice(price); };
        return hooks;
    }

    StallCaseWindow::Hooks WorldScene::stallCaseHooks() {
        StallCaseWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.openReprice = [this](int furnIndex, std::int32_t price) { m_stallPrice.openReprice(furnIndex, price); };
        hooks.closePrice = [this] { m_stallPrice.closeForCase(); };
        return hooks;
    }

    CupboardWindow::Hooks WorldScene::cupboardHooks() {
        CupboardWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        return hooks;
    }

    IceBoxWindow::Hooks WorldScene::iceBoxHooks() {
        IceBoxWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.cook = [this](int piece) { openMakeTree(piece); };
        return hooks;
    }

    PledgeBoxWindow::Hooks WorldScene::pledgeBoxHooks() {
        PledgeBoxWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        return hooks;
    }

    MusicCenterWindow::Hooks WorldScene::musicCenterHooks() {
        MusicCenterWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.inHouse = [this] { return m_map && isHouseMap(m_map->id()) && inHouseFloorRoom(); };
        hooks.ownId = [this] { return m_player.id; };
        return hooks;
    }

    GasStorageWindow::Hooks WorldScene::gasStorageHooks() {
        GasStorageWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.marksChanged = [this] { m_itemWindow.refresh(); };
        hooks.closeMakeInfo = [this] { m_makeInfo.close(); };
        return hooks;
    }

    VoyageCompassWindow::Hooks WorldScene::voyageCompassHooks() {
        VoyageCompassWindow::Hooks hooks;
        hooks.mapId = [this] { return m_map ? m_map->id() : 0; };
        hooks.mapSize = [this] { return m_map ? core::Point{m_map->width(), m_map->height()} : core::Point{}; };
        hooks.playerPosition = [this] { return m_player.position; };
        hooks.ownLook = [this]() -> const CharacterLook& { return m_player.look; };
        hooks.bagItem = [this](int slot) { return m_inventory.bagItem(slot).itemId; };
        return hooks;
    }

    BedWindow::Hooks WorldScene::bedHooks() {
        BedWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.petInStall = [this](int place) { return m_petStall.petInStall(place); };
        return hooks;
    }

    FurnitureBuyWindow::Hooks WorldScene::furnitureBuyHooks() {
        FurnitureBuyWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.gold = [this] { return static_cast<std::int32_t>(m_status.gold); };
        return hooks;
    }

    VendingWindow::Hooks WorldScene::vendingHooks() {
        VendingWindow::Hooks hooks;
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.ownId = [this] { return m_player.id; };
        hooks.buy = [this](const StallInFurItem& item) {
            protocol::ItemInstance shown = item.item;
            shown.itemId = item.itemId;
            m_furnitureBuy.open(FurnitureBuyWindow::Kind::Vending, item, stallItemName(m_services.items, shown));
        };
        hooks.askStop = [this](int number) { askStopSelling(number, true); };
        return hooks;
    }

    EggSellWindow::Hooks WorldScene::eggSellHooks() {
        EggSellWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.askTimed = [this](const std::string& text, std::function<void()> onOk) { askTimed(text, std::move(onOk)); };
        return hooks;
    }

    EggBuyWindow::Hooks WorldScene::eggBuyHooks() {
        EggBuyWindow::Hooks hooks;
        hooks.send = [this](const core::Bytes& message) { m_services.connection.send(message); };
        hooks.inFloorRoom = [this] { return inHouseFloorRoom(); };
        hooks.askTimed = [this](const std::string& text, std::function<void()> onOk) { askTimed(text, std::move(onOk)); };
        hooks.gold = [this] { return static_cast<std::int32_t>(m_status.gold); };
        return hooks;
    }

    void WorldScene::askTimed(const std::string& text, std::function<void()> onOk) {
        TalkDialog::Timing timing;
        timing.timeoutMs = kStopSellingMs;
        timing.onTimeout = [] {};
        m_talk.ask(text, std::nullopt, std::move(onOk), [] {}, std::move(timing));
    }

    bool WorldScene::dropOnShelf(int bagSlot, int piece) {
        const int kind = FurnitureKinds::shelfKind(m_services.items, m_house.furniture().piece(piece).itemId);
        if (kind == 0) {
            return false;
        }
        const int slot = m_house.utensils().slotOfPiece(piece);
        const auto& content = m_house.utensils().utensil(slot).contentSlots;
        const bool full = slot != 0 && content[1] != 0 && content[kShelfPlacesChecked] != 0;
        if (const auto refusal = m_stallDrops.check(bagSlot, m_player.id == m_house.furniture().ownerId(), full, false)) {
            m_messages.show(refusal->text, refusal->durationMs);
            return true;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        if (kind == 1) {
            m_stallPrice.open(StallPriceWindow::Mode::Shelf, piece, item.itemId, bagSlot, item.count, 0);
        } else {
            m_stallPrice.open(StallPriceWindow::Mode::Case, piece, item.itemId, bagSlot, item.count, 0);
        }
        return true;
    }

    void WorldScene::dropOnVending(int bagSlot) {
        const int piece = m_vending.furnIndex();
        const int slot = m_house.utensils().slotOfPiece(piece);
        const auto& content = m_house.utensils().utensil(slot).contentSlots;
        bool full = slot != 0;
        for (int place = 1; place <= kVendingPlaces; ++place) {
            full = full && content[static_cast<std::size_t>(place)] != 0;
        }
        int row = m_vending.rowUnderMouse();
        if (const auto refusal = m_stallDrops.check(bagSlot, m_player.id == m_house.furniture().ownerId(), full, true)) {
            m_messages.show(refusal->text, refusal->durationMs);
            return;
        }
        if (row != 0 && content[static_cast<std::size_t>(row)] != 0) {
            row = 0;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        m_stallPrice.open(StallPriceWindow::Mode::Vending, piece, item.itemId, bagSlot, item.count, row);
    }

    void WorldScene::clickShelfItem(int number) {
        const StallInFurItem& item = m_house.stalls().shelf(number);
        if (item.itemId == 0 || m_player.id == m_house.furniture().ownerId()) {
            return;
        }
        const int slot = m_house.utensils().slotOfPiece(item.furnIndex);
        if (slot == 0 || FurnitureKinds::shelfKind(m_services.items, m_house.utensils().utensil(slot).itemId) != 1) {
            return;
        }
        protocol::ItemInstance shown = item.item;
        shown.itemId = item.itemId;
        m_furnitureBuy.open(FurnitureBuyWindow::Kind::Shelf, item, stallItemName(m_services.items, shown));
    }

    void WorldScene::clickCaseItem(int number) {
        const StallInFurItem& item = m_house.stalls().displayed(number);
        if (item.itemId == 0) {
            return;
        }
        if (m_player.id == m_house.furniture().ownerId()) {
            const int slot = m_house.utensils().slotOfPiece(item.furnIndex);
            if (slot != 0 && FurnitureKinds::shelfKind(m_services.items, m_house.utensils().utensil(slot).itemId) == 3) { // shelf kind 3: display case
                m_stallCase.open(item.furnIndex, item.stallIndex, item.itemId, item.price);
            }
            return;
        }
        protocol::ItemInstance shown = item.item;
        shown.itemId = item.itemId;
        m_furnitureBuy.open(FurnitureBuyWindow::Kind::Case, item, stallItemName(m_services.items, shown));
    }

    void WorldScene::askStopSelling(int number, bool vending) {
        if (m_player.id != m_house.furniture().ownerId()) {
            return;
        }
        const StallInFurItem& item = vending ? m_house.stalls().vending(number) : m_house.stalls().shelf(number);
        if (item.itemId == 0) {
            return;
        }
        const int stall = item.stallIndex;
        askTimed("Want to stop selling?", [this, stall, vending] {
            m_services.connection.send(vending ? protocol::takeBackFromVendingRequest(stall) : protocol::takeBackFromShelfRequest(stall));
        });
    }

    bool WorldScene::pressVisitorPiece() {
        const int slot = m_housePainter.hoveredSlot();
        if (!m_house.inFloorRoom() || m_fightState != 0 || slot == 0 || m_player.id == m_house.furniture().ownerId()) {
            return false;
        }
        const int itemId = m_house.utensils().utensil(slot).itemId;
        if (!FurnitureKinds::isVendingMachine(m_services.items, itemId) && !FurnitureKinds::isFortuneEgg(itemId) && !FurnitureKinds::isTvGameConsole(itemId) &&
            FurnitureKinds::cupboardKind(m_services.items, itemId) == 0 && FurnitureKinds::bedKind(m_services.items, itemId) == 0 &&
            !FurnitureKinds::isPiano(m_services.items, itemId) && !FurnitureKinds::isFishBox(m_services.items, itemId) &&
            FurnitureKinds::maidKind(m_services.items, itemId) <= 1 && !FurnitureKinds::isPictureFrame(itemId) &&
            !UtensilSeat::isSeat(m_services.items, itemId)) {
            return false;
        }
        m_pressedVisitorPiece = slot;
        return true;
    }

    void WorldScene::clickVisitorPiece(int slot) {
        constexpr int kReach = 170; // max distance to the piece, px
        constexpr int kTooFarMs = 1500; // "too far" message duration, ms
        if (m_computer.state() == ComputerWindow::State::Running) {
            return;
        }
        const HouseUtensil& utensil = m_house.utensils().utensil(slot);
        const bool tooFar = std::abs(utensil.world.x - m_player.position.x) > kReach || std::abs(utensil.world.y - m_player.position.y) > kReach;
        if (FurnitureKinds::isPictureFrame(utensil.itemId)) {
            if (!tooFar) {
                m_portrait.clicked(utensil.furnIndex, utensil.itemId);
            }
            return;
        }
        if (UtensilSeat::isSeat(m_services.items, utensil.itemId)) {
            clickSeat(utensil.furnIndex);
            return;
        }
        if (FurnitureKinds::isPiano(m_services.items, utensil.itemId)) {
            if (!tooFar) {
                usePiano(utensil.furnIndex, false);
            }
            return;
        }
        if (FurnitureKinds::maidKind(m_services.items, utensil.itemId) > 1) {
            if (tooFar) {
                m_messages.show("too far away", kTooFarMs);
            } else if (!m_teams.isMember(m_player.id)) {
                m_maid.open(utensil.furnIndex);
            }
            return;
        }
        if (FurnitureKinds::isFishBox(m_services.items, utensil.itemId)) {
            if (!tooFar) {
                m_fishTank.showToVisitor();
            }
            return;
        }
        if (FurnitureKinds::isTvGameConsole(utensil.itemId)) {
            if (tooFar) {
                m_messages.show("too far away", kTooFarMs);
            } else {
                openTvConsole(utensil.furnIndex);
            }
            return;
        }
        if (FurnitureKinds::isFortuneEgg(utensil.itemId)) {
            if (tooFar) {
                m_messages.show("too far away", kTooFarMs);
            } else if (makeRefusal().empty()) {
                if (utensil.salePrice == 0 || utensil.eggCount <= 0) {
                    m_messages.show("It is empty", 2000); // message duration in ms
                } else {
                    m_eggBuy.open(utensil.furnIndex);
                }
            }
            return;
        }
        if (FurnitureKinds::cupboardKind(m_services.items, utensil.itemId) != 0 || FurnitureKinds::bedKind(m_services.items, utensil.itemId) != 0) {
            if (tooFar) {
                m_messages.show("too far away", kTooFarMs);
            } else if (FurnitureKinds::bedKind(m_services.items, utensil.itemId) != 0) {
                m_bed.open(utensil.furnIndex);
            } else {
                m_cupboard.open(utensil.furnIndex);
            }
            return;
        }
        if (!tooFar) {
            m_vending.open(utensil.furnIndex);
        }
    }

    void WorldScene::showShelfInfo() {
        const int number = m_housePainter.hoveredShelfItem();
        const int caseNumber = m_housePainter.hoveredCaseItem();
        if (number == 0 && caseNumber == 0) {
            if (m_shelfInfoShown) {
                m_itemInfo.hide();
                m_shelfInfoShown = false;
            }
            return;
        }
        const StallInFurItem& item = number != 0 ? m_house.stalls().shelf(number) : m_house.stalls().displayed(caseNumber);
        protocol::ItemInstance shown = item.item;
        shown.itemId = item.itemId;
        const core::Rect area = number != 0 ? m_housePainter.shelfItemArea(number, m_camera) : m_housePainter.caseItemArea(caseNumber, m_camera);
        m_itemInfo.showStallItem(shown, {area.x, area.y});
        m_shelfInfoShown = true;
    }

}
