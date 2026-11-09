#include "client/scenes/world/WorldScene.h"
#include "client/rules/FusionScrolls.h"
#include "protocol/EggMachineRequests.h"

namespace client {

    namespace {

        constexpr int kTransferMirror = 34018; // item 34018: Transfer Mirror
        constexpr int kDivorceItem = 34196; // item 34196: Divorce Seal
        constexpr int kClassCloak = 142; // item sub type 142: job manteau (class cloak)
        constexpr std::size_t kMainBoxWrap = 40; // characters per line in the talk box

    }

    CareerHooks WorldScene::careerHooks() {
        CareerHooks hooks;
        hooks.look = [this] { return m_player.look; };
        hooks.job = [this] { return m_status.job; };
        hooks.statPoints = [this] { return m_status.statPoints; };
        hooks.level = [this] { return m_status.level; };
        hooks.fighting = [this] { return m_fightState != 0; };
        hooks.cloakSlot = [this] { return bagSlotOfKind(kClassCloak); };
        hooks.mirrorSlot = [this] { return m_inventory.nextSlotWith(kTransferMirror); };
        hooks.message = [this](const std::string& text, int durationMs) { m_messages.show(text, durationMs); };
        hooks.ask = [this](const std::string& text, std::function<void()> onOk, std::function<void()> onCancel) {
            TalkDialog::Timing timing;
            timing.wrapChars = kMainBoxWrap;
            timing.exclusive = false;
            m_talk.ask(text, TalkDialog::Speaker{m_player.look, m_player.name}, std::move(onOk), std::move(onCancel), std::move(timing));
        };
        hooks.send = [this](const core::Bytes& bytes) { m_services.connection.send(bytes); };
        hooks.closeOthers = [this] { closeWindows(); };
        hooks.eventGoesOn = [this] {
            m_moveLocked = false;
            m_eventRunner.moveOn();
        };
        hooks.showCareer = [this](int line, int changeMode) { m_career.open(line, changeMode); };
        return hooks;
    }

    void WorldScene::closeWindows() {
        m_mall.hide();
        m_cart.hide();
        m_itemWindow.close();
        if (m_itemBag.isOpen()) {
            m_itemBag.close();
        }
        m_compoundWindow.close();
        m_skillWindow.close();
        m_groupWindow.close();
        m_moneyMenu.close();
        m_socialWindow.close();
        m_systemWindow.close();
    }

