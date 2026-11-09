#include "client/viewmodels/house/RetainFurWindow.h"

#include "client/rendering/painters/ScaledItemIcon.h"

#include <algorithm>

namespace client {

    namespace {

        /// @brief Loads RetainFurWindow.xml and returns its window.
        /// @param services Game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("RetainFurWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("RetainFurWindow");
        }

    }

    RetainFurWindow::RetainFurWindow(
        GameServices& services,
        ui::Screen& screen,
        MainButtons& buttons,
        const HomeFurniture& furniture,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_buttons(buttons),
          m_furniture(furniture),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::BlockList>("RetainFurList")),
          m_scroll(screen.get<ui::ScrollBar>("RetainFurScroll")),
          m_iconSpot(screen.get<ui::Spot>("RetainFurRowIcon")),
          m_nameSpot(screen.get<ui::Spot>("RetainFurRowName")) {
        screen.on("onRetainFurClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onRetainFurUp", [this](const ui::EventArgs&) { scrollTo(m_list.topIndex() - 1); });
        screen.on("onRetainFurDown", [this](const ui::EventArgs&) { scrollTo(m_list.topIndex() + 1); });
        screen.on("onRetainFurScrolled", [this](const ui::EventArgs& args) { scrollTo(args.index); });
        screen.on("onRetainFurPress", [this](const ui::EventArgs& args) { press(args.index); });
        screen.on("onRetainFurDrag", [this](const ui::EventArgs&) { m_hooks.carry(); });
        screen.on("onRetainFurRelease", [this](const ui::EventArgs&) { m_hooks.drop(); });
    }

    void RetainFurWindow::toggle() {
        if (isOpen()) {
            close();
        } else if (m_hooks.inFloorRoom()) {
            m_window.setVisible(true);
            if (m_screen.context().bringToFront) {
                m_screen.context().bringToFront(m_window.id());
            }
            fill();
        } else if (m_hooks.inOtherRoom()) {
            m_hooks.notice("Can't redeem furniture here.", 1200); // notice duration in ms
        }
    }

    void RetainFurWindow::update() {
        std::vector<int> pieces;
        if (m_hooks.ownHouse()) {
            for (int piece = 1; piece <= HomeFurniture::kMost; ++piece) {
                if (m_furniture.piece(piece).itemId != 0 && m_furniture.piece(piece).retained == 1) { // retained flag 1: the piece waits in the pick-up area
                    pieces.push_back(piece);
                }
            }
        }
        if (!pieces.empty() && m_hooks.inFloorRoom()) {
            m_buttons.showRetained(true);
        } else {
            m_buttons.showRetained(false);
            close();
        }
        if (pieces != m_pieces) {
            m_pieces = std::move(pieces);
            fill();
        }
    }

    void RetainFurWindow::fill() {
        if (!isOpen()) {
            return;
        }
        m_list.clear();
        for (const int piece : m_pieces) {
            const auto item = m_services.items.find(m_furniture.piece(piece).itemId);
            if (!item) {
                continue;
            }
            const ScaledItemIcon icon = scaledItemIcon(m_services, m_furniture.piece(piece).itemId, m_iconSpot.size());
            ui::BlockList::Row row;
            ui::BlockList::PictureBlock picture;
            picture.picture = icon.picture;
            picture.size = icon.size;
            picture.area = m_iconSpot.size();
            picture.horizontal = ui::BlockList::Align::Centre;
            picture.vertical = ui::BlockList::Align::Centre;
            picture.source = icon.source;
            row.pictures.push_back(picture);
            ui::BlockList::TextBlock name;
            name.text = item->name;
            name.offset = m_nameSpot.at();
            name.area = m_nameSpot.size();
            name.vertical = ui::BlockList::Align::Centre;
            name.color = m_nameSpot.color();
            row.texts.push_back(name);
            m_list.addRow(std::move(row));
        }
        scrollTo(m_list.topIndex());
    }

    void RetainFurWindow::scrollTo(int top) {
        const int rows = m_list.rowCount();
        const int shown = m_list.visibleRows();
        m_list.setTopIndex(std::clamp(top, 0, std::max(0, rows - shown)));
        m_scroll.setRange(rows, shown);
        m_scroll.setPosition(m_list.topIndex());
    }

    void RetainFurWindow::press(int row) {
        if (!m_hooks.ownHouse()) {
            close();
            return;
        }
        if (row < 0) {
            return;
        }
        int listed = -1;
        for (const int piece : m_pieces) {
            if (m_services.items.find(m_furniture.piece(piece).itemId) && ++listed == row) {
                m_hooks.take(piece);
                return;
            }
        }
    }

}
