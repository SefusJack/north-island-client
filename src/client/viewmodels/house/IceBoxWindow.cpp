#include "client/viewmodels/house/IceBoxWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "protocol/HouseRequests.h"

#include <algorithm>
#include <array>

namespace client {

    namespace {

        constexpr int kScreenWidth = 800; // game screen width, px (keeps the window on screen)
        constexpr int kScreenHeight = 600; // game screen height, px (keeps the window on screen)
        constexpr int kCellSide = 32; // size of one item cell, px
        constexpr int kMaxPlace = 10; // most places in a box
        constexpr int kFreeSearchItem = 10001; // item 10001: placeholder item ("question mark 2") used to look for a free bag slot
        constexpr int kAllowedMark = 1; // bag use mark 1 (compound window mark) that lockNotice lets through
        constexpr std::array<int, 2> kFoodTypes = {23, 32}; // item type 23: cooked food and drinks (Apple Juice, ...); type 32: ingredients (Beef, Banana, ...)
        // Material Cabinet item types, in order
        constexpr std::array<int, 9> kMaterialTypes = {
            28, // type 28: accessories and tools (Fishing Tool, AC Generator, ...)
            31, // type 31: plants and seeds (Apple Seed, Bamboo, ...)
            33, // type 33: metals and ores (Aluminum Ore, ...)
            34, // type 34: stones (Basalt, Granite, ...)
            35, // type 35: clays (Black Clay, ...)
            36, // type 36: woods (Cypress, Fine Log, ...)
            37, // type 37: monster parts (Ant Tooth, Beak, ...)
            38, // type 38: fuels (Gas, Diesel Oil, ...)
            51 // type 51: waters (Boiled Water, Mineral Water, ...)
        };
        constexpr int kAccessoryType = 28; // item type 28: accessories and tools, only stackable ones go in the Material Cabinet
        constexpr std::array<int, 2> kMedicineSubTypes = {113, 118}; // item sub types the Medicine Case takes (medicines)
        constexpr int kSmallCloseY = 26; // y of the small close button, px

        /// @brief Where the controls lie on the form of one box kind.
        struct Layout {
            const char* picture;
            core::Point grid;
            core::Point close;
            int smallCloseX;
            bool cooking;
        };

        // per box kind: form picture, cell grid position (px), close button position (px), small close button x (px, -1 = none), has Cook button
        constexpr Layout kLayouts[] = {
            {"Form_IceBox_1", {27, 64}, {84, 164}, 175, true}, // box 1: Fridge
            {"Form_Cupboard_2", {19, 64}, {75, 144}, 159, false}, // box 2: Material Cabinet
            {"Form_Cupboard_2", {19, 64}, {75, 144}, 159, false}, // box 3: Potion Cabinet
            {"Form_Medical", {22, 40}, {80, 117}, -1, false}, // box 4: Medicine Case
        };

        /// @brief Tells whether a value is in a fixed array.
        /// @tparam N Array length.
        /// @param values Array to search.
        /// @param value Value to look for.
        /// @return True when found.
        template <std::size_t N>
        bool contains(const std::array<int, N>& values, int value) {
            return std::find(values.begin(), values.end(), value) != values.end();
        }

        /// @brief Tells whether an item id is one of the extra items the Potion Cabinet takes.
        /// @param id Item id.
        /// @return True when the cabinet takes it.
        /// @details Original: CanPutInIceBox (AC_Item.pas).
        bool canPutInIceBox(int id) {
            // item ids (and id ranges) the Potion Cabinet accepts
            return id == 32310 || // Holy Water
                   id == 33006 || // Poisonous Potion
                   (id >= 33017 && id <= 33019) || // Strengthening Potion, Detoxifcation Potion, Recovery Potion
                   id == 33022 || // Protection Potion
                   id == 33023 || // Cure Potion
                   id == 33046 || // Recovery Medicine
                   id == 34040 || // Strange Eye-drops
                   (id >= 34107 && id <= 34116) || // 1.5X EXP Potion .. 6X EXP Potion (34107..34116)
                   id == 34123 || // 8X EXP Potion
                   id == 34131 || // 3 Holy EXP Potion
                   id == 34140 || // 1.5 Holy EXP Potion
                   id == 34143 || // 4 Holy EXP Potion
                   id == 34144 || // 5 Holy EXP Potion
                   id == 34146 || // 2 Holy EXP Potion
                   id == 34147 || // 2.5 Holy EXP Potion
                   id == 34173 || // 10X EXP Potion
                   (id >= 34185 && id <= 34190); // 3.5X Holy EXP Potion .. 10X Holy EXP Potion (34185..34190)
        }

