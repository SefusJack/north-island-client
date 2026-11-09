#include "client/viewmodels/crafting/BlueprintWindow.h"

#include "gfx/TextStyle.h"
#include "ui/DrawingArea.h"

#include <algorithm>
#include <utility>

namespace client {

    namespace {

        constexpr int kIconSide = 34; // side of one item cell, px
        constexpr int kLineWidth = 200; // text clip width, px
        constexpr int kProductSub =
            48000; // product item ids start at 48001 (Hot-air Balloon, Airship, Boat, ...); id - 48000 is the icon_blueprint<N>_1 title number
        constexpr int kMostMaterials = 5; // a recipe has at most five materials

        /// @brief Loads BlueprintWindow.xml and returns the window.
        /// @param services Game services providing the layout and load context.
        /// @param screen Screen the layout is loaded into.
        /// @return The BlueprintWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("BlueprintWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("BlueprintWindow");
        }

        /// @brief Offset that centres an icon of 1 to 5 cells across in a place of five cells: (50 - cells * 10) * 34 div 20.
        /// @param cells Icon width or height in cells.
        /// @return Pixel offset.
        int centred(int cells) { return (50 - cells * 10) * kIconSide / 20; } // 50: five cells of 10 units; 10: units per cell; 20: half of that scale

    }

    BlueprintWindow::BlueprintWindow(
        GameServices& services,
        ui::Screen& screen
    )
        : m_services(services),
          m_screen(screen),
          m_window(loadWindow(services, screen)),
          m_close(screen.get<ui::ImageButton>("BlueprintClose")),
          m_closeSmall(screen.get<ui::ImageButton>("BlueprintCloseSmall")),
          m_textSpot(screen.get<ui::Spot>("BlueprintText")),
          m_titleSpot(screen.get<ui::Spot>("BlueprintTitle")) {
        screen.on("onBlueprintClose", [this](const ui::EventArgs&) { m_window.setVisible(false); });
        screen.get<ui::DrawingArea>("BlueprintPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
    }

    void BlueprintWindow::toggle(int blueprintItemId) {
        if (m_window.isVisible()) {
            m_window.setVisible(false);
            return;
        }
        m_blueprintItem = blueprintItemId;
        layout();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void BlueprintWindow::layout() {
        m_product = {};
        m_materials.clear();
        m_titlePicture.clear();
        const formats::CompoundTable& compounds = m_services.compounds;
        const int productId = compounds.productOfBlueprint(m_blueprintItem);
        const int recipeIndex = compounds.indexOfProduct(productId);
        m_titlePicture = "icon_blueprint" + std::to_string(productId - kProductSub) + "_1";
        const formats::CompoundRecipe& recipe = compounds.at(recipeIndex);
        int count = 0;
        for (int number = 1; number <= kMostMaterials; ++number) {
            count += recipe.material(number).itemId > 0 ? 1 : 0;
        }
        const auto sizeOf = [&](int itemId) {
            const auto record = m_services.items.find(itemId);
            return core::Point{record ? record->cellWidth : 1, record ? record->cellHeight : 1};
        };
        const auto nameOf = [&](int number) {
            const formats::CompoundRecipe::Material& material = recipe.material(number);
            const auto record = m_services.items.find(material.itemId);
            return (record ? record->name : std::string()) + " x " + std::to_string(material.count);
        };
        if (count == 0 || recipeIndex == 0) {
            // Close button rect (x, y, w, h) and small close button rect of the first form
            m_window.setPicture("Form_blueprint_1", m_services.pictures);
            m_close.setBounds({295, 365, 34, 18});
            m_closeSmall.setBounds({585, 15, 13, 13});
            return;
        }
        const core::Point product = sizeOf(productId);
        m_product.itemId = productId;

        /// @brief Where a material's icon goes and where its text line is centred.
        struct Place {
            core::Point at;
            int lineX;
            int lineY;
        };

        std::vector<Place> places;
        int width = 0;
        int height = 0;
        const auto put = [&](const std::string& picture, int w, int h) {
            m_window.setPicture(picture, m_services.pictures);
            width = w;
            height = h;
            m_window.setBounds({m_window.bounds().x, m_window.bounds().y, w, h});
        };
        const auto iconAt = [&](int number, int x, int y) {
            return core::Point{centred(sizeOf(recipe.material(number).itemId).x) + x, centred(sizeOf(recipe.material(number).itemId).y) + y};
        };
        if (count == 5 || count == 3 || count == 4) {
            // form size 619 x 405; Close rect and small close rect; product x 160 (4 materials) or 230; product y 15; "Complete Map" at y 180 (x 160, or centred less 54)
            put("Form_blueprint_1", 619, 405);
            m_close.setBounds({295, 365, 34, 18});
            m_closeSmall.setBounds({585, 15, 13, 13});
            const int productX = count == 4 ? 160 : 230;
            m_product.at = {centred(product.x) + productX, centred(product.y) + 15};
            m_completeAt = count == 4 ? core::Point{centred(product.x) + 160, 180} : core::Point{width / 2 - 54, 180};
            // places of materials 1..5 on the big form: icon (x, y) offsets, then text line centre x and y
            places = {
                {},
                {iconAt(1, 230, 195), 315, 335},
                {iconAt(2, 60, 195), 145, 335},
                {iconAt(3, 400, 195), 485, 335},
                {iconAt(4, count == 4 ? 365 : 400, 15), count == 4 ? 450 : 485, 180},
                {iconAt(5, 60, 15), 145, 180}
            };
        } else if (count == 1) {
            // one-material form size 316 x 378; Close button 34 x 18 centred (17 = half width) at 7/8 height; small close 13 x 13 at 35 from the right and y 15
            put("Form_blueprint_3", 316, 378);
            m_close.setBounds({width / 2 - 17, height / 8 * 7, 34, 18});
            m_closeSmall.setBounds({width - 35, 15, 13, 13});
            m_product.at = {centred(product.x) + width / 4, centred(product.y) + height / 16};
            // 54: half of the "Complete Map" text width; 10: gap under the product icon
            m_completeAt = {width / 2 - 54, centred(product.y) + height / 16 + product.y * kIconSide + 10};
            // 6: sixth-row fractions of the height; 30: text line lift above the icon row
            places = {{}, {iconAt(1, width / 4, height / 16 * 6), width / 2, height / 8 * 6 - 30}};
        } else if (count == 2) {
            // two-material form size 471 x 378; same button layout as the one-material form
            put("Form_blueprint_2", 471, 378);
            m_close.setBounds({width / 2 - 17, height / 8 * 7, 34, 18});
            m_closeSmall.setBounds({width - 35, 15, 13, 13});
            m_product.at = {centred(product.x) + width / 4, centred(product.y) + height / 16};
            // 108: "Complete Map" x offset right of the first material; 17: per-cell height of the product; 10: gap
            m_completeAt = {
                centred(sizeOf(recipe.material(1).itemId).x) + width / 6 + 108,
                centred(sizeOf(recipe.material(1).itemId).y) + height / 16 * 6 - product.y * 17 + 10
            };
            // 6: sixth-row fractions of the height; 30: text line lift above the icon row
            places = {
                {},
                {iconAt(1, width / 6, height / 16 * 6), width / 3, height / 8 * 6 - 30},
                {iconAt(2, width / 2, height / 16 * 6), width / 3 * 2, height / 8 * 6 - 30}
            };
        } else {
            return;
        }
        for (int number = 1; number < static_cast<int>(places.size()); ++number) {
            const formats::CompoundRecipe::Material& material = recipe.material(number);
            if (material.itemId <= 0) {
                continue;
            }
            Entry entry;
            entry.itemId = material.itemId;
            entry.at = places[static_cast<std::size_t>(number)].at;
            if (material.count > 0) {
                const std::string text = nameOf(number);
                entry.text = text;
                // 9: pixels per two characters of the text
                entry.textAt = {
                    places[static_cast<std::size_t>(number)].lineX - static_cast<int>(text.size()) / 2 * 9,
                    places[static_cast<std::size_t>(number)].lineY
                };
            }
            m_materials.push_back(std::move(entry));
        }
    }

    void BlueprintWindow::icon(gfx::Canvas& canvas, const Entry& entry, core::Point at) {
        const auto record = m_services.items.find(entry.itemId);
        if (!record) {
            return;
        }
        const auto picture = m_services.pictures.find(std::to_string(record->iconId));
        if (!picture) {
            return;
        }
        const gfx::Picture& data = m_services.pictures.picture(*picture);
        const core::Rect source =
            core::Rect{0, 0, record->cellWidth * kIconSide, record->cellHeight * kIconSide}.intersected({0, 0, data.width(), data.height()});
        if (!source.isEmpty()) {
            canvas.drawPicture(*picture, {at.x + entry.at.x, at.y + entry.at.y}, source);
        }
    }

    void BlueprintWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (m_product.itemId == 0) {
            return;
        }
        const gfx::TextStyle style{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        canvas.drawText({at.x + m_completeAt.x, at.y + m_completeAt.y}, "Complete Map", style, kLineWidth, 0);
        for (const Entry& material : m_materials) {
            if (!material.text.empty()) {
                canvas.drawText({at.x + material.textAt.x, at.y + material.textAt.y}, material.text, style, kLineWidth, 0);
            }
        }
        icon(canvas, m_product, at);
        for (auto entry = m_materials.rbegin(); entry != m_materials.rend(); ++entry) {
            icon(canvas, *entry, at);
        }
        if (const auto title = m_services.pictures.find(m_titlePicture)) {
            const gfx::Picture& data = m_services.pictures.picture(*title);
            const core::Rect source = core::Rect{0, 0, m_titleSpot.rect().width, m_titleSpot.rect().height}.intersected({0, 0, data.width(), data.height()});
            if (!source.isEmpty()) {
                canvas.drawPicture(*title, at + m_titleSpot.at(), source);
            }
        }
    }

}
