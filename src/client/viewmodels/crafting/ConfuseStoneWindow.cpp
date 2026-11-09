#include "client/viewmodels/crafting/ConfuseStoneWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/ItemInfo.h"
#include "client/rules/Compounding.h"
#include "client/rules/ConfuseSteps.h"
#include "client/rules/Equipping.h"
#include "gfx/TextStyle.h"
#include "protocol/ConfuseStoneRequests.h"
#include "ui/DrawingArea.h"

#include <algorithm>
#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kGunType = 11; // item type 11: guns (Arquebus, Bazooka, ...)
        constexpr int kAccessorySlot = 6; // equip slot 6: accessories/special items (rings, horn hats, job manteaus)
        constexpr int kManteauSubType = 142; // item sub type 142: job manteau (Killer Manteau, ...)
        constexpr int kMostCount = 999; // largest stone count shown
        constexpr int kMostBagCells = 50; // bag places 1..50
        // (x, y) of the 16 marks round the wheel, clockwise from the top
        constexpr std::array<core::Point, ConfuseStoneWindow::kSteps> kStonePositions{
            {{127, 66},
             {158, 80},
             {180, 102},
             {191, 132},
             {191, 164},
             {180, 195},
             {158, 216},
             {127, 230},
             {95, 230},
             {64, 216},
             {42, 195},
             {31, 164},
             {31, 132},
             {42, 102},
             {64, 80},
             {95, 66}}
        };

        /// @brief Loads ConfuseStoneWindow.xml and returns the window.
        /// @param services Game services providing the layout and load context.
        /// @param screen Screen the layout is loaded into.
        /// @return The ConfuseStone window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ConfuseStoneWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ConfuseStone");
        }

    }

    ConfuseStoneWindow::ConfuseStoneWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        BagMarks& marks,
        PlayerMessages& messages,
        ItemInfoPopup& info,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_info(info),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_equipCell(screen.get<ui::DrawingArea>("ConfuseStoneEquip")),
          m_stoneCell(screen.get<ui::DrawingArea>("ConfuseStoneStone")),
          m_confirm(screen.get<ui::ImageButton>("ConfuseStoneConfirm")),
          m_cellSpot(screen.get<ui::Spot>("ConfuseStoneCell")),
          m_countSpot(screen.get<ui::Spot>("ConfuseStoneCount")) {
        screen.get<ui::DrawingArea>("ConfuseStoneBelow").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintBelow(canvas, at); });
        screen.get<ui::DrawingArea>("ConfuseStoneEquip").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            paintCell(canvas, at, m_shownEquip, 0);
        });
        screen.get<ui::DrawingArea>("ConfuseStoneStone").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            paintCell(canvas, at, m_stone, m_count);
        });
        screen.on("onConfuseStoneConfirm", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onConfuseStoneClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onConfuseStoneTake", [this](const ui::EventArgs&) { clearEquip(); });
    }

    void ConfuseStoneWindow::release(int slot) {
        if (slot >= 1 && slot <= BagMarks::kSlots && m_marks.useMark(slot) == kMark) {
            m_marks.setUseMark(slot, 0);
        }
    }

    void ConfuseStoneWindow::clearEquip() {
        release(m_equipSlot);
        m_strength = 0;
        m_equipSlot = 0;
        m_shownEquip = protocol::ItemInstance{};
        m_hooks.marksChanged();
    }

    void ConfuseStoneWindow::clearStone() {
        m_stone = protocol::ItemInstance{};
        m_stoneLevel = 0;
    }

    void ConfuseStoneWindow::open(int stoneSlot) {
        if (!m_window.isVisible()) {
            const core::Rect items = m_hooks.itemWindowBounds();
            const core::Rect area = m_window.bounds();
            const int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
            m_window.setPosition({left, items.y});
            m_stoneSlot = 0;
            m_askConfirm = true;
            clearEquip();
            clearStone();
            m_window.setVisible(true);
            if (m_screen.context().bringToFront) {
                m_screen.context().bringToFront(m_window.id());
            }
        }
        protocol::ItemInstance crystal;
        crystal.itemId = m_inventory.bagItem(stoneSlot).itemId;
        m_stone = crystal;
        m_stoneLevel = confuseCrystalLevel(crystal.itemId);
        m_stoneSlot = stoneSlot;
    }

    void ConfuseStoneWindow::close() {
        m_stoneSlot = 0;
        m_askConfirm = true;
        clearEquip();
        clearStone();
        m_window.setVisible(false);
    }

    void ConfuseStoneWindow::putEquip(int bagSlot) {
        if (bagSlot < 1 || bagSlot > Inventory::kBagSlots || m_inventory.bagItem(bagSlot).empty()) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        const auto record = m_services.items.find(item.itemId);
        if (!record) {
            return;
        }
        const std::string cannot = "This equipment cannot be strengthened";
        if (m_marks.useMark(bagSlot) != 0) {
            m_messages.show("Unable to operate when the item is locked", 1000); // message duration in ms
            return;
        }
        if (!m_marks.isAvailable(bagSlot) && isMallEquipment(*record)) {
            m_messages.show(cannot, 1000); // message duration in ms
            return;
        }
        if (compoundRefusal(m_services.items, record->id) != 0) {
            m_messages.show(cannot, 1000); // message duration in ms
            return;
        }
        if (!isEquipment(*record)) {
            m_messages.show("Can only be used for equipment", 1000); // message duration in ms
            return;
        }
        if (record->type == kGunType) {
            m_messages.show("Gun-related weapons cannot be put in here", 1000); // message duration in ms
            return;
        }
        if (record->equipSlot == kAccessorySlot && record->subType != kManteauSubType) {
            m_messages.show(cannot, 1000); // message duration in ms
            return;
        }
        int step = 0;
        if (item.insertItemId != 0) {
            step = confuseStoneStep(item.insertItemId);
            if (step == 0) {
                m_messages.show("Mounted", 1000); // message duration in ms
                return;
            }
        }
        m_strength = step;
        release(m_equipSlot);
        m_equipSlot = bagSlot;
        m_shownEquip = item;
        m_shownEquip.count = 0;
        if (m_marks.useMark(bagSlot) == 0) {
            m_marks.setUseMark(bagSlot, kMark);
        }
        m_hooks.marksChanged();
    }

    void ConfuseStoneWindow::putStone(int bagSlot) {
        if (bagSlot < 1 || bagSlot > Inventory::kBagSlots || confuseCrystalLevel(m_inventory.bagItem(bagSlot).itemId) == 0) {
            return;
        }
        protocol::ItemInstance crystal;
        crystal.itemId = m_inventory.bagItem(bagSlot).itemId;
        m_stone = crystal;
        m_stoneLevel = confuseCrystalLevel(crystal.itemId);
        m_stoneSlot = bagSlot;
    }

    void ConfuseStoneWindow::confirm() {
        if (m_shownEquip.empty()) {
            return;
        }
        if (m_strength >= kSteps) {
            m_messages.show("Reach the upper limit", 1000); // message duration in ms
            return;
        }
        if (m_askConfirm) {
            const auto crystal = m_services.items.find(m_stoneSlot > 0 ? m_inventory.bagItem(m_stoneSlot).itemId : m_stone.itemId);
            m_hooks.ask("Confirm " + (crystal ? crystal->name : std::string()) + " Want to strengthen?", [this] { send(); }, [] {});
        } else {
            send();
        }
    }

    void ConfuseStoneWindow::send() {
        if (m_hooks.fighting()) {
            m_messages.show("You can't use it during the battle", 1000); // message duration in ms
            return;
        }
        if (m_equipSlot == 0 || !m_marks.isAvailable(m_equipSlot) || m_shownEquip.empty()) {
            return;
        }
        const auto record = m_services.items.find(m_shownEquip.itemId);
        if (!record) {
            return;
        }
        if (isMallEquipment(*record)) {
            m_messages.show("Can only be used for equipment", 1000); // message duration in ms
            return;
        }
        if (compoundRefusal(m_services.items, record->id) != 0) {
            m_messages.show("This equipment cannot be strengthened", 1000); // message duration in ms
            return;
        }
        if (m_marks.useMark(m_equipSlot) != kMark) {
            m_messages.show("Unable to operate when the item is locked", 1000); // message duration in ms
            return;
        }
        if (!isEquipment(*record)) {
            return;
        }
        if (record->type == kGunType) {
            m_messages.show("Gun-related weapons cannot be put in here", 1000); // message duration in ms
            return;
        }
        if (record->equipSlot == kAccessorySlot && record->subType != kManteauSubType) {
            return;
        }
        const protocol::ItemInstance& piece = m_inventory.bagItem(m_equipSlot);
        int step = 0;
        if (piece.insertItemId != 0) {
            step = confuseStoneStep(piece.insertItemId);
            if (step == 0) {
                return;
            }
        }
        if (step + 1 > kSteps) {
            m_messages.show("Reach the upper limit", 1000); // message duration in ms
            return;
        }
        if (m_stoneSlot > 0) {
            const auto holds = [this](int slot) {
                return confuseCrystalLevel(m_inventory.bagItem(slot).itemId) == m_stoneLevel && m_marks.useMark(slot) == 0 && m_marks.isAvailable(slot);
            };
            if (!(confuseCrystalLevel(m_inventory.bagItem(m_stoneSlot).itemId) != 0 && holds(m_stoneSlot))) {
                int found = 0;
                for (int slot = 1; slot <= kMostBagCells && found == 0; ++slot) {
                    if (holds(slot)) {
                        found = slot;
                    }
                }
                if (found == 0) {
                    m_stoneSlot = 0;
                    clearStone();
                    return;
                }
                m_stoneSlot = found;
            }
            m_hooks.send(protocol::confuseStoneRequest(m_equipSlot, m_stoneSlot));
            m_askConfirm = false;
        }
    }

    void ConfuseStoneWindow::result(int step, int bagSlot) {
        if (m_equipSlot != bagSlot) {
            return;
        }
        if (step > m_strength) {
            m_services.playSound("sound/SEB0300.wav"); // enhancement success sound
        } else {
            m_messages.show("Enchancd failed ", 1000); // message duration in ms
        }
        m_strength = step;
        m_shownEquip.insertItemId = m_inventory.bagItem(bagSlot).insertItemId;
    }

    int ConfuseStoneWindow::crystalCount() const {
        int count = 0;
        for (int slot = 1; slot <= kMostBagCells; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (!item.empty() && confuseCrystalLevel(item.itemId) == m_stoneLevel && m_marks.useMark(slot) == 0 && m_marks.isAvailable(slot)) {
                count += item.count;
            }
        }
        return count;
    }

    void ConfuseStoneWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        m_count = 0;
        if (m_stoneLevel != 0) {
            m_count = crystalCount();
            if (m_count == 0) {
                m_stoneSlot = 0;
                clearStone();
            }
        }
        m_confirm.setVisible(m_count != 0);
        m_count = std::min(m_count, kMostCount);
        const core::Point origin = m_window.bounds().position();
        for (const auto& [cell, item] :
             {std::pair<ui::Element&, const protocol::ItemInstance&>{m_equipCell, m_shownEquip},
              std::pair<ui::Element&, const protocol::ItemInstance&>{m_stoneCell, m_stone}}) {
            if (item.empty() || m_screen.hoveredId() != cell.id()) {
                continue;
            }
            const core::Point at = origin + cell.bounds().position();
            protocol::ItemInstance shown = item;
            shown.count = std::max(shown.count, 1);
            m_info.show(shown, {at.x, at.y, m_cellSpot.size().width, m_cellSpot.size().height});
        }
    }

    void ConfuseStoneWindow::paintBelow(gfx::Canvas& canvas, core::Point at) {
        const auto big = m_services.pictures.find("PotentialFire1");
        const auto small = m_services.pictures.find("PotentialFire2");
        const auto step = [this](int index) {
            int& frame = m_frames[static_cast<std::size_t>(index)];
            if (++frame >= 12) { // 12 draws per flame cycle (4 frames, each held 3 draws)
                frame = 0;
            }
        };
        for (int level = 1; level <= m_strength && level <= kSteps; ++level) {
            if (big) {
                const core::Point where = at + kStonePositions[static_cast<std::size_t>(level - 1)];
                // flame offset (1, -10); 3 draws per frame; frame strip step 26 and frame size 16 x 26
                canvas.drawPicture(*big, {where.x + 1, where.y - 10}, core::Rect{0, m_frames[static_cast<std::size_t>(level)] / 3 * 26, 16, 26});
                step(level);
            }
        }
        const int next = m_strength + 1;
        if (!m_shownEquip.empty() && next <= kSteps && small) {
            const core::Point where = at + kStonePositions[static_cast<std::size_t>(next - 1)];
            // small flame offset x 5; 3 draws per frame; frame strip step 14 and frame size 8 x 14
            canvas.drawPicture(*small, {where.x + 5, where.y}, core::Rect{0, m_frames[static_cast<std::size_t>(next)] / 3 * 14, 8, 14});
            step(next);
        }
        if (m_count == 0) {
            if (const auto confirm = m_services.pictures.find("icon_Confirm_1")) {
                canvas.drawPicture(*confirm, at + m_confirm.bounds().position());
            }
        }
    }

    void ConfuseStoneWindow::paintCell(gfx::Canvas& canvas, core::Point at, const protocol::ItemInstance& item, int count) {
        if (item.empty()) {
            return;
        }
        const ui::IconCell cell = itemIconCell(m_services, item, false);
        if (cell.picture.isValid()) {
            canvas.drawPicture(cell.picture, at, core::Rect{0, 0, m_cellSpot.size().width, m_cellSpot.size().height});
        }
        if (count > 0) {
            canvas.drawText(at + m_countSpot.at(), std::to_string(count), {m_countSpot.color(), gfx::TextEffect::Plain, std::nullopt}, m_cellSpot.size().width);
        }
    }

}
