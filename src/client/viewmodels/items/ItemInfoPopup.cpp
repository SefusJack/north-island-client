#include "client/viewmodels/items/ItemInfoPopup.h"

#include "client/rules/ItemInfo.h"
#include "gfx/TextStyle.h"

namespace client {

    namespace {

        constexpr int kWidth = 188; // popup width, px
        constexpr int kLineHeight = 20; // height of one text line, px
        constexpr int kScreenWidth = 800; // game screen width, px
        constexpr int kScreenHeight = 600; // game screen height, px

        /// @brief Loads ItemInfoPopup.xml and returns its panel.
        /// @param services Game services (layout, load context).
        /// @param screen Screen to load into.
        /// @return The popup panel.
        ui::Element& loadPopup(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ItemInfoPopup.xml"), services.loadContext());
            return screen.get<ui::Element>("ItemInfo");
        }

    }

    ItemInfoPopup::ItemInfoPopup(
        GameServices& services,
        ui::Screen& screen,
        const PlayerStatus& status
    )
        : m_services(services),
          m_screen(screen),
          m_status(status),
          m_panel(loadPopup(services, screen)),
          m_comparePanel(screen.get<ui::Element>("ItemCompare")),
          m_lineSpot(screen.get<ui::Spot>("ItemInfoLine")),
          m_headerSpot(screen.get<ui::Spot>("ItemInfoHeader")) {
        screen.get<ui::DrawingArea>("ItemInfoLines").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            paint(canvas, at, m_text, nullptr);
        });
        screen.get<ui::DrawingArea>("ItemCompareLines").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            paint(canvas, at, m_compareText, "[Current Equipment]");
        });
    }

    namespace {

        /// @brief Height of the popup for its lines.
        /// @param text The lines (the last one is the wrapped description).
        /// @return Height in pixels.
        int popupHeight(const std::vector<std::string>& text) {
            const int lastLength = static_cast<int>(text.back().size());
            // 8 pixels per character, one extra line per 130 pixels
            return static_cast<int>(text.size()) * kLineHeight + kLineHeight + lastLength * 8 / 130 * kLineHeight;
        }

        /// @brief Moves a panel to a box, sizes its first child to fill it and shows it.
        /// @param panel The panel.
        /// @param box Screen box.
        void place(ui::Element& panel, const core::Rect& box) {
            panel.setBounds(box);
            panel.children().front()->setBounds({0, 0, box.width, box.height});
            panel.setVisible(true);
        }

    }

    void ItemInfoPopup::raise() {
        if (m_screen.context().bringAbovePopups) {
            m_screen.context().bringAbovePopups(m_panel.id());
        }
    }

    void ItemInfoPopup::show(const protocol::ItemInstance& item, const core::Rect& itemBox, const std::optional<protocol::ItemInstance>& worn) {
        if (m_panel.isVisible() && m_shown.itemId == item.itemId && m_shown.usedAmount == item.usedAmount && m_shownBox.x == itemBox.x &&
            m_shownBox.y == itemBox.y && m_shownHp == m_status.hpBottle && m_shownSp == m_status.spBottle) {
            raise();
            return;
        }
        m_shownHp = m_status.hpBottle;
        m_shownSp = m_status.spBottle;
        m_text = describeItem(m_services.items, item, true, {m_status.hpBottle, m_status.spBottle});
        m_compareText = worn ? describeItem(m_services.items, *worn) : std::vector<std::string>{};
        if (m_text.empty()) {
            hide();
            return;
        }
        m_shown = item;
        m_shownBox = itemBox;
        // 2 pixels gap between the item box and the popup
        core::Rect box{itemBox.x + itemBox.width + 2, itemBox.y, kWidth, popupHeight(m_text)};
        if (box.x + box.width > kScreenWidth) {
            box.x = itemBox.x - kWidth - 2;
        }
        if (box.y + box.height > kScreenHeight) {
            box.y = kScreenHeight - box.height;
        }
        if (m_compareText.empty()) {
            m_comparePanel.setVisible(false);
            place(m_panel, box);
            raise();
            return;
        }
        constexpr int kCellWidth = 32; // width of an item cell, px
        constexpr int kSnapWidth = 10; // distance from the right screen edge within which the pair is pushed back in, px
        constexpr int kLowest = 565; // lowest y a popup's bottom may reach before it is moved up, px
        constexpr int kBottom = 575; // y the popup's bottom is moved to when it was too low, px
        core::Rect compare{0, 0, kWidth, popupHeight(m_compareText) + kLineHeight};
        box.x = itemBox.x + kCellWidth;
        compare.x = box.x + box.width;
        if (compare.x + compare.width > kScreenWidth) {
            box.x = itemBox.x - box.width;
            compare.x = box.x - compare.width;
        } else if (compare.x + compare.width > kScreenWidth - kSnapWidth) {
            box.x = kScreenWidth - compare.width - box.width;
        }
        if (box.y + box.height > kLowest) {
            box.y = kBottom - box.height;
        }
        compare.y = box.y;
        if (compare.y + compare.height > kLowest) {
            compare.y = kBottom - compare.height;
            box.y = compare.y;
        }
        place(m_panel, box);
        place(m_comparePanel, compare);
        raise();
    }

    void ItemInfoPopup::showStallItem(const protocol::ItemInstance& item, core::Point at) {
        constexpr int kIconWidth = 32; // item icon size, px
        constexpr int kGap = 2; // gap between icon and popup, px
        constexpr int kStallGap = 35; // extra x shift of the popup for stall items, px
        if (m_panel.isVisible() && m_shown.itemId == item.itemId && m_shownBox.x == at.x && m_shownBox.y == at.y) {
            raise();
            return;
        }
        m_text = describeItem(m_services.items, item, false);
        if (m_text.empty()) {
            hide();
            return;
        }
        m_shown = item;
        m_shownBox = {at.x, at.y, kIconWidth, kIconWidth};
        core::Rect box{at.x + kIconWidth + kGap + kStallGap, at.y, kWidth, popupHeight(m_text)};
        if (box.x + box.width > kScreenWidth) {
            box.x = at.x - kWidth - kGap;
        }
        if (box.y + box.height > kScreenHeight) {
            box.y = kScreenHeight - box.height;
        }
        m_comparePanel.setVisible(false);
        place(m_panel, box);
        raise();
    }

    void ItemInfoPopup::hide() {
        m_panel.setVisible(false);
        m_comparePanel.setVisible(false);
        m_shown = {};
    }

    void ItemInfoPopup::paint(gfx::Canvas& canvas, core::Point at, const std::vector<std::string>& text, const char* header) const {
        const gfx::TextStyle style{m_lineSpot.color(), gfx::TextEffect::Shadow, std::nullopt};
        int top = at.y + 10; // top margin in pixels
        if (header != nullptr) {
            // left margin of the text is 5 pixels
            canvas.drawText({at.x + 5, top}, header, {m_headerSpot.color(), gfx::TextEffect::Shadow, std::nullopt}, m_headerSpot.size().width, kLineHeight);
            top += kLineHeight;
        }
        for (std::size_t i = 0; i < text.size(); ++i) {
            // left margin of the text is 5 pixels
            canvas.drawText({at.x + 5, top + static_cast<int>(i) * kLineHeight}, text[i], style, m_lineSpot.size().width, kLineHeight);
        }
    }

}
