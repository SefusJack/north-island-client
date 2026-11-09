#include "client/viewmodels/crafting/PackingWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/data/PackingItems.h"
#include "gfx/TextStyle.h"
#include "protocol/PackingRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kFirstEquipSlot = 1; // equipment slots 1..6 are the character's wearable slots
        constexpr int kLastEquipSlot = 6;

        /// @brief Loads PackingWindow.xml into the screen and returns its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PackingWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Packing");
        }

    }

    PackingWindow::PackingWindow(
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
          m_image(screen.get<ui::DrawingArea>("PackingImage")),
          m_countSpot(screen.get<ui::Spot>("PackingCount")) {
        screen.get<ui::DrawingArea>("PackingImage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onPackingOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onPackingClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onPackingTake", [this](const ui::EventArgs&) { clearImage(); });
    }

    void PackingWindow::release(int slot) {
        if (slot >= 1 && slot <= BagMarks::kSlots && m_marks.useMark(slot) == kMark) {
            m_marks.setUseMark(slot, 0);
        }
    }

    void PackingWindow::clearImage() {
        if (m_requestSent) {
            return;
        }
        m_shown = protocol::ItemInstance{};
        release(m_bagSlot);
        m_bagSlot = 0;
        m_hooks.marksChanged();
    }

    void PackingWindow::reset() {
        release(m_boxSlot);
        m_boxSlot = 0;
        clearImage();
        m_requestSent = false;
        m_hooks.marksChanged();
    }

    void PackingWindow::close() {
        if (!m_requestSent) {
            reset();
        }
        m_window.setVisible(false);
    }

    void PackingWindow::open(int boxSlot) {
        if (m_requestSent || boxSlot < 1 || boxSlot > BagMarks::kSlots) {
            return;
        }
        const protocol::ItemInstance& box = m_inventory.bagItem(boxSlot);
        if (box.itemId == kPackedItem) { // item 34194: Present Case (filled)
            const auto packed = box.boxItemId != 0 ? m_services.items.find(box.boxItemId) : std::nullopt;
            if (packed && isPackableShopPicture(packed->shopPicId)) {
                m_boxSlot = boxSlot;
                m_hooks.ask("Items can't be traded if you open the box\rSure? ", [this] { m_hooks.send(protocol::openPackedBoxRequest(m_boxSlot)); });
            }
            return;
        }
        if (box.itemId != kEmptyItem) { // item 34200: Present Case (empty)
            close();
            return;
        }
        reset();
        m_boxSlot = boxSlot;
        m_marks.setUseMark(boxSlot, kMark);
        const core::Rect items = m_hooks.itemWindowBounds();
        const core::Rect area = m_window.bounds();
        const int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
        m_window.setPosition({left, items.y});
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_hooks.marksChanged();
    }

    void PackingWindow::put(int bagSlot) {
        if (m_requestSent) {
            return;
        }
        clearImage();
        if (bagSlot < 1 || bagSlot > Inventory::kBagSlots || m_inventory.bagItem(bagSlot).empty()) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        const auto record = m_services.items.find(item.itemId);
        if (!record) {
            return;
        }
        const bool packable = isPackableShopPicture(record->shopPicId) && record->equipSlot >= kFirstEquipSlot && record->equipSlot <= kLastEquipSlot;
        if (!packable) {
            m_messages.show("The equipment which is not in item mall can't be used", 3000); // message display time in ms
        } else if (const auto notice = m_marks.lockNotice(bagSlot, BagMarks::kCompound)) {
            m_messages.show(notice->text, notice->ms);
        } else if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show("Unable to operate when the item is locked", 3000); // message display time in ms
        } else if (item.lookItemId != 0) {
            m_messages.show("Pose Equipment can't be used", 3000); // message display time in ms
        } else {
            m_bagSlot = bagSlot;
            m_shown = item;
            m_marks.setUseMark(bagSlot, kMark);
            m_hooks.marksChanged();
        }
    }

    void PackingWindow::confirm() {
        if (m_requestSent || m_boxSlot == 0 || m_bagSlot == 0) {
            return;
        }
        const auto box = m_services.items.find(kPackedItem);
        m_hooks.ask("Use " + (box ? box->name : std::string()) + "?", [this] { send(); });
    }

    void PackingWindow::send() {
        if (m_requestSent) {
            return;
        }
        m_hooks.send(protocol::packingRequest(m_boxSlot, m_bagSlot));
        m_requestSent = true;
        close();
    }

    void PackingWindow::result(int code) {
        if (code == 1) { // result code 1: packing succeeded
            m_messages.show("Packing Finished", 3000); // message display time in ms
            reset();
        }
    }

    void PackingWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        if (m_inventory.bagItem(m_boxSlot).itemId != kEmptyItem) {
            close();
            return;
        }
        if (m_bagSlot != 0 && m_inventory.bagItem(m_bagSlot).empty()) {
            clearImage();
        }
        if (m_shown.empty() || m_screen.hoveredId() != m_image.id()) {
            return;
        }
        const core::Point at = m_window.bounds().position() + m_image.bounds().position();
        m_info.show(m_shown, {at.x, at.y, m_image.bounds().width, m_image.bounds().height});
    }

    void PackingWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (m_shown.empty()) {
            return;
        }
        const ui::IconCell cell = itemIconCell(m_services, m_shown, false);
        if (cell.picture.isValid()) {
            canvas.drawPicture(cell.picture, at, core::Rect{0, 0, m_image.bounds().width, m_image.bounds().height});
        }
        if (m_shown.count > 0) {
            canvas.drawText(
                at + m_countSpot.at(),
                std::to_string(m_shown.count),
                {m_countSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                m_image.bounds().width
            );
        }
    }

}
