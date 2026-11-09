#include "client/viewmodels/trade/SaleNetWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "gfx/TextStyle.h"
#include "protocol/ItemRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <vector>

namespace client {

    namespace {

        constexpr int kCheckSize = 23; // check box picture size, px
        constexpr int kMostAmount = 9999999; // largest amount that can be entered

        /// @brief Loads SaleNetWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The "SaleNet" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("SaleNetWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("SaleNet");
        }

    }

    SaleNetWindow::SaleNetWindow(
        GameServices& services,
        ui::Screen& screen,
        std::uint8_t& requestPending
    )
        : m_services(services),
          m_screen(screen),
          m_requestPending(requestPending),
          m_window(loadWindow(services, screen)),
          m_items(screen.get<ui::IconGrid>("SaleNetItems")),
          m_checks(screen.get<ui::IconGrid>("SaleNetChecks")),
          m_amount(screen.get<ui::TextBox>("SaleNetAmount")),
          m_valueSpot(screen.get<ui::Spot>("SaleNetValue")) {
        screen.get<ui::DrawingArea>("SaleNetPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintValue(canvas, at); });
        screen.get<ui::DrawingArea>("SaleNetMarks").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintMarks(canvas, at); });
        screen.on("onSaleNetCheckPress", [this](const ui::EventArgs& args) { pressCheck(args.index); });
        screen.on("onSaleNetAmountUp", [this](const ui::EventArgs&) { changeAmount(1); });
        screen.on("onSaleNetAmountDown", [this](const ui::EventArgs&) { changeAmount(-1); });
        screen.on("onSaleNetGet", [this](const ui::EventArgs&) { get(); });
        screen.on("onSaleNetClose", [this](const ui::EventArgs&) { hide(); });
        m_amount.setText("0");
        refresh();
    }

    void SaleNetWindow::show(const protocol::SaleNetList& list) {
        m_places.fill({});
        m_state.fill(0);
        m_value = list.value;
        for (const protocol::SaleNetList::Entry& entry : list.entries) {
            if (entry.place < 1 || entry.place > kPlaces) {
                continue;
            }
            const std::size_t place = static_cast<std::size_t>(entry.place);
            if (entry.item.itemId != 0) {
                m_places[place] = entry.item;
            }
            // place states: 2 on sale, 3 competed for; anything else is unchecked
            m_state[place] = entry.state == 2 || entry.state == 3 ? entry.state : 0;
        }
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void SaleNetWindow::refresh() {
        m_items.clearCells();
        m_checks.clearCells();
        for (int place = 1; place <= kPlaces; ++place) {
            const std::size_t at = static_cast<std::size_t>(place);
            if (!m_places[at].empty()) {
                ui::IconCell cell = itemNameCell(m_services, m_places[at]);
                cell.hint.clear();
                m_items.setCell(place, cell);
            }
            ui::IconCell check;
            check.source = {0, 0, kCheckSize, kCheckSize};
            if (m_state[at] <= 1) { // only unchecked (0) or checked (1) places show a check picture
                check.picture = m_services.pictures.find(m_state[at] == 1 ? "icon_Check" : "icon_unCheck").value_or(gfx::PictureId{});
            }
            m_checks.setCell(place, check);
        }
    }

    void SaleNetWindow::pressCheck(int place) {
        if (place < 1 || place > kPlaces) {
            return;
        }
        int& state = m_state[static_cast<std::size_t>(place)];
        // state 2: on sale, cannot be checked
        if (state == 2 || m_places[static_cast<std::size_t>(place)].empty()) {
            return;
        }
        state = state == 1 ? 0 : 1;
        refresh();
    }

    std::int32_t SaleNetWindow::amount() const {
        const std::string& text = m_amount.text();
        // at most 9 digits fit an int32
        if (text.empty() || text.size() > 9 || text.find_first_not_of("0123456789") != std::string::npos) {
            return 0;
        }
        return std::stoi(text);
    }

    void SaleNetWindow::changeAmount(int step) {
        std::int32_t value = amount();
        if (step > 0 && value + 1 < kMostAmount) {
            ++value;
        } else if (step < 0 && value - 1 > 0) {
            --value;
        }
        m_amount.setText(std::to_string(value));
    }

    void SaleNetWindow::get() {
        if (m_requestPending != 0) {
            return;
        }
        std::vector<int> places;
        for (int place = 1; place <= kPlaces; ++place) {
            if (m_state[static_cast<std::size_t>(place)] == 1) {
                places.push_back(place);
                m_state[static_cast<std::size_t>(place)] = 0;
            }
        }
        m_requestPending = 3; // pending marker: sale net get requested
        m_services.connection.send(protocol::saleNetGetRequest(amount(), places));
        m_amount.setText("0");
        refresh();
    }

    void SaleNetWindow::paintValue(gfx::Canvas& canvas, core::Point at) {
        const std::string text = std::to_string(m_value);
        const gfx::TextStyle style{m_valueSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        // right edge x 166, y 90, 8 px per digit, text box 100x16
        canvas.drawText({at.x + 166 - static_cast<int>(text.size()) * 8, at.y + 90}, text, style, 100, 16);
    }

    void SaleNetWindow::paintMarks(gfx::Canvas& canvas, core::Point at) {
        for (int place = 1; place <= kPlaces; ++place) {
            const int state = m_state[static_cast<std::size_t>(place)];
            if (state != 2 && state != 3) { // only 2 (on sale) and 3 (competed for) get a mark
                continue;
            }
            if (const auto picture = m_services.pictures.find(state == 2 ? "icon_ForSale" : "icon_Compete")) {
                // five places per row
                canvas.drawPicture(
                    *picture,
                    {at.x + m_items.bounds().x + (place - 1) % 5 * m_items.pitch().x, at.y + m_items.bounds().y + (place - 1) / 5 * m_items.pitch().y}
                );
            }
        }
    }

}
