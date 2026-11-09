#include "client/viewmodels/events/LotteryPrizesWindow.h"

#include "gfx/TextStyle.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kColumnPitch = 150; // x distance between prize columns, px
        constexpr int kRowPitch = 36; // y distance between prize rows, px
        constexpr int kRows = 10; // prizes per column

        /// @brief Loads LotteryPrizesWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "LotteryPrizesWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("LotteryPrizesWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("LotteryPrizesWindow");
        }

        /// @brief Gives the place of a prize icon in the window.
        /// @param index Prize number from 0.
        /// @return Top left of the icon.
        core::Point iconAt(int index) { return {index / kRows * kColumnPitch + 5, index % kRows * kRowPitch + 35}; } // 5, 35: pixel offset of the first icon

    }

    LotteryPrizesWindow::LotteryPrizesWindow(
        GameServices& services,
        ui::Screen& screen,
        const std::vector<protocol::LotteryPrize>& prizes,
        ItemInfoPopup& info
    )
        : m_services(services),
          m_screen(screen),
          m_prizes(prizes),
          m_info(info),
          m_window(loadWindow(services, screen)),
          m_page(screen.get<ui::DrawingArea>("LotteryPrizesPage")),
          m_cancel(screen.get<ui::ImageButton>("LotteryPrizesCancel")),
          m_close(screen.get<ui::ImageButton>("LotteryPrizesClose")),
          m_formSpot(screen.get<ui::Spot>("LotteryPrizesForm")),
          m_iconSpot(screen.get<ui::Spot>("LotteryPrizesIcon")),
          m_textColorSpot(screen.get<ui::Spot>("LotteryPrizesTextColor")) {
        m_page.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onLotteryPrizesClose", [this](const ui::EventArgs&) { close(); });
    }

    void LotteryPrizesWindow::open() {
        refresh();
        m_window.setBounds(
            {(800 - m_window.bounds().width) / 2, 60, m_window.bounds().width, m_formSpot.size().height}
        ); // 800: screen width; 60: pixels from the top
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void LotteryPrizesWindow::refresh() {
        const int count = static_cast<int>(std::count_if(m_prizes.begin(), m_prizes.end(), [this](const protocol::LotteryPrize& prize) {
            return prize.itemId != 0 && m_services.items.find(prize.itemId).has_value();
        }));
        const int width = std::max(0, count - 1) / kRows * m_formSpot.pitch().x + m_formSpot.size().width;
        m_window.setBounds({m_window.bounds().x, m_window.bounds().y, width, m_formSpot.size().height});
        m_page.setBounds({0, 0, width, m_formSpot.size().height});
        // Cancel button: 56 by 20 at y 400, centred (x - 20)
        m_cancel.setBounds({width / 2 - 20, 400, 56, 20});
        // small Close button: 18 by 18, 20 px from the right edge, 2 px from the top
        m_close.setBounds({width - 20, 2, 18, 18});
    }

    void LotteryPrizesWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const gfx::TextStyle style{m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const core::Rect frame{at.x, at.y, m_window.bounds().width, m_formSpot.size().height};
        if (const auto article = m_services.pictures.find("Article")) {
            canvas.drawPictureScaled(*article, frame);
        }
        const std::string title = "Online draw";
        canvas.drawText(
            {at.x + frame.width / 2 - static_cast<int>(title.size()) / 2 - 30, at.y + 10},
            title,
            style
        ); // 30: centring nudge of the title; 10: pixels from the top
        int index = 0;
        for (const protocol::LotteryPrize& prize : m_prizes) {
            const int column = index / kRows;
            const int row = index % kRows;
            ++index;
            const auto record = m_services.items.find(prize.itemId);
            if (prize.itemId == 0 || !record) {
                continue;
            }
            const core::Point icon{at.x + iconAt(index - 1).x, at.y + iconAt(index - 1).y};
            if (const auto picture = m_services.pictures.find(std::to_string(record->iconId))) {
                canvas.drawPicture(*picture, icon);
            }
            const std::string name = prize.count > 1 ? record->name + "X" + std::to_string(prize.count) : record->name;
            canvas.drawText(
                {at.x + column * kColumnPitch + 40, at.y + row * kRowPitch + 45},
                name,
                style
            ); // name lies 40 px right and 45 px below the column start
            if (core::Rect{icon.x, icon.y, m_iconSpot.size().width, m_iconSpot.size().width}.contains(m_screen.mouse())) {
                protocol::ItemInstance item;
                item.itemId = prize.itemId;
                item.count = prize.count;
                m_info.show(item, {icon.x, icon.y, m_iconSpot.size().width, m_iconSpot.size().width});
            }
        }
    }

}
