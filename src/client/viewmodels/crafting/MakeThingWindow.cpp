#include "client/viewmodels/crafting/MakeThingWindow.h"

#include "client/rules/FurnitureBonus.h"
#include "gfx/TextStyle.h"

#include <algorithm>
#include <cstdio>
#include <set>
#include <string>

namespace client {

    namespace {

        constexpr int kRowGap = 2; // gap between tree rows, px
        constexpr int kMostRecipes = 200; // cap on listed recipes (the loop stops at this count)
        constexpr int kCharWidth = 8; // width of one text character, px
        constexpr int kLineHeight = 15; // text line height, px
        // products whose count is not raised by the furniture bonus
        constexpr int kFixedProducts[] = {
            30256, // Halloween Pumpkin
            30383, // 2010 Halloween Pumpkin
            55012, // 2011 Pumpkin
            55052, // 2012 Spooky Pumpkin
            55090 // 2013 Spooky Pumpkin
        };

        /// @brief Builds a plain text style of one colour.
        /// @param color Text colour.
        /// @return The text style.
        gfx::TextStyle plain(const core::Color& color) { return {color, gfx::TextEffect::Plain, std::nullopt}; }

        /// @brief Loads MakeThingWindow.xml into the screen and returns its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MakeThingWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("MakeThingWindow");
        }

        /// @brief Looks a picture up by name.
        /// @param services Game services.
        /// @param name Picture name.
        /// @return The picture id, or an invalid id when missing.
        gfx::PictureId pictureNamed(GameServices& services, const std::string& name) { return services.pictures.find(name).value_or(gfx::PictureId{}); }

        /// @brief Draws a tree line's text, centred vertically when shorter than the area is wide.
        /// @details Original: TRe_TreeLine.DrawLineItem's text.
        /// @param canvas Target canvas.
        /// @param at Top-left of the text area.
        /// @param text The text.
        /// @param areaWidth Width of the text area.
        /// @param areaHeight Height of the text area.
        /// @param color Text colour.
        void drawLineText(
            gfx::Canvas& canvas,
            core::Point at,
            const std::string& text,
            int areaWidth,
            int areaHeight,
            const core::Color& color
        ) {
            const int width = static_cast<int>(text.size()) * kCharWidth;
            if (width < areaWidth && areaHeight > kLineHeight) {
                at.y += (areaHeight - kLineHeight) / 2;
            }
            canvas.drawText(at, text, plain(color), areaWidth, kLineHeight);
        }

