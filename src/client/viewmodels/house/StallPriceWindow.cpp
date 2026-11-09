#include "client/viewmodels/house/StallPriceWindow.h"

#include "protocol/FurnitureStallRequests.h"

#include <cstdlib>
#include <string>

namespace client {

    namespace {

        constexpr int kMostPrice = 99999999; // highest price that can be entered
        constexpr int kAllowedMark = 1; // bag use mark 1 (compound window mark) that lockNotice lets through

        /// @brief Loads StallPriceWindow.xml and returns its window.
        /// @param services Game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("StallPriceWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("StallPriceWindow");
        }

    }

    StallPriceWindow::StallPriceWindow(
        GameServices& services,
        ui::Screen& screen,
        const BagMarks& marks,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_screen(screen),
          m_marks(marks),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_price(screen.get<ui::TextBox>("StallPrice")) {
        screen.get<ui::DrawingArea>("StallPricePaint").setPainter([this](gfx::Canvas&, core::Point, std::uint64_t) { paint(); });
        screen.on("onStallPriceConfirm", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onStallPriceCancel", [this](const ui::EventArgs&) { close(); });
        screen.on("onStallPriceMore", [this](const ui::EventArgs&) { step(1); });
        screen.on("onStallPriceLess", [this](const ui::EventArgs&) { step(-1); });
        m_price.setText("0");
    }

    void StallPriceWindow::open(
        Mode mode,
        int furnIndex,
        int itemId,
        int bagSlot,
        int count,
        int vendingRow
    ) {
        m_mode = mode;
        m_furnIndex = furnIndex;
        m_itemId = itemId;
        m_bagSlot = bagSlot;
        m_count = count;
        m_vendingRow = vendingRow;
        m_price.setText("0");
        m_window.setVisible(true);
        if (m_screen.context().bringAbovePopups) {
            m_screen.context().bringAbovePopups(m_window.id());
        }
    }

    int StallPriceWindow::price() const { return m_price.text().empty() ? 0 : std::atoi(m_price.text().c_str()); }

    void StallPriceWindow::closeForCase() {
        if (m_window.isVisible() && (m_mode == Mode::Case || m_mode == Mode::CaseReprice)) {
            close();
        }
    }

    void StallPriceWindow::openReprice(int furnIndex, std::int32_t price) {
        open(Mode::CaseReprice, furnIndex, 0, 0, 0, 0);
        m_price.setText(std::to_string(price));
    }

    void StallPriceWindow::step(int by) {
        if (by < 0 || price() != kMostPrice) {
            m_price.setText(std::to_string(price() + by));
        }
    }

    void StallPriceWindow::paint() {
        if (!m_window.isVisible()) {
            return;
        }
        if (!m_hooks.inFloorRoom()) {
            close();
        }
        if (!m_price.text().empty() && price() < 0) {
            m_price.setText("0");
        }
    }

    void StallPriceWindow::confirm() {
        if (price() <= 0) {
            m_messages.show("Wrong price. Please enter again", 1000); // notice duration in ms
            return;
        }
        if (m_mode == Mode::Case) {
            if (!m_hooks.storable(m_bagSlot)) {
                m_messages.show("The item can't be stored", 1000); // notice duration in ms
                return;
            }
            if (!m_hooks.caseFree()) {
                m_messages.show("No enough space for you to sell\xA1I", 1000); // notice duration in ms
                return;
            }
            m_hooks.putInCase(m_furnIndex, m_bagSlot, price());
            close();
            return;
        }
        if (m_mode == Mode::CaseReprice) {
            m_hooks.repriceCase(price());
            close();
            return;
        }
        if (m_mode == Mode::Shelf) {
            if (const auto lock = m_marks.lockNotice(m_bagSlot, kAllowedMark)) {
                m_messages.show(lock->text, lock->ms);
                return;
            }
            if (!m_marks.isAvailable(m_bagSlot)) {
                m_messages.show("Unable to operate when the item is locked", 2000); // notice duration in ms
                return;
            }
            m_hooks.send(protocol::putOnShelfRequest(m_furnIndex, m_itemId, m_bagSlot, m_count, price()));
        } else {
            m_hooks.send(protocol::putInVendingRequest(m_furnIndex, m_itemId, m_bagSlot, m_count, price(), m_vendingRow));
        }
        close();
    }

}
