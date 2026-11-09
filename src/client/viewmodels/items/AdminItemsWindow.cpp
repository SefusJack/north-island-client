#include "client/viewmodels/items/AdminItemsWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rendering/painters/ScaledItemIcon.h"
#include "ui/ImageButton.h"
#include "ui/Label.h"
#include "ui/TextBox.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace client {

    namespace {

        constexpr int kKinds = 15; // category buttons AdminItemsKind0 .. 14 (Kind)
        constexpr int kMaxCount = 999; // most items given in one /give

        /// @brief Loads the window's layout and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the form is loaded into.
        /// @return The admin items window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("AdminItemsWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("AdminItems");
        }

        /// @brief Reads the number that follows a prefix in a control name.
        /// @param name Control name.
        /// @param prefix Expected prefix.
        /// @return The number, or 0 when the name does not start with the prefix.
        int numberAfter(const std::string& name, const std::string& prefix) { return name.rfind(prefix, 0) == 0 ? std::stoi(name.substr(prefix.size())) : 0; }

        /// @brief Lower-cases a text.
        /// @param text Text to convert.
        /// @return Lower-case copy.
        std::string lower(std::string text) {
            std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return text;
        }

    }

    AdminItemsWindow::AdminItemsWindow(
        GameServices& services,
        ui::Screen& screen,
        std::function<void(const std::string&)> sendCommand
    )
        : m_services(services),
          m_screen(screen),
          m_sendCommand(std::move(sendCommand)),
          m_window(loadWindow(services, screen)),
          m_grid(screen.get<ui::IconGrid>("AdminItemsGrid")),
          m_scroll(screen.get<ui::ScrollBar>("AdminItemsScroll")),
          m_chosenSpot(screen.get<ui::Spot>("AdminItemsChosenCell")) {
        screen.get<ui::DrawingArea>("AdminItemsChosen").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintChosen(canvas, at); });
        screen.on("onAdminBadge", [this](const ui::EventArgs&) { open(); });
        screen.on("onAdminItemsClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onAdminItemsKind", [this](const ui::EventArgs& args) { showKind(static_cast<Kind>(numberAfter(args.source, "AdminItemsKind"))); });
        screen.on("onAdminItemsSearch", [this](const ui::EventArgs&) { filter(); });
        screen.on("onAdminItemsScrolled", [this](const ui::EventArgs& args) { scrollTo(args.index); });
        screen.on("onAdminItemsWheel", [this](const ui::EventArgs& args) { scrollTo(m_topRow - args.index); });
        screen.on("onAdminItemsPick", [this](const ui::EventArgs& args) { pick(args.index); });
        screen.on("onAdminItemsQuickGive", [this](const ui::EventArgs& args) {
            const std::size_t at = static_cast<std::size_t>(m_topRow * kColumns + args.index - 1);
            if (args.index >= 1 && at < m_shown.size()) {
                m_chosen = m_shown[at];
                give(m_chosen, 1);
            }
        });
        screen.on("onAdminItemsGive", [this](const ui::EventArgs&) {
            const std::string& amount = m_screen.get<ui::TextBox>("AdminItemsAmount").text();
            give(m_chosen, amount.empty() ? 1 : std::clamp(std::stoi(amount), 1, kMaxCount));
        });
    }

    void AdminItemsWindow::setAdminMode(bool on) {
        const bool was = std::exchange(m_adminMode, on);
        m_screen.get<ui::ImageButton>("AdminBadge").setVisible(on);
        if (on && !was) {
            open();
        } else if (!on) {
            close();
        }
    }

    void AdminItemsWindow::open() {
        if (!m_adminMode) {
            return;
        }
        if (m_allIds.empty()) {
            for (const int id : m_services.items.ids()) {
                if (const auto record = m_services.items.find(id); record && record->unlisted == 0) {
                    m_allIds.push_back(id);
                }
            }
            m_screen.get<ui::TextBox>("AdminItemsAmount").setText("1");
            showKind(Kind::All);
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void AdminItemsWindow::close() { m_window.setVisible(false); }

    void AdminItemsWindow::showKind(Kind kind) {
        m_kind = kind;
        for (int i = 0; i < kKinds; ++i) {
            m_screen.get<ui::ImageButton>("AdminItemsKind" + std::to_string(i)).setSelected(i == static_cast<int>(kind));
        }
        filter();
    }

    AdminItemsWindow::Kind AdminItemsWindow::kindOf(const formats::ItemRecord& item) {
        // Item.dat's item type (record byte 21)
        switch (item.type) {
            case 1: // knives
            case 2: // swords
            case 3: // spears
            case 4: // bows
            case 5: // wands
            case 6: // claws
            case 7: // (no items)
            case 8: // axes
            case 9: // sticks
            case 10: // fans
            case 11: return Kind::Weapons; // guns
            case 12: return Kind::Clothes; // clothes
            case 13: return Kind::Hats; // headdresses
            case 14: return Kind::Gloves; // gloves and bracers
            case 15: return Kind::Shoes; // shoes
            case 16: return Kind::Jewelry; // rings and other accessories
            case 20: // ability items (candy, syrup, perfume)
            case 24: return Kind::Usables; // battle items
            case 23: return Kind::Food; // cooked food and drink
            case 21: return Kind::Quest; // quest props and scrolls
            case 17: // weapon boxes
            case 25: // utility boxes (EXP Bag, Picnic Box, Compass, ...)
            case 26: // gift packs
            case 40: // gift cards
            case 41: return Kind::Boxes; // lucky bags
            case 31: // flowers
            case 32: // herbs
            case 33: // ores
            case 34: // stones
            case 35: // clays
            case 36: // woods
            case 37: // feathers and pet materials
            case 38: // gas
            case 51: // water
            case 52: // stat gems (Devil's Spar)
            case 54: return Kind::Materials; // bomb diamonds
            case 29: // instruments and machines
            case 30: return Kind::Furniture; // furniture
            case 39: return Kind::Vehicles; // vehicles
            default: break; // tools and building parts (28), tents (27), castle defence (53), any other type
        }
        return Kind::Other;
    }

    void AdminItemsWindow::filter() {
        const std::string search = lower(m_screen.get<ui::TextBox>("AdminItemsSearch").text());
        const bool byId = !search.empty() && std::all_of(search.begin(), search.end(), [](unsigned char c) { return std::isdigit(c) != 0; });
        m_shown.clear();
        for (const int id : m_allIds) {
            const auto record = m_services.items.find(id);
            if (!record || (m_kind != Kind::All && kindOf(*record) != m_kind)) {
                continue;
            }
            if (byId ? std::to_string(id).rfind(search, 0) == 0 : search.empty() || lower(record->name).find(search) != std::string::npos) {
                m_shown.push_back(id);
            }
        }
        const int rows = (static_cast<int>(m_shown.size()) + kColumns - 1) / kColumns;
        m_scroll.setRange(rows, kRows);
        m_topRow = -1;
        scrollTo(0);
    }

    void AdminItemsWindow::scrollTo(int row) {
        const int rows = (static_cast<int>(m_shown.size()) + kColumns - 1) / kColumns;
        row = std::clamp(row, 0, std::max(0, rows - kRows));
        if (row == m_topRow) {
            return;
        }
        m_topRow = row;
        m_scroll.setPosition(row);
        refresh();
    }

    void AdminItemsWindow::refresh() {
        m_grid.clearCells();
        const std::size_t first = static_cast<std::size_t>(m_topRow * kColumns);
        for (int cell = 1; cell <= kColumns * kRows && first + static_cast<std::size_t>(cell - 1) < m_shown.size(); ++cell) {
            protocol::ItemInstance item;
            item.itemId = m_shown[first + static_cast<std::size_t>(cell - 1)];
            ui::IconCell icon = itemNameCell(m_services, item);
            icon.count.reset();
            // a large item (furniture, ...): the whole of its middle part shrunk into the cell
            if (const ScaledItemIcon whole = scaledItemIcon(m_services, item.itemId, m_grid.cellSize()); whole.picture.isValid() && whole.source) {
                icon.picture = whole.picture;
                icon.source = *whole.source;
                icon.size = whole.size;
            }
            icon.hint += " (" + std::to_string(item.itemId) + ")";
            m_grid.setCell(cell, icon);
        }
        const std::size_t last = std::min(m_shown.size(), first + static_cast<std::size_t>(kColumns * kRows));
        m_screen.get<ui::Label>("AdminItemsCount")
            .setText(
                m_shown.empty() ? (m_kind == Kind::All ? "No items." : "No items here; try All.")
                                : std::to_string(first + 1) + "-" + std::to_string(last) + " of " + std::to_string(m_shown.size()) + " items"
            );
    }

    void AdminItemsWindow::pick(int cell) {
        const std::size_t at = static_cast<std::size_t>(m_topRow * kColumns + cell - 1);
        if (cell < 1 || at >= m_shown.size()) {
            return;
        }
        m_chosen = m_shown[at];
        const auto record = m_services.items.find(m_chosen);
        m_screen.get<ui::Label>("AdminItemsChoice").setText((record ? record->name : std::string("?")) + " (" + std::to_string(m_chosen) + ")");
    }

    void AdminItemsWindow::give(int itemId, int count) {
        if (itemId == 0) {
            m_screen.get<ui::Label>("AdminItemsChoice").setText("Choose an item first.");
            return;
        }
        m_sendCommand("/give " + std::to_string(itemId) + " " + std::to_string(count));
    }

    void AdminItemsWindow::paintChosen(gfx::Canvas& canvas, core::Point at) {
        const auto found = std::find(m_shown.begin(), m_shown.end(), m_chosen);
        if (m_chosen == 0 || found == m_shown.end()) {
            return;
        }
        const int cell = static_cast<int>(found - m_shown.begin()) - m_topRow * kColumns + 1;
        if (cell < 1 || cell > kColumns * kRows) {
            return;
        }
        const core::Point position = m_grid.bounds().position() + m_grid.cellPosition(cell);
        canvas.fillRectAlpha({at.x + position.x, at.y + position.y, m_chosenSpot.size().width, m_chosenSpot.size().height}, m_chosenSpot.color());
    }

}
