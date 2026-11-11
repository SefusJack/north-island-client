#include "client/viewmodels/mall/MallWindow.h"

#include "client/rendering/hud/GameNumbers.h"
#include "client/rendering/painters/ItemCells.h"
#include "client/rules/ItemInfo.h"
#include "client/data/GiftBoxContents.h"
#include "client/rules/Experience.h"
#include "core/Log.h"
#include "gfx/TextStyle.h"
#include "protocol/MallMessages.h"
#include "ui/DrawingArea.h"
#include "ui/TextWrap.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <string>

namespace client {

    namespace {

        constexpr int kCombatSlots = 6; // equip slots 1..6 that count for the try-on (headgear, clothes, ..., shoes, weapon)
        constexpr int kSlotMachines = 6; // mall tab 6: the Game 777 slot machines
        constexpr int kForgeTab = 7; // mall tab 7: the forging room
        constexpr int kSecretMap = 10072; // map 10072: Survival Isle (the Game 777 tab cannot be used there)

        /// @brief Loads MallWindow.xml into the screen and returns its window.
        /// @param services Shared game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The "Mall" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MallWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Mall");
        }

        /// @brief A plain text style in a colour.
        /// @param color Text colour.
        /// @return The text style.
        gfx::TextStyle plain(core::Color color) { return {color, gfx::TextEffect::Plain, std::nullopt}; }

        /// @brief Item a row shows: a gift box shows what is in it.
        /// @param items Item table.
        /// @param itemId Item id of the row.
        /// @return The item id to show.
        int shownItemId(const formats::ItemTable& items, int itemId) {
            const auto item = items.find(itemId);
            // item type 17 (wing/weapon boxes, with an alias item) shows its alias, type 26 (gold/set packs) shows itself
            if (item && ((item->type == 17 && item->aliasItemId != 0) || item->type == 26)) {
                return item->type == 26 ? item->id : item->aliasItemId;
            }
            return itemId;
        }

        /// @brief Text of what an item's two attributes give ("HP+50, SP+20").
        /// @details Original: DrawShopItemDetail.
        /// @param item Item to describe.
        /// @return The bonus text.
        std::string bonusText(const formats::ItemRecord& item) {
            std::string text;
            for (std::size_t k = 0; k < item.attributeKinds.size(); ++k) {
                std::string stat;
                switch (item.attributeKinds[k]) {
                    case 25: stat = "HP"; break; // attribute kind 25: HP
                    case 26: stat = "SP"; break; // attribute kind 26: SP
                    case 207: stat = "MaxHp"; break; // attribute kind 207: max HP
                    case 208: stat = "MaxSp"; break; // attribute kind 208: max SP
                    case 210: stat = "ATK"; break; // attribute kind 210: attack
                    case 211: stat = "DEF"; break; // attribute kind 211: defence
                    case 215: stat = "MATK"; break; // attribute kind 215: magic attack
                    case 216: stat = "MDEF"; break; // attribute kind 216: magic defence
                    case 214: stat = "SPD"; break; // attribute kind 214: speed
                    case 64: stat = "Amity Degree"; break; // attribute kind 64: amity degree
                    case 36: stat = "Exp"; break; // attribute kind 36: experience
                    case 100: stat = "resistance of shoot"; break; // attribute kind 100: resistance of shoot
                    case 250: continue; // attribute kind 250: unused slot, skipped
                    default: break; // other kinds: no stat name
                }
                constexpr int kStink = 33053; // item 33053: Korean Cake (no bonus text for it)
                if (item.id == kStink || bottleKind(item) != 0) {
                    continue;
                }
                if (!text.empty() && !stat.empty()) {
                    text += ", ";
                }
                text += stat;
                const int value = item.attributeValues[k];
                // attribute values are stored offset by 100 (100 = no change)
                if (value == 100) {
                    continue;
                }
                text += value > 100 ? "+" + std::to_string(value - 100) : std::to_string(value - 100);
                // item subtype 113 with kind 36 (exp) and subtype 119 with kind 100 (resistance of shoot) are percentages
                if ((item.subType == 113 && item.attributeKinds[k] == 36) || (item.subType == 119 && item.attributeKinds[k] == 100)) {
                    text += "%";
                }
            }
            if (bottleKind(item) != 0) {
                text = item.description;
            }
            return text;
        }