        /// @brief Centres a picture in an area of a row when the area is larger than it.
        /// @param at Top-left of the area.
        /// @param area Area size.
        /// @param size Picture size.
        /// @return Top-left to draw the picture at.
        core::Point centred(core::Point at, core::Size area, core::Size size) {
            return {
                area.width > size.width ? at.x + (area.width - size.width) / 2 : at.x,
                area.height > size.height ? at.y + (area.height - size.height) / 2 : at.y
            };
        }

    }

    MakeThingWindow::MakeThingWindow(
        GameServices& services,
        ui::Screen& screen,
        const HomeFurniture& furniture,
        MakeJobs& jobs,
        const MakeMaterials& materials,
        MakeThingInfoWindow& info,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_furniture(furniture),
          m_jobs(jobs),
          m_materials(materials),
          m_info(info),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_tree(screen.get<ui::DrawingArea>("MakeThingTree")),
          m_scroll(screen.get<ui::ScrollBar>("MakeThingScroll")),
          m_rowSpot(screen.get<ui::Spot>("MakeThingRow")),
          m_indentSpot(screen.get<ui::Spot>("MakeThingIndent")),
          m_iconSpot(screen.get<ui::Spot>("MakeThingIcon")),
          m_textSpot(screen.get<ui::Spot>("MakeThingText")),
          m_groupSpot(screen.get<ui::Spot>("MakeThingGroup")),
          m_fold(pictureNamed(services, "Icon_Fold_1")),
          m_unfold(pictureNamed(services, "Icon_Unfold_1")),
          m_minuteIcon(pictureNamed(services, "Icon_Minute_1")),
          m_hoverTile(pictureNamed(services, "Icon_BlueNet32x32_1")) {
        m_tree.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onMakeThingTreeClick", [this](const ui::EventArgs&) { clicked(); });
        screen.on("onMakeThingScrolled", [this](const ui::EventArgs& args) { scrollTo(args.index); });
        screen.on("onMakeThingUp", [this](const ui::EventArgs&) { scrollTo(m_top - 1); });
        screen.on("onMakeThingDown", [this](const ui::EventArgs&) { scrollTo(m_top + 1); });
        screen.on("onMakeThingClose", [this](const ui::EventArgs&) { close(); });
    }

    void MakeThingWindow::show(int piece, const std::vector<int>& facilityIds) {
        m_groups.clear();
        m_top = 0;
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_piece = piece;
        load(facilityIds);
    }

    void MakeThingWindow::close() {
        m_piece = 0;
        m_window.setVisible(false);
    }

    std::string MakeThingWindow::minutesText(const formats::CompoundRecipe& recipe) const {
        char text[16]; // buffer for the "%3d" minutes text
        std::snprintf(text, sizeof text, "%3d", FurnitureBonus::minutes(recipe.makeMinutes, FurnitureBonus::flags(m_furniture)));
        return text;
    }

    void MakeThingWindow::load(const std::vector<int>& facilityIds) {
        const bool anyMakes =
            std::any_of(facilityIds.begin(), facilityIds.end(), [this](int facility) { return m_services.compounds.findByFacility(facility, 0) != 0; });
        if (!anyMakes) {
            close();
            return;
        }
        int recipeCount = 0;
        std::set<int> seen;
        for (const int facility : facilityIds) {
            if (!seen.insert(facility).second) {
                continue;
            }
            const auto item = m_services.items.find(facility);
            if (!item) {
                break;
            }
            Group group;
            group.name = item->name;
            const int saved = FurnitureBonus::extraProducts(m_furniture, m_services.compounds, facility, 1);
            int index = 0;
            while ((index = m_services.compounds.findByFacility(facility, index + 1)) != 0) {
                const formats::CompoundRecipe& recipe = m_services.compounds.at(index);
                const auto product = m_services.items.find(recipe.productId);
                if (!product || product->unlisted == 1) { // 1: product flagged unlisted, so it is left out of the list
                    continue;
                }
                const bool fixed = std::find(std::begin(kFixedProducts), std::end(kFixedProducts), recipe.productId) != std::end(kFixedProducts);
                const int each = recipe.productCount + (fixed ? 0 : saved);
                if (++recipeCount >= kMostRecipes) {
                    break;
                }
                Recipe entry;
                entry.index = index;
                entry.name = product->name + (each > 1 ? "x" + std::to_string(each) : "");
                entry.minutes = minutesText(recipe);
                entry.icon = scaledItemIcon(m_services, recipe.productId, m_iconSpot.size());
                group.recipes.push_back(std::move(entry));
            }
            m_groups.push_back(std::move(group));
        }
        if (m_groups.size() == 1) {
            m_groups.front().expanded = true;
        }
        scrollTo(0);
    }

    std::vector<MakeThingWindow::Line> MakeThingWindow::lines() const {
        std::vector<Line> rows;
        for (int group = 0; group < static_cast<int>(m_groups.size()); ++group) {
            rows.push_back({group, -1});
            if (m_groups[static_cast<std::size_t>(group)].expanded) {
                for (int recipe = 0; recipe < static_cast<int>(m_groups[static_cast<std::size_t>(group)].recipes.size()); ++recipe) {
                    rows.push_back({group, recipe});
                }
            }
        }
        return rows;
    }

    int MakeThingWindow::visibleRows() const {
        int rows = m_tree.bounds().height / m_rowSpot.pitch().y;
        if (m_tree.bounds().height - m_rowSpot.pitch().y * rows == m_rowSpot.rect().height) {
            ++rows;
        }
        return rows;
    }

    void MakeThingWindow::scrollTo(int top) {
        const int count = static_cast<int>(lines().size());
        m_top = std::clamp(top, 0, std::max(0, count - visibleRows()));
        m_scroll.setRange(count, visibleRows());
        m_scroll.setPosition(m_top);
    }

    int MakeThingWindow::hoveredRow() const {
        if (m_screen.hoveredId() != m_tree.id()) {
            return -1;
        }
        const int y = m_screen.mouse().y - (m_window.bounds().position().y + m_tree.bounds().position().y);
        const int rowPitch = m_rowSpot.pitch().y;
        const int row = (y + rowPitch - 1) / rowPitch;
        if (y < 0 || row < 1 || row * rowPitch - y <= kRowGap || row * rowPitch - kRowGap > m_tree.bounds().height) {
            return -1;
        }
        return row - 1 + m_top < static_cast<int>(lines().size()) ? row - 1 : -1;
    }

    void MakeThingWindow::clicked() {
        const int row = hoveredRow();
        if (row < 0) {
            return;
        }
        const Line line = lines()[static_cast<std::size_t>(m_top + row)];
        Group& group = m_groups[static_cast<std::size_t>(line.group)];
        if (line.recipe < 0) {
            group.expanded = !group.expanded;
            scrollTo(m_top);
        } else {
            pick(group.recipes[static_cast<std::size_t>(line.recipe)].index);
        }
    }

    void MakeThingWindow::pick(int recipeIndex) {
        if (!m_hooks.inFloorRoom()) {
            close();
            m_info.close();
            return;
        }
        const formats::CompoundRecipe& recipe = m_services.compounds.at(recipeIndex);
        int source = 0;
        int slot = 0;
        const auto name = [this](int itemId) {
            const auto item = m_services.items.find(itemId);
            return item ? item->name : std::string();
        };
        if (recipe.blueprintItemId != 0) {
            const MakeMaterials::BlueprintAt found = m_materials.blueprintAt(recipe.blueprintItemId);
            if (found.source != 0) {
                source = found.source;
                slot = found.slot;
            } else {
                m_messages.show(
                    " Manufacture " + name(recipe.productId) + " Need " + "\" " + name(recipe.blueprintItemId) + "\" ",
                    2500
                ); // message display time in ms
                return;
            }
        }
        for (int job = 1; job <= MakeJobs::kSlots; ++job) {
            const MakeSlot& waiting = m_jobs.slot(job);
            if (waiting.productId > 0 && !waiting.active && recipe.productId == waiting.productId) {
                m_hooks.askContinue(job, name(recipe.productId), m_piece, source, slot);
                return;
            }
        }
        m_info.show(recipeIndex, m_piece, source, slot);
    }

    void MakeThingWindow::paintTiled(gfx::Canvas& canvas, core::Rect area, gfx::PictureId picture) {
        if (!picture.isValid()) {
            return;
        }
        const int tileWidth = m_services.pictures.picture(picture).width();
        const int tileHeight = m_services.pictures.picture(picture).height();
        const int width = area.width + 1;
        const int height = area.height + 1;
        const int across = width / tileWidth;
        const int down = height / tileHeight;
        const int restWidth = width - across * tileWidth;
        const int restHeight = height - down * tileHeight;
        for (int i = 0; i <= across; ++i) {
            for (int j = 0; j <= down; ++j) {
                const int w = i == across ? restWidth : tileWidth;
                const int h = j == down ? restHeight : tileHeight;
                if (w > 0 && h > 0) {
                    canvas.drawPicture(picture, {area.x + i * tileWidth, area.y + j * tileHeight}, core::Rect{0, 0, w, h});
                }
            }
        }
    }

    void MakeThingWindow::paintGroup(gfx::Canvas& canvas, core::Point at, const Group& group) {
        const gfx::PictureId fold = group.expanded ? m_unfold : m_fold;
        if (!group.recipes.empty() && fold.isValid()) {
            const gfx::Picture& picture = m_services.pictures.picture(fold);
            canvas.drawPicture(fold, centred(at, {m_indentSpot.at().x, m_rowSpot.rect().height}, {picture.width(), picture.height()}));
        }
        drawLineText(canvas, {at.x + m_indentSpot.at().x, at.y}, group.name, 115, m_rowSpot.rect().height, m_groupSpot.color()); // width of the name area
    }

    void MakeThingWindow::paintRecipe(gfx::Canvas& canvas, core::Point at, const Recipe& recipe) {
        // name text x offset (after the product icon) and name area width
        drawLineText(canvas, {at.x + 39, at.y}, recipe.name, 115, m_rowSpot.rect().height, m_textSpot.color());
        if (m_minuteIcon.isValid()) {
            // minute icon: area x offset 166, y offset -10, area width 20, icon size 17 x 17
            canvas.drawPicture(m_minuteIcon, centred({at.x + 166, at.y - 10}, {20, m_rowSpot.rect().height}, {17, 17}));
        }
        // minutes text: x offset 169, y offset 11, area width 35
        canvas.drawText({at.x + 169, at.y + 11}, recipe.minutes, plain(m_textSpot.color()), 35, kLineHeight);
        if (recipe.icon.picture.isValid()) {
            const core::Point place = centred(at, m_iconSpot.size(), recipe.icon.size);
            if (recipe.icon.source) {
                canvas.drawPictureScaled(recipe.icon.picture, {place.x, place.y, recipe.icon.size.width, recipe.icon.size.height}, recipe.icon.source);
            } else {
                canvas.drawPicture(recipe.icon.picture, place);
            }
        }
    }

    void MakeThingWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (!m_window.isVisible()) {
            return;
        }
        if (!m_hooks.inFloorRoom()) {
            close();
            m_info.close();
            return;
        }
        const std::vector<Line> rows = lines();
        const int hover = hoveredRow();
        if (hover >= 0) {
            paintTiled(canvas, {at.x, at.y + hover * m_rowSpot.pitch().y, m_rowSpot.rect().width, m_rowSpot.rect().height}, m_hoverTile);
        }
        const int shown = std::min(visibleRows(), static_cast<int>(rows.size()) - m_top);
        for (int row = 0; row < shown; ++row) {
            const Line& line = rows[static_cast<std::size_t>(m_top + row)];
            const Group& group = m_groups[static_cast<std::size_t>(line.group)];
            const core::Point place{at.x, at.y + row * m_rowSpot.pitch().y};
            if (line.recipe < 0) {
                paintGroup(canvas, place, group);
            } else {
                paintRecipe(canvas, {place.x + m_indentSpot.at().x, place.y}, group.recipes[static_cast<std::size_t>(line.recipe)]);
            }
        }
    }

}
