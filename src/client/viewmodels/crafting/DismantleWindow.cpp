#include "client/viewmodels/crafting/DismantleWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/Stalling.h"
#include "protocol/DismantleRequests.h"
#include "gfx/TextStyle.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kDevilSparRank = 25; // stones above rank 25 are Devil's Spars, which the small tear plier cannot remove

        /// @brief Loads DismantleWindow.xml and returns the window.
        /// @param services Game services providing the layout and load context.
        /// @param screen Screen the layout is loaded into.
        /// @return The Dismantle window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("DismantleWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Dismantle");
        }

    }

    DismantleWindow::DismantleWindow(
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
          m_image(screen.get<ui::DrawingArea>("DismantleImage")),
          m_countSpot(screen.get<ui::Spot>("DismantleCount")) {
        screen.get<ui::DrawingArea>("DismantleImage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onDismantleOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onDismantleCancel", [this](const ui::EventArgs&) { close(); });
        screen.on("onDismantleTake", [this](const ui::EventArgs&) { clearSelection(); });
    }

    void DismantleWindow::release(int slot) {
        if (slot >= 1 && slot <= BagMarks::kSlots && m_marks.useMark(slot) == kMark) {
            m_marks.setUseMark(slot, 0);
        }
    }

    void DismantleWindow::open(int pliersSlot, int mode) {
        if (m_pliersSlot >= 1 && m_pliersSlot <= BagMarks::kSlots) {
            m_marks.setUseMark(m_pliersSlot, 0);
        }
        m_pliersSlot = pliersSlot;
        m_marks.setUseMark(pliersSlot, kMark);
        const core::Rect items = m_hooks.itemWindowBounds();
        const core::Rect area = m_window.bounds();
        const int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
        m_window.setPosition({left, items.y});
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_mode = mode;
        m_hooks.marksChanged();
    }

    void DismantleWindow::clearSelection() {
        m_shown = protocol::ItemInstance{};
        release(m_pieceSlot);
        m_pieceSlot = 0;
        m_hooks.marksChanged();
    }

    void DismantleWindow::close() {
        m_shown = protocol::ItemInstance{};
        release(m_pieceSlot);
        release(m_pliersSlot);
        m_pieceSlot = 0;
        m_pliersSlot = 0;
        m_mode = 0;
        m_window.setVisible(false);
        m_hooks.marksChanged();
    }

    void DismantleWindow::put(int bagSlot) {
        if (bagSlot < 1 || bagSlot > Inventory::kBagSlots || m_inventory.bagItem(bagSlot).empty()) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        if (isNonTransferable(item.insertItemId)) {
            m_messages.show("The parts on this item cannot be moved", 2000); // message duration in ms
            return;
        }
        if (m_hooks.fighting()) {
            m_messages.show("Fighting, cannot repair the equipment", 1000); // message duration in ms
            return;
        }
        if (!m_services.items.find(item.itemId)) {
            m_messages.show("Unable to lay the item", 1000); // message duration in ms
            return;
        }
        if (const auto notice = m_marks.lockNotice(bagSlot, 0)) {
            m_messages.show(notice->text, notice->ms);
            return;
        }
        if (!m_marks.isAvailable(bagSlot)) {
            m_messages.show("Unable to lay the item", 1000); // message duration in ms
            return;
        }
        if (item.insertItemId == 0) {
            m_messages.show("Disassembly not needed", 2000); // message duration in ms
            return;
        }
        if (m_pieceSlot >= 1 && m_pieceSlot <= Inventory::kBagSlots) {
            release(m_pieceSlot);
            m_shown = protocol::ItemInstance{};
        }
        const auto stone = m_services.items.find(item.insertItemId);
        if (m_mode == kSmallTearPlier && stone && stone->rank > kDevilSparRank) {
            m_messages.show("Devil's Spar can't be removed", 2000); // message duration in ms
            return;
        }
        m_pieceSlot = bagSlot;
        m_shown = item;
        m_marks.setUseMark(bagSlot, kMark);
        m_hooks.marksChanged();
    }

    void DismantleWindow::confirm() {
        if (m_pieceSlot == 0) {
            return;
        }
        if (m_hooks.fighting()) {
            m_messages.show("Fighting, cannot repair the equipment", 1000); // message duration in ms
            return;
        }
        const int stoneId = m_inventory.bagItem(m_pieceSlot).insertItemId;
        if (stoneId == 0) {
            m_messages.show("Equipment without socketing", 2000); // message duration in ms
            return;
        }
        const auto stone = m_services.items.find(stoneId);
        if (m_mode == kSmallTearPlier && stone && stone->rank > kDevilSparRank) {
            m_messages.show("Devil's Spar can't be removed", 2000); // message duration in ms
            return;
        }
        if (m_inventory.placeFor(stoneId, 1) != 0) {
            m_services.connection.send(protocol::dismantleRequest(m_pieceSlot, m_pliersSlot, m_mode));
        } else {
            m_messages.show("You should unfill at least 2 items space", 2000); // message duration in ms
        }
        m_services.playSound("sound/wav0152.wav"); // sound played when Ok is pressed
        clearSelection();
    }

    void DismantleWindow::result(int bagSlot) {
        if (bagSlot < 1 || bagSlot > Inventory::kBagSlots) {
            return;
        }
        if (const auto record = m_services.items.find(m_inventory.bagItem(bagSlot).itemId)) {
            m_messages.show(record->name + " Disassembled", 1000); // message duration in ms
        }
    }

    void DismantleWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (m_shown.empty()) {
            return;
        }
        const ui::IconCell cell = itemIconCell(m_services, m_shown, false);
        if (cell.picture.isValid()) {
            canvas.drawPicture(cell.picture, at, core::Rect{0, 0, m_image.bounds().width, m_image.bounds().height});
        }
        canvas.drawText(
            at + m_countSpot.at(),
            std::to_string(m_shown.count),
            {m_countSpot.color(), gfx::TextEffect::Plain, std::nullopt},
            m_image.bounds().width
        );
    }

    void DismantleWindow::update() {
        if (!m_window.isVisible() || m_shown.empty() || m_screen.hoveredId() != m_image.id()) {
            return;
        }
        const core::Point at = m_window.bounds().position() + m_image.bounds().position();
        m_info.show(m_shown, {at.x, at.y, m_image.bounds().width, m_image.bounds().height});
    }

}
