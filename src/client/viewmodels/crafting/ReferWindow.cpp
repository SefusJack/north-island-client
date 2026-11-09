#include "client/viewmodels/crafting/ReferWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "core/ByteReader.h"
#include "gfx/TextStyle.h"
#include "protocol/ReferRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kVehicleType = 39; // item type 39: vehicles (Airship, Boat, Cabriolet, Hot-air Balloon, ...)
        constexpr int kTextWidth = 160; // text clip width, px
        constexpr int kMaterials = 5; // most materials listed in the result
        constexpr int kUnsearchableFlags = 12; // item flag bits 4 and 8: items carrying either cannot be searched

        /// @brief Loads ReferWindow.xml into the screen and returns its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ReferWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ReferWindow");
        }

    }

    ReferWindow::ReferWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        const BagMarks& marks,
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
          m_image(screen.get<ui::DrawingArea>("ReferImage")),
          m_nameSpot(screen.get<ui::Spot>("ReferName")),
          m_firstNumberSpot(screen.get<ui::Spot>("ReferFirstNumber")),
          m_secondNumberSpot(screen.get<ui::Spot>("ReferSecondNumber")),
          m_materialPictureSpot(screen.get<ui::Spot>("ReferMaterialPicture")),
          m_materialNameSpot(screen.get<ui::Spot>("ReferMaterialName")) {
        screen.get<ui::DrawingArea>("ReferPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onReferQuery", [this](const ui::EventArgs&) { ask(); });
        screen.on("onReferClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onReferTake", [this](const ui::EventArgs&) { clearItem(); });
    }

    void ReferWindow::clearMaterial() {
        m_materials = {};
        m_itemId = 0;
        m_name.clear();
        m_first = -1;
        m_second = -1;
        m_shown = protocol::ItemInstance{};
    }

    void ReferWindow::clearItem() {
        clearMaterial();
        m_slot = 0;
    }

    void ReferWindow::close() {
        clearItem();
        m_window.setVisible(false);
    }

    void ReferWindow::open() {
        if (m_window.isVisible()) {
            return;
        }
        m_slot = 0;
        clearMaterial();
        const core::Rect items = m_hooks.itemWindowBounds();
        const core::Rect area = m_window.bounds();
        const int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
        m_window.setPosition({left, items.y});
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void ReferWindow::put(int bagSlot) {
        if (m_hooks.inFight()) {
            m_messages.show("Fighting, cannot repair the equipment", 1000); // message display time in ms
            return;
        }
        if (bagSlot < 1 || bagSlot > Inventory::kBagSlots || m_inventory.bagItem(bagSlot).empty()) {
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        const auto record = m_services.items.find(item.itemId);
        if (!record) {
            return;
        }
        if (record->rank <= 0 || record->requiredLevel <= 0 ||
            (record->flags & kUnsearchableFlags) != 0) { // items with no rank or no level requirement can't be searched
            m_messages.show("The item can't be searched", 2000); // message display time in ms
            return;
        }
        if (record->type == kVehicleType || (record->cellWidth > 1 && record->cellHeight > 1)) { // vehicles and items larger than 1 x 1 cells are refused
            m_messages.show("Unable to lay the item", 1000); // message display time in ms
            return;
        }
        if (m_slot >= 1 && m_slot <= Inventory::kBagSlots) {
            clearMaterial();
        }
        m_slot = bagSlot;
        m_shown = item;
        m_shown.count = 0;
        m_itemId = item.itemId;
        m_name = itemIconCell(m_services, item, true).hint;
    }

    int ReferWindow::glassSlot() const {
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            if (m_inventory.bagItem(slot).itemId == kMagnifyingGlass && // item 34179: Magnifying Glass
                m_marks.isAvailable(slot) && !m_marks.lockNotice(slot, 0)) {
                return slot;
            }
        }
        return 0;
    }

    void ReferWindow::ask() {
        if (!m_window.isVisible() || m_first != -1 || m_itemId == 0) {
            return;
        }
        m_glass = glassSlot();
        const auto glass = m_services.items.find(kMagnifyingGlass);
        const std::string name = glass ? glass->name : std::string();
        if (m_glass == 0) {
            m_messages.show(name + "Not enough , can't search", 1000); // message display time in ms
        } else {
            m_hooks.ask("Confirm " + name + " Search for file?", [this] { confirm(); });
        }
    }

    void ReferWindow::confirm() { m_hooks.send(protocol::referSearchRequest(m_itemId, m_slot, m_glass)); }

    void ReferWindow::result(const core::Bytes& payload) {
        if (!m_window.isVisible()) {
            return;
        }
        core::ByteReader reader(payload, 2); // payload offset past the packet header
        const int code = reader.u8();
        if (code == 1) { // result code 1: the two numbers and the materials
            m_materials = {};
            m_first = reader.u8();
            m_second = reader.u8();
            for (std::size_t place = 0; place < m_materials.size() && !reader.failed(); ++place) {
                m_materials[place] = ReferMaterials::of(reader.u8());
            }
            m_messages.show("Inquiry successfully", 2000); // message display time in ms
        } else if (code == 2) { // result code 2: the item cannot be searched
            m_messages.show("The item can't be searched", 2000); // message display time in ms
        }
    }

    void ReferWindow::update() {
        if (m_itemId != 0 && m_first == -1 && m_slot >= 1 && m_inventory.bagItem(m_slot).itemId != m_itemId) {
            clearItem();
        }
        if (!m_window.isVisible() || m_shown.empty() || m_screen.hoveredId() != m_image.id()) {
            return;
        }
        const core::Point at = m_window.bounds().position() + m_image.bounds().position();
        m_info.show(m_shown, {at.x, at.y, m_image.bounds().width, m_image.bounds().height});
    }

    void ReferWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const gfx::TextStyle style{m_nameSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        canvas.drawText(at + m_nameSpot.at(), m_name, style, kTextWidth, 15); // text line height
        if (const auto digits = m_services.pictures.find("Num_White3_1")) {
            const gfx::Picture& data = m_services.pictures.picture(*digits);
            const int height = data.height() / 10; // the digit sheet holds the 10 digits stacked
            const auto number = [&](int value, core::Point place) {
                const std::string text = std::to_string(value);
                for (std::size_t index = 0; index < text.size(); ++index) {
                    canvas.drawPicture(
                        *digits,
                        {at.x + place.x + static_cast<int>(index) * data.width(), at.y + place.y},
                        core::Rect{0, (text[index] - '0') * height, data.width(), height}
                    );
                }
            };
            if (m_first != -1) {
                number(m_first, m_firstNumberSpot.at());
            }
            if (m_second != -1) {
                number(m_second, m_secondNumberSpot.at());
            }
        }
        for (int place = 0; place < kMaterials; ++place) {
            const auto& material = m_materials[static_cast<std::size_t>(place)];
            if (!material) {
                break;
            }
            if (const auto picture = m_services.pictures.find(material->picture)) {
                canvas.drawPicture(
                    *picture,
                    {at.x + m_materialPictureSpot.at().x, at.y + m_materialPictureSpot.at().y + place * m_materialPictureSpot.pitch().y}
                );
            }
            canvas.drawText(
                {at.x + m_materialNameSpot.at().x, at.y + m_materialNameSpot.at().y + place * m_materialNameSpot.pitch().y},
                material->name,
                style,
                kTextWidth,
                15
            ); // text line height
        }
        if (!m_shown.empty()) {
            const ui::IconCell cell = itemIconCell(m_services, m_shown, false);
            if (cell.picture.isValid()) {
                canvas.drawPicture(
                    cell.picture,
                    {at.x + m_image.bounds().x, at.y + m_image.bounds().y},
                    core::Rect{0, 0, m_image.bounds().width, m_image.bounds().height}
                );
            }
        }
    }

}
