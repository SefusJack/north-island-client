#include "client/viewmodels/travel/CapsuleWindow.h"

#include "client/rendering/painters/VehicleBoxPainter.h"
#include "client/data/VehicleKinds.h"
#include "protocol/VehicleRequests.h"
#include "ui/DrawingArea.h"

namespace client {

    namespace {

        constexpr int kScreenWidth = 800; // client screen width, px
        constexpr int kCapsuleMark = 26; // bag use-mark 26: the capsule item that is open in this window
        constexpr int kVehicleType = 39; // item type 39: vehicles (Airship, Boat, Cabriolet, ...)
        const char* const kLocked = "Unable to operate when the item is locked";

        /// @brief Loads CapsuleWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The "CapsuleWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("CapsuleWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("CapsuleWindow");
        }

    }

    CapsuleWindow::CapsuleWindow(
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
          m_image(screen.get<ui::DrawingArea>("CapsuleImage")) {
        screen.get<ui::DrawingArea>("CapsuleImage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onCapsuleOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onCapsuleClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onCapsulePress", [this](const ui::EventArgs&) { m_pressed = true; });
        screen.on("onCapsuleTake", [this](const ui::EventArgs&) {
            m_pressed = true;
            take();
        });
    }

    void CapsuleWindow::release(int slot) {
        if (slot >= 1 && slot <= BagMarks::kSlots && m_marks.useMark(slot) == kCapsuleMark) {
            m_marks.setUseMark(slot, 0);
            m_hooks.marksChanged();
        }
    }

    void CapsuleWindow::open(int bagSlot) {
        if (m_capsuleSlot >= 1 && m_capsuleSlot <= BagMarks::kSlots) {
            m_marks.setUseMark(m_capsuleSlot, 0);
        }
        m_capsuleSlot = bagSlot;
        m_marks.setUseMark(m_capsuleSlot, kCapsuleMark);
        m_hooks.marksChanged();
        const core::Rect items = m_hooks.itemWindowBounds();
        const core::Rect area = m_window.bounds();
        int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
        if (left + area.width >= kScreenWidth) {
            const int spaceRight = kScreenWidth - left;
            const int shift = area.width - spaceRight + 20; // 20 px margin from the screen edge
            left -= shift;
            m_hooks.moveItemWindow(items.x - shift);
        }
        m_window.setBounds({left, items.y, area.width, area.height});
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void CapsuleWindow::clearImage() { m_shown = protocol::ItemInstance{}; }

    void CapsuleWindow::close() {
        m_window.setVisible(false);
        clearImage();
        release(m_capsuleSlot);
        release(m_itemSlot);
    }

    void CapsuleWindow::take() {
        clearImage();
        release(m_itemSlot);
        m_itemSlot = 0;
    }

    void CapsuleWindow::put(int bagSlot) {
        if (bagSlot < 1 || bagSlot > Inventory::kBagSlots || m_inventory.bagItem(bagSlot).empty()) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        const auto record = m_services.items.find(item.itemId);
        if (!record || record->type != kVehicleType) {
            m_messages.show("Only vehicles can be put in the garage", 2000); // message display time in ms
            return;
        }
        if (const auto notice = m_marks.lockNotice(bagSlot, 0)) {
            m_messages.show(notice->text, notice->ms);
            return;
        }
        if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show(kLocked, 1000); // message display time in ms
            return;
        }
        if (m_marks.vehicleState(bagSlot) != 0) {
            return;
        }
        if (m_hooks.fighting()) {
            m_messages.show("Fighting, cannot repair the equipment", 1000); // message display time in ms
            return;
        }
        if (VehicleKinds::isCapsule(item.itemId) || record->aliasItemId == 0) {
            m_messages.show("Unable to lay the item", 1000); // message display time in ms
            return;
        }
        release(m_itemSlot);
        m_itemSlot = bagSlot;
        m_marks.setUseMark(bagSlot, kCapsuleMark);
        m_hooks.marksChanged();
        m_shown = item;
    }

    void CapsuleWindow::confirm() {
        if (m_capsuleSlot == 0 || m_itemSlot == 0) {
            return;
        }
        if (m_hooks.fighting()) {
            m_messages.show("Cannot use during battle", 1000); // message display time in ms
            return;
        }
        bool free = false;
        for (int slot = 1; slot <= Inventory::kBagSlots && !free; ++slot) {
            free = m_inventory.bagItem(slot).empty();
        }
        if (!free) {
            m_messages.show("Need an available slot in your bag to use", 1000); // message display time in ms
            return;
        }
        m_hooks.send(protocol::capsuleRequest(m_capsuleSlot, m_itemSlot));
        clearImage();
        release(m_capsuleSlot);
        release(m_itemSlot);
        m_itemSlot = 0;
        m_window.setVisible(false);
    }

    void CapsuleWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (m_shown.empty()) {
            return;
        }
        protocol::ItemInstance shown = m_shown;
        shown.usedAmount = 0;
        shown.extraAmount = 0;
        VehicleBoxPainter::paint(m_services, canvas, at, shown);
    }

    void CapsuleWindow::update() {
        if (m_screen.hoveredId() != m_image.id()) {
            m_pressed = false;
        }
        if (!m_window.isVisible() || m_shown.empty() || m_pressed || m_screen.hoveredId() != m_image.id()) {
            return;
        }
        const core::Point at = m_window.bounds().position() + m_image.bounds().position();
        m_info.show(m_shown, {at.x, at.y, VehicleBoxPainter::kSide, VehicleBoxPainter::kSide});
    }

}
