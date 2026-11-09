#include "client/viewmodels/travel/GpsWindow.h"

#include "gfx/TextStyle.h"
#include "protocol/GpsRequests.h"

#include <algorithm>
#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kNormalGps = 34006; // Normal GPS
        constexpr int kAdvanceGps = 34195; // Advance GPS
        constexpr int kSmartGps = 34373; // Smart GPS
        constexpr int kMemoryCrystal = 37257; // Memory Crystal (upgrades a GPS)
        constexpr int kBagSlots = Inventory::kBagSlots;
        constexpr int kLockedMs = 2000; // message display time, ms
        const core::Color kTextColor = core::Color::fromRgb565(65504); // RGB565 0xFFE0: yellow text
        constexpr const char* kColon = "\xA1G";

        /// @brief Loads GpsWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The "Gps" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("GpsWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Gps");
        }

        /// @brief Gives the plain yellow text style of the form.
        /// @return The text style.
        gfx::TextStyle plain() { return gfx::TextStyle{kTextColor, gfx::TextEffect::Plain, std::nullopt}; }

    }

    GpsWindow::GpsWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        const BagMarks& marks,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_previous(screen.get<ui::ImageButton>("GpsPrevious")),
          m_next(screen.get<ui::ImageButton>("GpsNext")),
          m_nameSpot(screen.get<ui::Spot>("GpsName")),
          m_xSpot(screen.get<ui::Spot>("GpsX")),
          m_ySpot(screen.get<ui::Spot>("GpsY")),
          m_emptySpot(screen.get<ui::Spot>("GpsEmpty")) {
        screen.get<ui::DrawingArea>("GpsPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onGpsSetPoint", [this](const ui::EventArgs&) { setPointClicked(); });
        screen.on("onGpsTransmit", [this](const ui::EventArgs&) { teleportClicked(); });
        screen.on("onGpsPrevious", [this](const ui::EventArgs&) { previous(); });
        screen.on("onGpsNext", [this](const ui::EventArgs&) { next(); });
        screen.on("onGpsClose", [this](const ui::EventArgs&) { hide(); });
    }

    std::string GpsWindow::itemName(int itemId) const {
        const auto record = m_services.items.find(itemId);
        return record ? record->name : std::string();
    }

    void GpsWindow::open(int bagSlot) {
        m_slot = bagSlot;
        updateButtons();
        const core::Rect items = m_hooks.itemWindowBounds();
        const core::Rect area = m_window.bounds();
        const int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
        m_window.setPosition({left, items.y});
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void GpsWindow::setMark(int point, int map, int x, int y) {
        if (point >= 1 && point <= kPoints) {
            m_points[static_cast<std::size_t>(point - 1)] = Mark{map, x, y};
        }
    }

    void GpsWindow::setPointClicked() {
        if (const auto refusal = m_hooks.refusal(false)) {
            m_messages.show(refusal->text, refusal->ms);
            if (refusal->closes) {
                hide();
            }
            return;
        }
        m_hooks.askMain(std::string("Are you sure you want to make a mark here") + "\xA1H", [this] {
            m_services.connection.send(protocol::gpsMarkRequest(m_slot, m_point));
        });
    }

    void GpsWindow::teleportClicked() {
        if (const auto refusal = m_hooks.refusal(true)) {
            m_messages.show(refusal->text, refusal->ms);
            if (refusal->closes) {
                hide();
            }
            return;
        }
        m_requestSlot = m_slot;
        m_requestPoint = m_point;
        m_hooks.startTeleport();
        m_hooks.closeItemWindow();
        hide();
    }

    core::Bytes GpsWindow::teleportRequest() const { return protocol::gpsTeleportRequest(m_requestSlot, m_requestPoint); }

    int GpsWindow::pageCount() const {
        switch (m_inventory.bagItem(m_slot).itemId) {
            case kAdvanceGps: return 2; // Advance GPS (34195): two points
            case kSmartGps: return 3; // Smart GPS (34373): three points
            default: return 1; // Normal GPS: one point
        }
    }

    void GpsWindow::previous() {
        if (pageCount() == 1) {
            m_point = 1;
        } else if (m_point - 1 >= 1) {
            --m_point;
        }
        updateButtons();
    }

    void GpsWindow::next() {
        if (pageCount() == 1) {
            m_point = 1;
        } else if (m_point - 1 <= kPoints) {
            ++m_point;
        }
        updateButtons();
    }

    void GpsWindow::updateButtons() {
        const int pages = pageCount();
        m_previous.setVisible(pages > 1);
        m_next.setVisible(pages > 1);
        m_point = std::clamp(m_point, 1, pages);
        if (pages > 1) {
            if (m_point == 1) {
                m_previous.setVisible(false);
            }
            if (m_point == pages) {
                m_next.setVisible(false);
            }
        }
    }

    bool GpsWindow::crystalUsable(int slot) const {
        if (m_hooks.fighting() || slot == 0 || slot > kBagSlots) {
            return false;
        }
        if (const auto notice = m_marks.lockNotice(slot, 0)) {
            m_messages.show(notice->text, notice->ms);
            return false;
        }
        if (!m_marks.isAvailable(slot)) {
            m_messages.show("Unable to operate when the item is locked", kLockedMs);
            return false;
        }
        return m_inventory.bagItem(slot).itemId == kMemoryCrystal;
    }

    void GpsWindow::askUpgrade(int crystalSlot) {
        m_crystalSlot = 0;
        if (!crystalUsable(crystalSlot)) {
            return;
        }
        m_crystalSlot = crystalSlot;
        m_hooks.choose(" Use " + itemName(kMemoryCrystal), {itemName(kNormalGps) + "Upgrade", itemName(kAdvanceGps) + "Upgrade"}, [this](int choice) {
            upgradeChosen(choice);
        });
    }

    void GpsWindow::upgradeChosen(int choice) {
        const int crystalSlot = m_crystalSlot;
        m_crystalSlot = 0;
        if (!crystalUsable(crystalSlot)) {
            return;
        }
        int from = 0;
        int to = 0;
        if (choice == 0) {
            from = kNormalGps;
            to = kAdvanceGps;
        } else if (choice == 1) {
            from = kAdvanceGps;
            to = kSmartGps;
        }
        if (from == 0) {
            return;
        }
        int gpsSlot = 0;
        for (int slot = 1; slot <= kBagSlots && gpsSlot == 0; ++slot) {
            if (m_inventory.bagItem(slot).itemId == from && !m_marks.lockNotice(slot, 0) && m_marks.isAvailable(slot)) {
                gpsSlot = slot;
            }
        }
        if (gpsSlot == 0) {
            m_messages.show("Nothing you have can be upgraded " + itemName(from), kLockedMs);
            return;
        }
        if (m_inventory.placeFor(to, 1) == 0) {
            m_messages.show("Not enough space", kLockedMs);
            return;
        }
        m_hooks.askTimed(
            "Will " + itemName(kMemoryCrystal) + " be equipped " + itemName(from) + "\rUpgrade to " + itemName(to) + ",Are you sure?",
            [this, crystalSlot, gpsSlot] { m_services.connection.send(protocol::gpsUpgradeRequest(crystalSlot, gpsSlot)); }
        );
    }

    void GpsWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const Mark& mark = m_points[static_cast<std::size_t>(m_point - 1)];
        if (mark.map != 0) {
            const auto name = m_hooks.sceneName(mark.map);
            if (!name) {
                return;
            }
            const int half = static_cast<int>(name->size()) / 2 * 8; // half the text width at 8 px per character
            canvas.drawText({at.x + m_nameSpot.at().x - half, at.y + m_nameSpot.at().y}, *name, plain(), m_nameSpot.rect().width);
            canvas.drawText({at.x + m_xSpot.at().x, at.y + m_xSpot.at().y}, std::string("X") + kColon + std::to_string(mark.x), plain(), m_xSpot.rect().width);
            canvas.drawText({at.x + m_ySpot.at().x, at.y + m_ySpot.at().y}, std::string("Y") + kColon + std::to_string(mark.y), plain(), m_ySpot.rect().width);
        } else {
            // the 20 characters of the empty-point line at 8 px each; text width 112
            const int half = 20 / 2 * 8;
            canvas.drawText({at.x + m_emptySpot.at().x - half, at.y + m_emptySpot.at().y}, "Field without record", plain(), 112);
        }
    }

}