        /// @brief Loads IceBoxWindow.xml and returns its window.
        /// @param services Game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("IceBoxWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("IceBoxWindow");
        }

    }

    IceBoxWindow::IceBoxWindow(
        GameServices& services,
        ui::Screen& screen,
        const HouseInterior& house,
        const HouseBoxes& boxes,
        const Inventory& inventory,
        const BagMarks& marks,
        PlayerMessages& messages,
        ItemInfoPopup& info,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_house(house),
          m_boxes(boxes),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_info(info),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_cells(screen.get<ui::IconGrid>("IceBoxCells")),
          m_cooking(screen.get<ui::ImageButton>("IceBoxCooking")),
          m_close(screen.get<ui::ImageButton>("IceBoxClose")),
          m_closeSmall(screen.get<ui::ImageButton>("IceBoxCloseSmall")) {
        screen.on("onIceBoxClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onIceBoxTake", [this](const ui::EventArgs& args) { take(args.index); });
        screen.on("onIceBoxCook", [this](const ui::EventArgs&) {
            if (m_box == 1 && m_hooks.cook) {
                m_hooks.cook(m_piece);
            }
            close();
        });
    }

    int IceBoxWindow::boxOf(int itemId) {
        switch (itemId) {
            case 39023: return 1; // Fridge piece -> box 1
            case 39132: return 2; // Material Cabinet piece -> box 2
            case 39157: return 3; // Potion Cabinet piece -> box 3
            case 39188: return 4; // Medicine Case piece -> box 4
            default: return 0; // not a box piece
        }
    }

    void IceBoxWindow::open(int piece) {
        const int box = boxOf(m_house.furniture().piece(piece).itemId);
        if (box == 0) {
            return;
        }
        m_piece = piece;
        m_box = box;
        const Layout& layout = kLayouts[box - 1];
        m_window.setPicture(layout.picture, m_services.pictures);
        core::Rect area = m_window.bounds();
        area.x = std::min(area.x, kScreenWidth - area.width);
        area.y = std::min(area.y, kScreenHeight - area.height);
        m_window.setBounds(area);
        m_cells.setPosition(layout.grid);
        m_close.setPosition(layout.close);
        m_closeSmall.setVisible(layout.smallCloseX >= 0);
        m_closeSmall.setPosition({std::max(layout.smallCloseX, 0), kSmallCloseY});
        m_cooking.setVisible(layout.cooking);
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    bool IceBoxWindow::accepts(int itemId) const {
        const auto record = m_services.items.find(itemId);
        if (!record) {
            return false;
        }
        switch (m_box) {
            case 1: return contains(kFoodTypes, record->type); // Fridge: food item types
            case 2: // Material Cabinet: material item types, accessories only when stackable
                return contains(kMaterialTypes, record->type) && !(record->type == kAccessoryType && !m_inventory.isStackable(itemId));
            case 3: return canPutInIceBox(itemId); // Potion Cabinet: listed item ids
            default: // Medicine Case: medicine sub types plus four listed item ids
                // extra medicine item ids the case takes
                return contains(kMedicineSubTypes, record->subType) || itemId == 34026 || // Protective EXP Pill
                       itemId == 34029 || // Protective EXP Bolus
                       itemId == 34269 || // Potential Pill
                       itemId == 34289; // Potential Pill (second item)
        }
    }

    void IceBoxWindow::drop(int bagSlot, int place) {
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        // 50 = number of bag slots
        if (bagSlot <= 0 || bagSlot > 50 || item.empty()) {
            return;
        }
        if (m_marks.lockNotice(bagSlot, kAllowedMark)) {
            return;
        }
        if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show("Unable to operate when the item is locked", 1000); // notice duration in ms
            return;
        }
        if (place <= 0) {
            return;
        }
        if (!accepts(item.itemId)) {
            m_messages.show("The item can't be stored", 1000); // notice duration in ms
            return;
        }
        if (place > kMaxPlace) {
            return;
        }
        int target = place;
        if (m_boxes.place(m_box, place).itemId != 0) {
            target = m_boxes.firstFree(m_box);
            if (target == 0) {
                return;
            }
        }
        constexpr int kPutOptions[] = {1, 3, 5, 9}; // 62/52 put options: Fridge, Material Cabinet, Potion Cabinet, Medicine Case
        m_hooks.send(protocol::houseBoxRequest(kPutOptions[m_box - 1], m_piece, target, bagSlot));
    }

    void IceBoxWindow::take(int place) {
        if (place <= 0 || place > kMaxPlace) {
            return;
        }
        if (m_inventory.freeSlotFor(kFreeSearchItem) == 0) {
            m_messages.show("Not enough space", 1000); // notice duration in ms
            return;
        }
        constexpr int kTakeOptions[] = {2, 4, 6, 10}; // 62/52 take options: Fridge, Material Cabinet, Potion Cabinet, Medicine Case
        m_hooks.send(protocol::houseBoxRequest(kTakeOptions[m_box - 1], m_piece, place));
    }

    void IceBoxWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        m_cells.clearCells();
        for (int place = 1; place <= kMaxPlace; ++place) {
            const HouseBoxes::Place& held = m_boxes.place(m_box, place);
            if (held.itemId != 0 && held.count != 0) {
                protocol::ItemInstance item;
                item.itemId = held.itemId;
                item.count = held.count;
                ui::IconCell cell = itemIconCell(m_services, item, true);
                cell.hint.clear();
                m_cells.setCell(place, cell);
            }
        }
        if (m_screen.hoveredId() != m_cells.id()) {
            return;
        }
        const core::Point origin = m_window.bounds().position() + m_cells.bounds().position();
        const int place = m_cells.cellAt(m_screen.mouse() - origin);
        const HouseBoxes::Place& held = place != 0 ? m_boxes.place(m_box, place) : HouseBoxes::Place{};
        if (held.itemId != 0) {
            protocol::ItemInstance item;
            item.itemId = held.itemId;
            item.count = held.count;
            const core::Point at = origin + m_cells.cellPosition(place);
            m_info.show(item, {at.x, at.y, kCellSide, kCellSide});
        }
    }

}