        /// @brief Adds what an item's attributes give to the bonus array (max HP, max SP, ATK, DEF, MATK, MDEF, SPD).
        /// @param bonus Bonus array to change.
        /// @param item Item whose attributes are added.
        /// @param sign 1 to put the item on, -1 to take it off.
        void addBonus(std::array<int, 7>& bonus, const formats::ItemRecord& item, int sign) {
            for (std::size_t k = 0; k < item.attributeKinds.size(); ++k) {
                // attribute values are stored offset by 100
                const int value = (item.attributeValues[k] - 100) * sign;
                switch (item.attributeKinds[k]) {
                    case 207: bonus[0] += value; break; // attribute kind 207: max HP
                    case 208: bonus[1] += value; break; // attribute kind 208: max SP
                    case 210: bonus[2] += value; break; // attribute kind 210: attack
                    case 211: bonus[3] += value; break; // attribute kind 211: defence
                    case 215: bonus[4] += value; break; // attribute kind 215: magic attack
                    case 216: bonus[5] += value; break; // attribute kind 216: magic defence
                    case 214: bonus[6] += value; break; // attribute kind 214: speed
                    default: break; // other kinds add nothing
                }
            }
        }

    }

    MallWindow::MallWindow(
        GameServices& services,
        ui::Screen& screen,
        MallShop& shop,
        const PlayerStatus& status,
        const MapCharacter& player,
        Inventory& inventory,
        BagMarks& marks,
        PlayerMessages& messages,
        TalkDialog& talk,
        ItemInfoPopup& info,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_shop(shop),
          m_status(status),
          m_player(player),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_info(info),
          m_hooks(std::move(hooks)),
          m_forge(services, inventory, marks, shop, messages, talk, [this] { m_hooks.bagChanged(); }),
          m_window(loadWindow(services, screen)),
          m_bag(screen.get<ui::IconGrid>("MallBag")),
          m_forgeCell(screen.get<ui::IconGrid>("MallForgeItem")),
          m_forgeConfirm(screen.get<ui::ImageButton>("MallForgeConfirm")),
          m_forgeCancel(screen.get<ui::ImageButton>("MallForgeCancel")),
          m_pageNext(screen.get<ui::ImageButton>("MallPageNext")),
          m_pagePrevious(screen.get<ui::ImageButton>("MallPagePrevious")),
          m_textSpot(screen.get<ui::Spot>("MallTextColor")),
          m_descriptionSpot(screen.get<ui::Spot>("MallDescriptionColor")),
          m_gainSpot(screen.get<ui::Spot>("MallGainColor")),
          m_lossSpot(screen.get<ui::Spot>("MallLossColor")),
          m_tabs{
              std::ref(screen.get<ui::ImageButton>("MallTab1")),
              std::ref(screen.get<ui::ImageButton>("MallTab2")),
              std::ref(screen.get<ui::ImageButton>("MallTab3")),
              std::ref(screen.get<ui::ImageButton>("MallTab4")),
              std::ref(screen.get<ui::ImageButton>("MallTab5")),
              std::ref(screen.get<ui::ImageButton>("MallTab6")),
              std::ref(screen.get<ui::ImageButton>("MallTab7"))
          },
          m_addToCart{
              std::ref(screen.get<ui::ImageButton>("MallAddToCart1")),
              std::ref(screen.get<ui::ImageButton>("MallAddToCart2")),
              std::ref(screen.get<ui::ImageButton>("MallAddToCart3")),
              std::ref(screen.get<ui::ImageButton>("MallAddToCart4")),
              std::ref(screen.get<ui::ImageButton>("MallAddToCart5")),
              std::ref(screen.get<ui::ImageButton>("MallAddToCart6")),
              std::ref(screen.get<ui::ImageButton>("MallAddToCart7")),
              std::ref(screen.get<ui::ImageButton>("MallAddToCart8"))
          } {
        screen.get<ui::DrawingArea>("MallPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paint(canvas, at, nowMs); });
        screen.get<ui::DrawingArea>("MallOverlay").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintDescription(canvas, at); });
        screen.on("onMallTab", [this](const ui::EventArgs& args) {
            for (std::size_t i = 0; i < m_tabs.size(); ++i) {
                if (args.source == m_tabs[i].get().name()) {
                    selectTab(static_cast<int>(i) + 1);
                }
            }
        });
        screen.on("onMallAddToCart", [this](const ui::EventArgs& args) {
            for (std::size_t i = 0; i < m_addToCart.size(); ++i) {
                if (args.source == m_addToCart[i].get().name()) {
                    addToCart(static_cast<int>(i) + 1);
                }
            }
        });
        screen.on("onMallPageNext", [this](const ui::EventArgs&) { turnPage(1); });
        screen.on("onMallPagePrevious", [this](const ui::EventArgs&) { turnPage(-1); });
        screen.on("onMallRotateNext", [this](const ui::EventArgs&) { rotate(1); });
        screen.on("onMallRotatePrevious", [this](const ui::EventArgs&) { rotate(-1); });
        screen.on("onMallReset", [this](const ui::EventArgs&) { resetTryOn(); });
        screen.on("onMallPress", [this](const ui::EventArgs&) {
            if (m_hoverPack != 0) {
                m_hooks.showContents(m_hoverPack);
                m_hoverPack = 0;
            }
            if (m_tab == kSlotMachines) {
                requestGame();
            } else {
                tryOn();
            }
        });
        screen.on("onMallForgeSelect", [this](const ui::EventArgs& args) { m_forge.select(args.index); });
        screen.on("onMallForgeRelease", [this](const ui::EventArgs& args) { m_forge.release(args.index); });
        screen.on("onMallForgeCancel", [this](const ui::EventArgs&) { m_forge.cancel(); });
        screen.on("onMallForgeConfirm", [this](const ui::EventArgs&) { m_forge.confirm(); });
        screen.on("onMallOpenCart", [this](const ui::EventArgs&) { m_hooks.toggleCart(); });
        screen.on("onMallExit", [this](const ui::EventArgs&) { hide(); });
        m_tabs[0].get().setSelected(true);
        m_tryOnRoom = services.pictures.loadFile(services.game.data("Menu/Skins/white/Form_TryClotheRoom_1.bmp"));
        m_rowsRoom = services.pictures.loadFile(services.game.data("Menu/Skins/white/Form_Shopping_2.bmp"));
    }

    void MallWindow::toggle() {
        if (m_hooks.fighting()) {
            return;
        }
        if (!m_shop.loaded()) {
            m_messages.show("Item list is not at hand", 1000); // message display time in ms
            m_services.connection.send(protocol::mallCatalogRequest());
            return;
        }
        if (m_window.isVisible()) {
            hide();
            return;
        }
        show();
        if (m_window.isVisible()) {
            resetTryOn();
            m_services.connection.send(protocol::imPointsRequest(0));
        }
    }

    void MallWindow::show() {
        if (m_hooks.fighting()) {
            return;
        }
        m_window.setVisible(true);
        m_shop.gameAsked = false;
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_hooks.hideCart();
        m_forge.open();
        selectTab(m_tab);
    }

    void MallWindow::hide() {
        m_window.setVisible(false);
        m_forge.close();
        m_hooks.hideContents();
    }

    void MallWindow::selectTab(int tab) {
        if (m_hooks.fighting()) {
            return;
        }
        if (tab == kSlotMachines) {
            if (!m_gamesTab) {
                m_messages.show(" Hasen't been used", 1000); // message display time in ms
                if (m_tab != kSlotMachines) {
                    return;
                }
                tab = 1;
            } else if (m_hooks.mapId() == kSecretMap) {
                m_messages.show("Cannot use this function", 1000); // message display time in ms
                return;
            }
        }
        m_tabs[static_cast<std::size_t>(m_tab - 1)].get().setSelected(false);
        if (tab != m_tab) {
            m_shop.page = 1;
        }
        m_tab = tab;
        if (m_shop.page > m_shop.pageCount(m_tab)) {
            m_shop.page = 1;
        }
        m_tabs[static_cast<std::size_t>(m_tab - 1)].get().setSelected(true);
        for (auto& button : m_addToCart) {
            button.get().setVisible(m_tab <= MallShop::kTabs);
        }
        m_pageNext.setVisible(m_tab <= kSlotMachines);
        m_pagePrevious.setVisible(m_tab <= kSlotMachines);
        // 4 controls: the bag, the slot and the two buttons
        for (ui::Element& control : std::array<std::reference_wrapper<ui::Element>, 4>{m_bag, m_forgeCell, m_forgeConfirm, m_forgeCancel}) {
            control.setVisible(m_tab == kForgeTab);
        }
        // pose 1 (battle stance) for tab 3, pose 2 (shop pose) for tabs 4, 5 and the forging room, else standing
        setPose(m_tab == 3 ? 1 : m_tab == 4 || m_tab == 5 || m_tab == kForgeTab ? 2 : 0);
        updatePageButtons();
    }

    void MallWindow::turnPage(int step) {
        if (m_hooks.fighting()) {
            return;
        }
        m_shop.page = std::clamp(m_shop.page + step, 1, std::max(1, m_shop.pageCount(m_tab)));
        updatePageButtons();
    }

    void MallWindow::updatePageButtons() {
        const int pages = m_shop.pageCount(m_tab);
        m_pageNext.setAnimating(m_shop.page < pages);
        m_pagePrevious.setAnimating(m_shop.page > 1);
    }

    void MallWindow::addToCart(int slot) {
        if (m_hooks.fighting()) {
            return;
        }
        if (static_cast<int>(m_shop.cart().size()) + 1 > MallShop::kCartLines) {
            m_messages.show("The cart is full", 1500); // message display time in ms
            return;
        }
        const std::vector<protocol::MallRow>& rows = m_shop.tab(m_tab);
        const std::size_t index = static_cast<std::size_t>((m_shop.page - 1) * MallShop::kPageRows + slot - 1);
        if (index >= rows.size()) {
            return;
        }
        const protocol::MallRow& row = rows[index];
        if (m_shop.addToCart(row)) {
            const auto item = m_services.items.find(row.itemId);
            m_messages.show("Add [" + (item ? item->name : std::string()) + "] into cart", 1500); // message display time in ms
        }
        m_hooks.showCart();
    }

    void MallWindow::rotate(int step) {
        // figure directions per pose, {first, last}: pose 0 standing 8..15, pose 1 battle stance 16..17, pose 2 shop pose 46..53
        static constexpr std::array<std::array<int, 2>, 3> kRanges{{{8, 15}, {16, 17}, {46, 53}}};
        const auto& range = kRanges[static_cast<std::size_t>(m_pose)];
        m_direction += step;
        if (m_direction > range[1]) {
            m_direction = range[0];
        } else if (m_direction < range[0]) {
            m_direction = range[1];
        }
    }

    void MallWindow::setPose(int pose) {
        // starting direction per pose (0 standing, 1 battle stance, 2 shop pose), each inside its kRanges range
        static constexpr std::array<int, 3> kDirections{13, 17, 51};
        m_pose = pose >= 1 && pose <= 2 ? pose : 0; // valid poses are 0..2
        m_direction = kDirections[static_cast<std::size_t>(m_pose)];
    }

    void MallWindow::resetTryOn() {
        std::array<int, 7> bonus{}; // max HP, max SP, ATK, DEF, MATK, MDEF, SPD
        for (int slot = 1; slot <= kCombatSlots; ++slot) {
            if (const auto item = m_services.items.find(m_inventory.equipped(slot).itemId)) {
                addBonus(bonus, *item, 1);
            }
        }
        m_base = {
            m_status.maxHp - bonus[0],
            m_status.maxSp - bonus[1],
            m_status.combat[0] - bonus[2],
            m_status.combat[1] - bonus[3],
            m_status.combat[2] - bonus[4],
            m_status.combat[3] - bonus[5],
            m_status.combat[4] - bonus[6]
        };
        m_tryOnBonus = {};
        m_tried = {};
        m_wearing = m_inventory.equipmentIds();
        m_look = m_player.look;
        m_tryOnCleared = true;
        setPose(m_tab == 3 ? 1 : m_tab == 4 ? 2 : 0); // pose 1 for tab 3, pose 2 for tab 4, else standing
    }

    void MallWindow::tryOn() {
        const int place = hoveredPlace();
        if (place < 1 || m_tab < 1 || m_tab > 3) { // only tabs 1..3 (wearable goods) can be tried on
            return;
        }
        m_lastTried = place;
        const std::vector<protocol::MallRow>& rows = m_shop.tab(m_tab);
        const std::size_t index = static_cast<std::size_t>((m_shop.page - 1) * MallShop::kPageRows + place - 1);
        if (index >= rows.size() || (rows[index].category != 1 && rows[index].category != 2)) { // mall categories 1 and 2: armour, weapons
            return;
        }
        int itemId = rows[index].itemId;
        constexpr int kSetA = 35131; // item 35131: M-Dream Xmas Set (male bodies 1 and 3)
        constexpr int kSetB = 35132; // item 35132: F-Dream Xmas Set (female bodies 2 and 4)
        if (itemId == kSetA || itemId == kSetB) {
            // worn item per equip slot 1..6, 0 = none
            static constexpr std::array<std::uint16_t, 6>
                kLooksA{22529, 21315, 0, 0, 24222, 0}; // M-Dream Xmas Hat, M-Dream Xmas Suit, -, -, M-Dream Xmas Boots, -
            static constexpr std::array<std::uint16_t, 6>
                kLooksB{22530, 21316, 0, 0, 24223, 0}; // F-Dream Xmas Hat, F-Dream Xmas Suit, -, -, F-Dream Xmas Boots, -
            const bool fits = itemId == kSetA ? (m_look.bodyType == 1 || m_look.bodyType == 3) : (m_look.bodyType == 2 || m_look.bodyType == 4);
            if (!fits) {
                m_messages.show("Unable to equip this gear", 2000); // message display time in ms
                return;
            }
            const auto& looks = itemId == kSetA ? kLooksA : kLooksB;
            for (std::size_t slot = 0; slot < looks.size(); ++slot) {
                if (looks[slot] != 0 && m_wearing[slot] != looks[slot]) {
                    wear(static_cast<int>(slot) + 1, looks[slot]);
                }
            }
            m_tryOnCleared = false;
            dress();
            return;
        }
        itemId = shownItemId(m_services.items, itemId);
        const auto item = m_services.items.find(itemId);
        if (!item || item->equipSlot < 1 || item->equipSlot > kCombatSlots || item->petNpcId != 0) {
            return;
        }
        const auto slot = static_cast<std::size_t>(item->equipSlot - 1);
        if (m_wearing[slot] != itemId) {
            wear(item->equipSlot, itemId);
        } else {
            if (m_tried[slot] != 0) {
                addBonus(m_tryOnBonus, *item, -1);
            }
            m_tried[slot] = 0;
            m_wearing[slot] = 0;
            m_messages.show("Try on [" + item->name + "] taken off", 1500); // message display time in ms
        }
        m_tryOnCleared = false;
        dress();
    }

    void MallWindow::wear(int equipSlot, int itemId) {
        const auto slot = static_cast<std::size_t>(equipSlot - 1);
        if (m_tried[slot] != 0) {
            if (const auto previous = m_services.items.find(m_wearing[slot])) {
                addBonus(m_tryOnBonus, *previous, -1);
            }
        }
        const auto item = m_services.items.find(itemId);
        if (item) {
            addBonus(m_tryOnBonus, *item, 1);
        }
        m_tried[slot] = static_cast<std::uint16_t>(itemId);
        m_wearing[slot] = static_cast<std::uint16_t>(itemId);
        m_messages.show("Try on [" + (item ? item->name : std::string()) + "]", 1500); // message display time in ms
    }

    void MallWindow::dress() {
        m_look = CharacterLook::dressed(m_player.look.bodyType, m_player.look.head, m_player.look.basePattern, m_wearing, m_services.items);
    }

    std::vector<protocol::MallRow> MallWindow::pageRows() const {
        std::vector<protocol::MallRow> rows;
        if (m_tab < 1 || m_tab > MallShop::kTabs) {
            return rows;
        }
        const std::vector<protocol::MallRow>& tab = m_shop.tab(m_tab);
        for (int j = (m_shop.page - 1) * MallShop::kPageRows; j < m_shop.page * MallShop::kPageRows && j < static_cast<int>(tab.size()); ++j) {
            if (tab[static_cast<std::size_t>(j)].itemId != 0) {
                rows.push_back(tab[static_cast<std::size_t>(j)]);
            }
        }
        return rows;
    }

    int MallWindow::hoveredPlace() const {
        const core::Point mouse = m_screen.mouse();
        for (int i = 1; i <= MallShop::kPageRows; ++i) {
            // row cell: 2 columns 267 px apart from x 243, 112 px row pitch from y 70, size 100x90
            const core::Rect slot{(i - 1) % 2 * 267 + 243, (i - 1) / 2 * 112 + 70, 100, 90};
            if (slot.contains(mouse)) {
                return i;
            }
        }
        return 0;
    }

    int MallWindow::hoveredGame() const {
        const core::Point mouse = m_screen.mouse();
        for (int i = 1; i <= kGameIcons; ++i) {
            // game icon: 2 columns 266 px apart from x 236, 104 px row pitch from y 68, size 247x72
            const core::Rect slot{(i - 1) % 2 * 266 + 236, (i - 1) / 2 * 104 + 68, 247, 72};
            if (slot.contains({mouse.x, mouse.y - 18})) { // the test point is 18 px up
                return i;
            }
        }
        return 0;
    }

    void MallWindow::requestGame() {
        // game kind sent to the server per icon 1..8: Lucky Wheel, Baby Dolls, Lucky 7, Lucky Cards, Diamond Hit, Fortune Egg, Lucky 7 Mini, Jigsaw
        static constexpr std::array<int, kGameIcons> kKinds{21, 7, 8, 16, 19, 6, 10, 14};
        const int icon = hoveredGame();
        if (icon == 0 || m_hooks.fighting()) {
            return;
        }
        m_shop.gameAsked = true;
        m_services.connection.send(protocol::mallGameRequest(kKinds[static_cast<std::size_t>(icon) - 1]));
    }

    void MallWindow::refresh() {
        m_bag.clearCells();
        for (int slot = 1; slot <= BagMarks::kSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (!item.empty() && !m_marks.isHidden(slot)) {
                m_bag.setCell(slot, itemIconCell(m_services, item, true, m_marks.isBusy(slot)));
            }
        }
        m_forgeCell.clearCells();
        if (m_forge.bagSlot() != 0) {
            m_forgeCell.setCell(1, itemIconCell(m_services, m_forge.item(), true));
        }
    }

    void MallWindow::update() {
        if (m_window.isVisible() && m_hooks.fighting()) {
            hide();
        }
        if (hoveredPlace() == 0) {
            m_lastTried = 0;
        }
        m_hoverPack = 0;
        if (const int place = hoveredPlace(); m_window.isVisible() && place >= 1 && m_tab >= 1 && m_tab <= 4) { // tabs 1 to 4 hold packs
            const std::vector<protocol::MallRow>& rows = m_shop.tab(m_tab);
            const std::size_t index = static_cast<std::size_t>((m_shop.page - 1) * MallShop::kPageRows + place - 1);
            if (index < rows.size()) {
                const int itemId = rows[index].itemId;
                if (GiftBoxContents::isRandomPack(m_services.items, itemId) || m_hooks.luckyBag(itemId, m_screen.context().nowMs)) {
                    m_hoverPack = itemId;
                }
            }
        }
        if (!m_window.isVisible() || m_tab != kForgeTab) {
            return;
        }
        const int hovered = m_screen.hoveredId();
        for (const ui::IconGrid& grid : std::array<std::reference_wrapper<const ui::IconGrid>, 2>{m_bag, m_forgeCell}) {
            if (grid.id() != hovered) {
                continue;
            }
            const bool bag = grid.id() == m_bag.id();
            const core::Point origin = m_window.bounds().position() + grid.bounds().position();
            const int cell = grid.cellAt(m_screen.mouse() - origin);
            const protocol::ItemInstance& item = bag ? m_inventory.bagItem(std::max(cell, 0)) : m_forge.item();
            if (cell != 0 && !item.empty() && (!bag || !m_marks.isHidden(cell))) {
                const core::Point at = origin + grid.cellPosition(cell);
                const core::Size cells = m_inventory.cellsOf(item.itemId);
                m_info.show(item, core::Rect{at.x, at.y, cells.width * 34 - 2, cells.height * 34 - 2}); // bag cell is 34 px, minus 2 px gap
            }
        }
    }

    void MallWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        if (m_tryOnRoom) {
            canvas.drawPicture(*m_tryOnRoom, {at.x + 20, at.y + 114}); // try-on room picture position (20, 114)
        }
        const std::string points = std::to_string(m_shop.points);
        // points right-aligned in 8 digits of 8 px from x 120, y 129; box height 12
        canvas.drawText(
            {at.x + 120 + (8 - static_cast<int>(points.size())) * 8, at.y + 129},
            points,
            plain(m_textSpot.color()),
            static_cast<int>(points.size()) * 8,
            12
        );
        if (m_tab == kForgeTab) {
            paintForge(canvas, at);
        } else if (m_tab == kSlotMachines) {
            paintGames(canvas, at);
        } else {
            if (m_rowsRoom) {
                canvas.drawPicture(*m_rowsRoom, {at.x + 228, at.y + 42}); // rows room picture position (228, 42)
            }
            const std::string page = std::to_string(m_shop.page) + " / " + std::to_string(m_shop.pageCount(m_tab));
            const int extra = static_cast<int>(std::to_string(m_shop.page).size()) - 1;
            // page text at x 474 (8 px left per extra digit), y 489; 8 px per character, height 12
            canvas.drawText({at.x + 474 - extra * 8, at.y + 489}, page, plain(m_textSpot.color()), static_cast<int>(page.size()) * 8, 12);
            paintRows(canvas);
        }
        paintTryOnRoom(canvas, at, nowMs);
    }

    void MallWindow::paintGames(gfx::Canvas& canvas, core::Point at) {
        for (int i = 1; i <= kGameIcons; ++i) {
            // icon grid as in hoveredGame (266/104 px pitch, from 236/68), names 18 px above the frames; picture 9 for icon 1
            if (const auto mark = m_services.pictures.find("Icon_Game777_" + std::to_string(i == 1 ? 9 : i))) {
                canvas.drawPicture(*mark, {at.x + (i - 1) % 2 * 266 + 236, at.y + (i - 1) / 2 * 104 + 68 - 18});
            }
        }
        if (const int hover = hoveredGame(); hover != 0) {
            if (const auto frame = m_services.pictures.find("Icon_Game777_0")) {
                canvas.drawPicture(*frame, {at.x + (hover - 1) % 2 * 266 + 236, at.y + (hover - 1) / 2 * 104 + 68}); // hover frame on the icon grid
            }
        }
        if (const auto attention = m_services.pictures.find("Icon_Attention_1")) {
            canvas.drawPicture(*attention, {at.x + 239, at.y + 463}); // notice picture position (239, 463)
        }
        const std::string page = std::to_string(m_shop.page) + " / " + std::to_string(m_shop.pageCount(m_tab));
        const int extra = static_cast<int>(std::to_string(m_shop.page).size()) - 1;
        // page text at x 474 (8 px left per extra digit), y 489; 8 px per character, height 12
        canvas.drawText({at.x + 474 - extra * 8, at.y + 489}, page, plain(m_textSpot.color()), static_cast<int>(page.size()) * 8, 12);
    }

    void MallWindow::paintForge(gfx::Canvas& canvas, core::Point at) {
        if (const auto room = m_services.pictures.find("Form_ BeatScaffold_1")) {
            canvas.drawPicture(*room, {at.x + 271, at.y + 47}); // forging room picture position (271, 47)
        }
        const std::string cost = std::to_string(m_forge.cost());
        // cost right-aligned to x 271+403, y 65 (8 px per digit); box height 12
        canvas.drawText(
            {at.x + 271 + 403 - (static_cast<int>(cost.size()) - 1) * 8, at.y + 65},
            cost,
            plain(m_textSpot.color()),
            static_cast<int>(cost.size()) * 8,
            12
        );
        const std::string level = m_forge.levelText();
        // level right-aligned to x 271+373, y 87 (8 px per character); box height 12
        canvas.drawText(
            {at.x + 271 + 373 - (static_cast<int>(level.size()) - 1) * 8, at.y + 87},
            level,
            plain(m_textSpot.color()),
            static_cast<int>(level.size()) * 8,
            12
        );
    }

    void MallWindow::paintRows(gfx::Canvas& canvas) {
        int shown = 0;
        for (const protocol::MallRow& row : pageRows()) {
            ++shown;
            const int left = (shown - 1) % 2 * 267 + 268; // 2 columns 267 px apart, the first at x 268
            const int top = (shown - 1) / 2 * 112 + 94; // rows 112 px apart, the first at y 94
            const auto item = m_services.items.find(row.itemId);
            if (!item) {
                continue;
            }
            constexpr int kNoShopPicture = 1000; // shop picture id 1000: the item has no shop picture
            if (item->shopPicId != kNoShopPicture) {
                if (const auto picture = m_services.pictures.find(std::to_string(item->shopPicId), {}, gfx::PictureLibrary::Source::Bitmap)) {
                    canvas.drawPicture(*picture, {left - 29, top - 29}); // shop picture offset (-29, -29) from the row
                }
                // without a shop picture the item icon is stretched to 50x50
            } else if (const auto icon = m_services.pictures.findStretched(std::to_string(item->iconId), 50, 50)) {
                canvas.drawPictureScaled(*icon, {left, top, 50, 50});
            }
            const auto shown_ = m_services.items.find(shownItemId(m_services.items, row.itemId));
            if (!shown_) {
                continue;
            }
            std::string name = item->name;
            if (row.category == 4) { // mall category 4: sold in packs
                name += "(" + std::to_string(row.count) + " Quantity)";
            } else if (isMultiUse(*shown_) && shown_->maxUses > 0) {
                name += "(" + std::to_string(shown_->maxUses) + "times)";
            }
            // name at (+92, -28), wrapped at 18 characters; text box 144x15
            canvas.drawText({left + 92, top - 28}, ui::wrapTextLines(name, 18), plain(m_textSpot.color()), 144, 15);
            const int nameLines = name.size() > 18 ? static_cast<int>(name.size()) / 18 : 0; // names wrap at 18 characters
            int lines = 0;
            if (shown_->requiredLevel > m_status.level) {
                // level line at (+92, -13); text box 144x15
                canvas.drawText({left + 92, top - 13}, "Required Level" + std::to_string(shown_->requiredLevel), plain(m_textSpot.color()), 144, 15);
                lines = 1;
            }
            const auto meaningful = [](int kind) { return kind != 0 && kind != 250; }; // attribute kinds 0 and 250 mean no attribute
            if (meaningful(shown_->attributeKinds[0]) || meaningful(shown_->attributeKinds[1])) {
                // bonus text at (+92, -13) below the level and name lines of 15 px each; box 144x15
                canvas.drawText({left + 92, top - 13 + lines * 15 + nameLines * 15}, bonusText(*shown_), plain(m_textSpot.color()), 144, 15);
            }
            const std::string price = std::to_string(row.price);
            // price right-aligned to x +156 (8 px per digit), y +34; box height 12
            canvas.drawText(
                {left + 156 - (static_cast<int>(price.size()) - 1) * 8, top + 34},
                price,
                plain(m_textSpot.color()),
                static_cast<int>(price.size()) * 8,
                12
            );
            if (row.discount < 100) { // discount is a percentage of the price, 100 = none
                if (const auto notThis = m_services.pictures.find("Icon_NotThis")) {
                    canvas.drawPicture(*notThis, {left + 128, top + 38}); // strike-through picture offset (128, 38)
                }
                if (const auto onSale = m_services.pictures.find("Icon_OnSale")) {
                    canvas.drawPicture(*onSale, {left + 165, top + 33}); // on-sale picture offset (165, 33)
                }
                const std::string sale = std::to_string(MallShop::linePrice(row));
                // sale price right-aligned to x +228 (8 px per digit), y +34; box height 12
                canvas.drawText(
                    {left + 228 - (static_cast<int>(sale.size()) - 1) * 8, top + 34},
                    sale,
                    plain(m_textSpot.color()),
                    static_cast<int>(sale.size()) * 8,
                    12
                );
                // badges exist for discounts 10, 20 .. 90 percent
                const bool tens = row.discount >= 10 && row.discount <= 90 && row.discount % 10 == 0;
                const std::string badge = tens ? "Icon_Abate_" + std::to_string(row.discount / 10) : "Icon_OnSaleMark";
                if (const auto mark = m_services.pictures.find(badge); mark && row.hot == 0) {
                    canvas.drawPicture(*mark, {left + 28, top + 23}); // discount badge offset (28, 23)
                }
            }
            // row marks by hot value: 1 new, 2 hot, 3 time, 4 special item; mark at (+28, +23), special item at (+30, +27)
            static constexpr std::array<const char*, 5> kMarks{"", "Icon_New", "Icon_Hot", "Icon_Time", "Icon_SpecItem"};
            if (row.hot >= 1 && row.hot <= 4) {
                if (const auto mark = m_services.pictures.find(kMarks[static_cast<std::size_t>(row.hot)])) {
                    canvas.drawPicture(*mark, row.hot == 4 ? core::Point{left + 30, top + 27} : core::Point{left + 28, top + 23});
                }
            }
        }
    }

    void MallWindow::paintTryOnRoom(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        const core::Point feet{at.x + 116, at.y + 306}; // figure's feet position (116, 306)
        const int count = m_services.characters.figureFrameCount(m_look, m_direction);
        m_services.characters.drawFigure(canvas, m_look, feet, m_direction, m_animation.frame(nowMs, count));
        drawLevelDigits(canvas, m_status.level, {at.x + 35, at.y + 192}); // level digits position (35, 192)
        static constexpr std::array<const char*, 7> kJobIcons{"", "Icon_Asn", "Icon_Samurai", "Icon_Knight", "Icon_Sage", "Icon_Clergyman", "Icon_Prophet"};
        if (m_status.job >= 1 && m_status.job <= 6) { // jobs 1..6 have an icon
            if (const auto icon = m_services.pictures.find(kJobIcons[static_cast<std::size_t>(m_status.job)])) {
                canvas.drawPicture(*icon, {at.x + 39, at.y + 212}); // job icon position (39, 212)
            }
        }
        canvas.drawText({at.x + 65, at.y + 165}, m_player.name, plain(m_textSpot.color()), 138, 20); // name at (65, 165); text box 138x20
        if (const auto element = m_services.pictures.find("Icon_Element_" + std::to_string(m_status.element) + "_1")) {
            canvas.drawPicture(*element, {at.x * 2 + 23, at.y * 2 + 139}); // element panel offset (23, 139) from the form position, counted twice
        }
        const auto bar = [&](const char* frame, core::Point place) {
            if (const auto picture = m_services.pictures.find(frame)) {
                canvas.drawPicture(*picture, {at.x + place.x, at.y + place.y});
            }
        };
        const auto fill = [&](const char* name, core::Point place, double ratio) {
            if (const auto picture = m_services.pictures.find(name)) {
                const gfx::Picture& data = m_services.pictures.picture(*picture);
                const int width = static_cast<int>(std::ceil(data.width() * std::clamp(ratio, 0.0, 1.0)));
                canvas.drawPicture(*picture, {at.x + place.x, at.y + place.y}, core::Rect{0, 0, width, data.height()});
            }
        };
        // bar frames at x 60, y 330 / 344 / 358 (HP, SP, exp); fills at x 62, y 332 / 346 / 360
        bar("Panel34", {60, 330});
        bar("Panel35", {60, 344});
        const int maxHp = m_base[0] + m_tryOnBonus[0];
        const int maxSp = m_base[1] + m_tryOnBonus[1];
        fill("Panel31", {62, 332}, maxHp > 0 ? static_cast<double>(m_status.hp) / maxHp : 0.0);
        fill("Panel32", {62, 346}, maxSp > 0 ? static_cast<double>(m_status.sp) / maxSp : 0.0);
        bar("Panel36", {60, 358});
        const double progress = std::min(1.0, levelProgress(m_services.formula, m_status.level, m_status.exp, m_status.reborn));
        fill("Panel33", {62, 360}, progress);
        // text rows: HP at y 326, SP 340, exp percent 354 (x 129), gold 370 at x 61, stat points 370 at x 176; boxes 200x20
        const auto shade = [&](int bonus, bool gained) {
            return bonus == 0 || m_tryOnCleared ? m_textSpot.color() : gained ? m_gainSpot.color() : m_lossSpot.color();
        };
        canvas.drawText(
            {at.x + 129, at.y + 326},
            std::to_string(m_status.hp) + "/" + std::to_string(maxHp),
            plain(shade(m_tryOnBonus[0], maxHp > m_base[0])),
            200,
            20
        );
        canvas.drawText(
            {at.x + 129, at.y + 340},
            std::to_string(m_status.sp) + "/" + std::to_string(maxSp),
            plain(shade(m_tryOnBonus[1], maxSp > m_base[0])),
            200,
            20
        );
        canvas.drawText({at.x + 129, at.y + 354}, std::to_string(static_cast<int>(std::trunc(progress * 100))) + "%", plain(m_textSpot.color()), 200, 20);
        canvas.drawText({at.x + 61, at.y + 370}, std::to_string(m_status.gold), plain(m_textSpot.color()), 200, 20);
        canvas.drawText({at.x + 176, at.y + 370}, std::to_string(m_status.statPoints), plain(m_textSpot.color()), 200, 20);
        // 5 attribute rows (ATK to SPD), 16 px apart from y 390; stat values at x 146, base values at x 60; boxes 200x20
        for (int i = 0; i < 5; ++i) {
            const int y = at.y + 390 + i * 16;
            canvas.drawText({at.x + 146, y}, std::to_string(m_status.stats[static_cast<std::size_t>(i)]), plain(m_textSpot.color()), 200, 20);
            const int base = m_base[static_cast<std::size_t>(i + 2)];
            const int bonus = m_tryOnBonus[static_cast<std::size_t>(i + 2)];
            if (bonus == 0) {
                canvas.drawText({at.x + 60, y}, std::to_string(base), plain(m_textSpot.color()), 200, 20);
            } else {
                canvas.drawText(
                    {at.x + 60, y},
                    std::to_string(base) + (bonus > 0 ? "+" : "") + std::to_string(bonus),
                    plain(
                        m_tryOnCleared ? m_textSpot.color()
                        : bonus > 0    ? m_gainSpot.color()
                                       : m_lossSpot.color()
                    ),
                    200,
                    20
                );
            }
        }
    }

    void MallWindow::paintDescription(gfx::Canvas& canvas, core::Point at) {
        const int place = hoveredPlace();
        if (place < 1 || m_tab > MallShop::kTabs) {
            return;
        }
        const std::vector<protocol::MallRow>& tab = m_shop.tab(m_tab);
        const std::size_t index = static_cast<std::size_t>((m_shop.page - 1) * MallShop::kPageRows + place - 1);
        if (index >= tab.size()) {
            return;
        }
        const int itemId = tab[index].itemId;
        const auto item = m_services.items.find(shownItemId(m_services.items, itemId));
        if (!item || item->description.empty()) {
            return;
        }
        // hover area of a row's picture: 2 columns 267 px apart from x 273, 112 px row pitch from y 96, size 70x64
        const core::Rect slot{(place - 1) % 2 * 267 + 273, (place - 1) / 2 * 112 + 96, 70, 64};
        if (!slot.contains(m_screen.mouse())) {
            return;
        }
        std::string text = item->description;
        if (GiftBoxContents::isRandomPack(m_services.items, itemId)) {
            constexpr const char* kSeeContents = "Click to see specific information.    ";
            text = std::string(kSeeContents) + text;
        }
        // tabs 1..3 can be tried on; equip places 1..6; items usable by 8, 9 and 11 cannot be tried on in tab 1
        bool canTryOn = m_tab >= 1 && m_tab <= 3 && !GiftBoxContents::isRandomPack(m_services.items, itemId);
        if (m_tab == 1) {
            canTryOn = canTryOn && item->equipSlot >= 1 && item->equipSlot <= 6 && item->usableBy != 8 && item->usableBy != 9 && item->usableBy != 11;
        }
        if (canTryOn && m_lastTried != place) {
            text = "Click to try it on." + text;
        }
        text = ui::wrapTextLines(text, 20); // description wraps at 20 characters
        const int lines = static_cast<int>(text.size()) / 20 + 2; // 20 characters per line, plus 2 spare lines
        int boxLeft = 142; // x of the box beside a left-column row
        int boxRow = place / 2;
        if (place % 2 == 0) {
            boxLeft = 413; // x of the box beside a right-column row
            boxRow -= 1;
        }
        // box origin (197, 44) plus the column x and 112 px per row; 179 px wide, 15 px per line
        const core::Rect box{at.x + 197 + boxLeft, at.y + 44 + boxRow * 112, 179, lines * 15};
        // fill colour RGB 3D8BF9 at alpha 200
        canvas.fillRectAlpha(box, {0x3D, 0x8B, 0xF9, 200});
        canvas.drawRectOutline(box, {0, 0, 0x80, 255}, 1); // dark blue outline 0x80, opaque (alpha 255)
        // text at (207, 54) from the form plus the box column and row; text box 160x15
        canvas.drawText({at.x + 207 + boxLeft, at.y + 54 + boxRow * 112}, text, plain(m_descriptionSpot.color()), 160, 15);
    }

}