    int WorldScene::bagSlotOfKind(int subType) const {
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (!item.empty()) {
                const auto record = m_services.items.find(item.itemId);
                if (record && record->subType == subType) {
                    return slot;
                }
            }
        }
        return 0;
    }

    void WorldScene::showCareerForm(int changeMode) {
        constexpr std::size_t kStrength = 0;
        constexpr std::size_t kIntelligence = 2;
        const int strength = m_status.stats[kStrength];
        const int intelligence = m_status.stats[kIntelligence];
        if (strength > intelligence) {
            m_career.open(1, changeMode);
        } else if (strength < intelligence) {
            m_career.open(2, changeMode);
        } else {
            m_beforeCareer.open(changeMode);
        }
    }

    void WorldScene::onCareerForm() { showCareerForm(0); }

    void WorldScene::onCareerResult(int code) {
        if (code == 1) {
            m_messages.show("Not enough space on the item bar for joining the class ", 3000); // message display time in ms
        }
    }

    bool WorldScene::useSpecialItem(int itemId, int bagSlot) {
        if (itemId == kDivorceItem) {
            if (m_fightState != 0) {
                m_messages.show("Can be used during the battle", 2000); // message display time in ms
            } else if (m_marriage.spouseId() == 0) {
                m_messages.show("can't be used, if you haven't got married", 2000); // message display time in ms
            } else {
                m_marriage.askForcedDivorce(bagSlot);
            }
            return true;
        }
        if (itemId == kTransferMirror) {
            const std::string noClass = "Can't use the item because you have no class.";
            if (m_fightState != 0) {
                m_messages.show("Unable to use in combat", 1500); // message display time in ms
            } else if (m_status.reborn != 1 || m_status.job < 1 || m_status.job > 6) { // jobs 1 to 6 are the classes
                m_messages.show(noClass, 1500); // message display time in ms
            } else if (bagSlotOfKind(kClassCloak) == 0) {
                m_messages.show("Class Cloak should be in Item Frame if you want to change your class.", 3000); // message display time in ms
            } else {
                showCareerForm(1);
            }
            return true;
        }
        if (const auto dye = m_services.items.find(itemId); dye && DyeWindow::partOf(dye->subType) != 0) {
            m_dye.toggle(bagSlot, DyeWindow::partOf(dye->subType));
            return true;
        }
        constexpr int kBlueprintSubType = 103; // item sub type 103: furniture blueprints
        if (const auto blueprint = m_services.items.find(itemId); blueprint && blueprint->subType == kBlueprintSubType) {
            m_blueprint.toggle(itemId);
            m_itemWindow.close();
            return true;
        }
        constexpr int kTearPlier = 34152; // item 34152: Tear Plier
        constexpr int kSmallTearPlier = 34155; // item 34155: Small Tear Plier
        if (itemId == kTearPlier || itemId == kSmallTearPlier) {
            m_dismantle.open(bagSlot, itemId == kTearPlier ? DismantleWindow::kTearPlier : DismantleWindow::kSmallTearPlier);
            return true;
        }
        if (itemId == SewingKitWindow::kSewingTools || itemId == SewingKitWindow::kWeaponModel) {
            m_sewingKit.open(bagSlot);
            return true;
        }
        if (useWishItem(itemId, bagSlot)) {
            return true;
        }
        constexpr int kTokenEgg = 34059; // item 34059: Token Egg
        constexpr int kSlot9 = 9; // equip slot 9 (the egg slot)
        if (const auto egg = m_services.items.find(itemId); itemId == kTokenEgg && egg && egg->equipSlot == kSlot9) {
            m_services.connection.send(protocol::feedEggMachineRequest(bagSlot));
            return true;
        }
        if (itemId == MagicBoxWindow::kBoxItem) {
            m_magicBox.open(bagSlot);
            return true;
        }
        if (const auto tool = m_services.items.find(itemId); tool && tool->subType == RepairWindow::kRepairSubType) {
            m_repair.open(bagSlot);
            return true;
        }
        if (itemId == ReferWindow::kMagnifyingGlass) {
            m_refer.open();
            return true;
        }
        if (itemId == InsteadEcisWindow::kMemoryCard) {
            m_insteadEcis.use(bagSlot);
            return true;
        }
        if (itemId == PackingWindow::kEmptyItem || itemId == PackingWindow::kPackedItem) {
            m_packing.open(bagSlot);
            return true;
        }
        if (isFusionScroll(itemId)) {
            m_reinforce.open(bagSlot);
            return true;
        }
        constexpr int kChaosCrystals[] = {61062, 61079, 61080}; // Chaos Crystal, Shadow Chaos Crystal, Pure Chaos Crystal
        if (std::find(std::begin(kChaosCrystals), std::end(kChaosCrystals), itemId) != std::end(kChaosCrystals)) {
            m_confuseStone.open(bagSlot);
            return true;
        }
        constexpr int kPicnicBox = 34376; // item 34376: Picnic Box
        constexpr int kExpBag = 34377; // item 34377: EXP Bag
        if (itemId == kPicnicBox) {
            m_picnic.open(bagSlot);
            return true;
        }
        if (itemId == kExpBag) {
            m_expBox.open(bagSlot);
            return true;
        }
        constexpr int kGpsItems[] = {34006, 34195, 34373}; // Normal GPS, Advance GPS, Smart GPS
        constexpr int kMemoryCrystal = 37257; // item 37257: Memory Crystal
        if (std::find(std::begin(kGpsItems), std::end(kGpsItems), itemId) != std::end(kGpsItems)) {
            m_gps.open(bagSlot);
            return true;
        }
        if (itemId == kMemoryCrystal) {
            m_gps.askUpgrade(bagSlot);
            return true;
        }
        constexpr int kDogBone = 34017; // item 34017: Dog Bone
        if (itemId == kDogBone) {
            m_forage.open(bagSlot);
            return true;
        }
        constexpr int kAutoPlaySubType = 105; // item sub type 105: auto-play remote controls
        if (const auto remote = m_services.items.find(itemId); remote && remote->subType == kAutoPlaySubType) {
            useRemoteControl(bagSlot);
            return true;
        }
        constexpr int kMagicMagnet = 34021; // item 34021: Magic Magnet
        constexpr int kCarnieVoucher = 34028; // item 34028: Carnie Voucher
        if (itemId == kMagicMagnet) {
            chooseMagnetUse(bagSlot);
            return true;
        }
        if (itemId == kCarnieVoucher) {
            useCarnieVoucher(bagSlot);
            return true;
        }
        constexpr int kStoreTank = 34399; // item 34399: Fuel Bank
        if (itemId == kStoreTank) {
            m_storeTank.use(bagSlot);
            return true;
        }
        // item ids 34163: Teleport Scroll, 34169: Bamboo Dragonfly, 34332: Mini Dragonfly; map mode 1 for 34163, 2 for the others
        if (itemId == 34163 || itemId == 34169 || itemId == 34332) {
            useMagicMapItem(bagSlot, itemId == 34163 ? 1 : 2);
            return true;
        }
        constexpr int kSpaceCapsule = 34270; // item 34270: Space Capsule
        if (itemId == kSpaceCapsule) {
            m_capsule.open(bagSlot);
            return true;
        }
        constexpr int kAntiPotentialPill = 34351; // item 34351: Anti-Potential Pill
        if (itemId == kAntiPotentialPill) {
            m_potentialReturn.open(bagSlot);
            return true;
        }
        constexpr int kExchangeCardType = 40; // item type 40: gift/exchange cards (Bronze Card, Anniversary Card, ...)
        if (const auto card = m_services.items.find(itemId); card && card->type == kExchangeCardType) {
            m_itemWindow.close();
            m_giftExchange.use(bagSlot);
            return true;
        }
        constexpr int kPillModes[3] = {34269, 34289, 34350}; // Potential Pill, Potential Pill (second id), Super Potential Pill
        for (int mode = 0; mode < 3; ++mode) { // three pill modes
            if (itemId == kPillModes[mode]) {
                m_potentialWindow.open(mode);
                return true;
            }
        }
        if (const auto rod = m_services.items.find(itemId); rod && isFishingRod(*rod)) {
            useFishingRod(bagSlot, rod->rank);
            return true;
        }
        if (const auto record = m_services.items.find(itemId); record && vehicleSupplyRefused(*record)) {
            return true;
        }
        return useTentItem(itemId, bagSlot);
    }

}
