#include "client/viewmodels/mall/ArticleInfoWindow.h"

#include "gfx/TextStyle.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kMostRows = 41; // box contents must have fewer than this many entries to be shown
        constexpr int kFrameEdge = 3; // width of the frame picture's left and right border strips, px

        /// @brief Loads ArticleInfoWindow.xml into the screen and returns its window.
        /// @param services Shared game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The "ArticleWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ArticleInfoWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ArticleWindow");
        }

    }

    ArticleInfoWindow::ArticleInfoWindow(
        GameServices& services,
        ui::Screen& screen,
        const LuckyBags& bags,
        ItemInfoPopup& info
    )
        : m_services(services),
          m_screen(screen),
          m_bags(bags),
          m_info(info),
          m_window(loadWindow(services, screen)),
          m_content(screen.get<ui::DrawingArea>("ArticleContent")),
          m_close(screen.get<ui::ImageButton>("ArticleClose")),
          m_cancel(screen.get<ui::ImageButton>("ArticleCancel")),
          m_formSpot(screen.get<ui::Spot>("ArticleForm")),
          m_cellsSpot(screen.get<ui::Spot>("ArticleCells")),
          m_nameSpot(screen.get<ui::Spot>("ArticleName")),
          m_petSpot(screen.get<ui::Spot>("ArticlePet")),
          m_petNameSpot(screen.get<ui::Spot>("ArticlePetName")),
          m_titleSpot(screen.get<ui::Spot>("ArticleTitle")),
          m_textSpot(screen.get<ui::Spot>("ArticleTextColor")) {
        m_content.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paint(canvas, at, nowMs); });
        screen.on("onArticleClose", [this](const ui::EventArgs&) { hide(); });
    }

    core::Point ArticleInfoWindow::iconAt(int number) const {
        return {
            (number - 1) / m_cellsSpot.rows() * m_cellsSpot.pitch().x + m_cellsSpot.at().x,
            (number - 1) % m_cellsSpot.rows() * m_cellsSpot.pitch().y + m_cellsSpot.at().y
        };
    }

    void ArticleInfoWindow::show(int boxId) {
        const auto found = m_services.items.find(boxId);
        const std::string title = (found ? found->name : std::string()) + "Content";
        if (const auto box = GiftBoxContents::find(boxId)) {
            fill(title, box->get().entries, box->get().pets);
        } else if (const auto bag = m_bags.find(boxId); bag && static_cast<int>(bag->get().items.size()) <= kMostRows - 1) {
            fill(title, bag->get().items, false);
        }
    }

    void ArticleInfoWindow::fill(const std::string& title, const std::vector<GiftBoxContents::Entry>& entries, bool pets) {
        m_title = title;
        m_entries = entries;
        m_pets = pets;
        m_width = (static_cast<int>(entries.size()) - 1) / m_cellsSpot.rows() * m_formSpot.pitch().x + m_formSpot.rect().width;
        m_window.setBounds({m_formSpot.rect().x - m_width / 2, m_formSpot.rect().y, m_width, m_formSpot.rect().height});
        m_content.setBounds({0, 0, m_width, m_formSpot.rect().height});
        m_close.setPosition({m_width - 20, 2}); // close button: 20 px from the right edge, 2 px from the top
        m_cancel.setPosition({m_width / 2 - 20, 400}); // cancel button: centred (half its 40 px width), 400 px from the top
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void ArticleInfoWindow::hide() {
        m_window.setVisible(false);
        if (m_infoShown) {
            m_info.hide();
            m_infoShown = false;
        }
    }

    void ArticleInfoWindow::update() {
        if (!m_window.isVisible() || m_pets) {
            return;
        }
        const core::Point origin = m_window.bounds().position();
        const core::Point mouse = m_screen.mouse();
        for (std::size_t index = 0; index < m_entries.size(); ++index) {
            const core::Point at = origin + iconAt(static_cast<int>(index) + 1);
            const int hovered = m_screen.hoveredId();
            if (core::Rect{at.x, at.y, m_cellsSpot.rect().width, m_cellsSpot.rect().height}.contains(mouse) &&
                (hovered == m_window.id() || hovered == m_content.id())) {
                protocol::ItemInstance item;
                item.itemId = m_entries[index].itemId;
                item.count = std::max(1, m_entries[index].count);
                m_info.show(item, core::Rect{at.x, at.y, m_cellsSpot.rect().width, m_cellsSpot.rect().height});
                m_infoShown = true;
                return;
            }
        }
        if (m_infoShown) {
            m_info.hide();
            m_infoShown = false;
        }
    }

    void ArticleInfoWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        (void)nowMs;
        if (const auto frame = m_services.pictures.find("Article")) {
            canvas.drawPicture(*frame, at, core::Rect{0, 0, kFrameEdge, m_formSpot.rect().height});
            canvas.drawPicture(
                *frame,
                {at.x + m_width - kFrameEdge, at.y},
                core::Rect{m_formSpot.rect().width - kFrameEdge, 0, kFrameEdge, m_formSpot.rect().height}
            );
            canvas.drawPictureScaled(
                *frame,
                {at.x + kFrameEdge, at.y, m_width - 2 * kFrameEdge, m_formSpot.rect().height},
                core::Rect{kFrameEdge, 0, m_formSpot.rect().width - 2 * kFrameEdge, m_formSpot.rect().height}
            );
        }
        const int titleX = m_width / 2 - static_cast<int>(m_title.size()) / 2 - m_titleSpot.at().x;
        // title 10 px from the top; box width 8 px per character, height 15
        canvas.drawText(
            {at.x + titleX, at.y + m_titleSpot.at().y},
            m_title,
            {m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt},
            static_cast<int>(m_title.size()) * 8,
            15
        );
        for (std::size_t index = 0; index < m_entries.size(); ++index) {
            const int number = static_cast<int>(index) + 1;
            const GiftBoxContents::Entry& entry = m_entries[index];
            const auto item = m_services.items.find(entry.itemId);
            if (!item) {
                continue;
            }
            if (m_pets) {
                const core::Point cell{
                    at.x + (number - 1) / m_cellsSpot.rows() * m_cellsSpot.pitch().x + m_petSpot.at().x,
                    at.y + (number - 1) % m_cellsSpot.rows() * m_cellsSpot.pitch().y + m_petSpot.at().y
                };
                if (const auto npc = m_services.npcs.find(item->petNpcId)) {
                    constexpr int kPetSlot = 0;
                    m_services.characters.drawPet(canvas, *npc, {cell.x + 6, cell.y - 6}, kPetSlot, 0); // sprite offset (+6, -6) from the cell
                    // pet name 5 px below the cell top; box height 15
                    canvas.drawText(
                        {at.x + (number - 1) / m_cellsSpot.rows() * m_cellsSpot.pitch().x + m_petNameSpot.rect().x, cell.y + m_petNameSpot.rect().y},
                        npc->name,
                        {m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                        m_petNameSpot.rect().width,
                        m_petNameSpot.rect().height
                    );
                }
                continue;
            }
            const core::Point icon = at + iconAt(number);
            if (const auto picture = m_services.pictures.find(std::to_string(item->iconId))) {
                canvas.drawPicture(*picture, icon, core::Rect{0, 0, m_cellsSpot.rect().width, m_cellsSpot.rect().height});
            }
            const std::string name = entry.count > 1 ? item->name + "X" + std::to_string(entry.count) : item->name;
            // name row starts 45 px below the icon row top (minus the name spot's y); box height 15
            canvas.drawText(
                {at.x + (number - 1) / m_cellsSpot.rows() * m_cellsSpot.pitch().x + m_nameSpot.rect().x,
                 at.y + (number - 1) % m_cellsSpot.rows() * m_cellsSpot.pitch().y + 45 - m_nameSpot.rect().y},
                name,
                {m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                m_nameSpot.rect().width,
                m_nameSpot.rect().height
            );
        }
    }

}
