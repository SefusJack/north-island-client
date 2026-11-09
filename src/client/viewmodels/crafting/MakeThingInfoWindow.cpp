#include "client/viewmodels/crafting/MakeThingInfoWindow.h"

#include "client/rules/FurnitureBonus.h"
#include "client/rules/FurnitureKinds.h"
#include "client/rendering/painters/ScaledItemIcon.h"
#include "gfx/TextStyle.h"
#include "protocol/ManufactureRequests.h"

#include <algorithm>
#include <cstdlib>
#include <string>

namespace client {

    namespace {

        constexpr int kQuantityPiece = 0x94A4; // item 38052: Mill (furniture piece that unlocks making several at once)

        /// @brief Makes a plain (no effect) text style of a colour.
        /// @param color Text colour.
        /// @return The style.
        gfx::TextStyle plain(const core::Color& color) { return {color, gfx::TextEffect::Plain, std::nullopt}; }

        /// @brief Pads a text with spaces on the left to a width.
        /// @details Original: Format('%<width>s', [text]).
        /// @param text Text to pad.
        /// @param width Minimum width.
        /// @return The padded text.
        std::string padLeft(const std::string& text, std::size_t width) { return text.size() >= width ? text : std::string(width - text.size(), ' ') + text; }

        /// @brief Loads MakeThingInfoWindow.xml and returns the window.
        /// @param services Game services providing the layout and load context.
        /// @param screen Screen the layout is loaded into.
        /// @return The MakeThingInfoWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MakeThingInfoWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("MakeThingInfoWindow");
        }

        /// @brief Finds a picture by name.
        /// @param services Game services holding the pictures.
        /// @param name Picture name.
        /// @return The picture id, or an invalid id when missing.
        gfx::PictureId pictureNamed(GameServices& services, const std::string& name) { return services.pictures.find(name).value_or(gfx::PictureId{}); }

    }

    MakeThingInfoWindow::MakeThingInfoWindow(
        GameServices& services,
        ui::Screen& screen,
        const HomeFurniture& furniture,
        MakeJobs& jobs,
        const MakeMaterials& materials,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_furniture(furniture),
          m_jobs(jobs),
          m_materials(materials),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::BlockList>("MakeThingMaterials")),
          m_count(screen.get<ui::TextBox>("MakeThingCount")),
          m_titleSpot(screen.get<ui::Spot>("MakeThingInfoTitle")),
          m_productSpot(screen.get<ui::Spot>("MakeThingInfoProduct")),
          m_neededSpot(screen.get<ui::Spot>("MakeThingInfoNeeded")),
          m_timeSpot(screen.get<ui::Spot>("MakeThingInfoTime")),
          m_quantitySpot(screen.get<ui::Spot>("MakeThingInfoQuantity")),
          m_plusSpot(screen.get<ui::Spot>("MakeThingInfoPlus")),
          m_frameSpot(screen.get<ui::Spot>("MakeThingInfoFrame")),
          m_rowColorSpot(screen.get<ui::Spot>("MakeThingInfoRowColor")),
          m_markSpot(screen.get<ui::Spot>("MakeThingInfoMark")),
          m_enough(pictureNamed(services, "Icon_Enough_1")),
          m_lack(pictureNamed(services, "Icon_Lack_1")),
          m_grid(pictureNamed(services, "Icon_MakeThingInfo_Grid_1")),
          m_plus(pictureNamed(services, "Icon_MakeThingInfo_Plus_1")),
          m_quantity(pictureNamed(services, "Icon_MakeThingInfo_MakeQuant_1")) {
        screen.get<ui::DrawingArea>("MakeThingInfoPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.get<ui::DrawingArea>("MakeThingMarks").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintMarks(canvas, at); });
        screen.on("onMakeThingInfoClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onMakeThingConfirm", [this](const ui::EventArgs&) { confirm(); });
        m_count.setText("1");
    }

    void MakeThingInfoWindow::show(int recipe, int piece, int blueprintSource, int blueprintSlot) {
        m_list.clear();
        m_recipe = recipe;
        m_piece = piece;
        m_blueprintSource = blueprintSource;
        m_blueprintSlot = blueprintSlot;
        if (recipe < 1 || recipe > m_services.compounds.size()) {
            return;
        }
        const auto product = m_services.items.find(m_services.compounds.at(recipe).productId);
        if (product && product->unlisted == 1) { // 1: the product is flagged unlisted, so the window closes
            close();
            return;
        }
        fillRows();
        m_count.setText("1");
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void MakeThingInfoWindow::close() {
        m_piece = 0;
        m_window.setVisible(false);
    }

    void MakeThingInfoWindow::fillRows() {
        using Align = ui::BlockList::Align;
        const formats::CompoundRecipe& recipe = m_services.compounds.at(m_recipe);
        const gfx::PictureId semicolon = pictureNamed(m_services, "Icon_Semicolon_1");
        const gfx::PictureId piece = pictureNamed(m_services, "Icon_Piece_1");
        const auto sizeOf = [this](gfx::PictureId picture) {
            return picture.isValid() ? core::Size{m_services.pictures.picture(picture).width(), m_services.pictures.picture(picture).height()} : core::Size{};
        };
        for (int number = 1; number <= recipe.materialCount; ++number) {
            const formats::CompoundRecipe::Material& material = recipe.material(number);
            const auto item = m_services.items.find(material.itemId);
            if (!item) {
                m_list.clear();
                return;
            }
            ui::BlockList::Row row;
            // 68 x 68: size of the material icon area; offset (0, -19) lifts the icon; (215, 0) semicolon and (238, -10) piece icon offsets; 20 x 30: their area
            const ScaledItemIcon icon = scaledItemIcon(m_services, material.itemId, {68, 68});
            ui::BlockList::PictureBlock picture;
            picture.picture = icon.picture;
            picture.offset = {0, -19};
            picture.size = icon.size;
            picture.area = {68, 68};
            picture.horizontal = Align::Centre;
            picture.vertical = Align::Centre;
            picture.source = icon.source;
            row.pictures.push_back(picture);
            row.pictures.push_back({semicolon, {215, 0}, sizeOf(semicolon), {20, 30}, Align::Centre, Align::Centre, std::nullopt, std::nullopt});
            row.pictures.push_back({piece, {238, -10}, sizeOf(piece), {20, 30}, Align::Centre, Align::Centre, std::nullopt, std::nullopt});
            // text blocks: name at x 75 (115 x 30), the count the player has at x 189 padded to 4 (45 x 30), the count needed at x 228 padded to 3 (45 x 30)
            row.texts.push_back({item->name, {75, 0}, {115, 30}, Align::Start, Align::Centre, m_rowColorSpot.color()});
            row.texts.push_back({padLeft("0", 4), {189, 0}, {45, 30}, Align::Start, Align::Centre, m_rowColorSpot.color()});
            row.texts.push_back({padLeft(std::to_string(material.count), 3), {228, 0}, {45, 30}, Align::Start, Align::Centre, m_rowColorSpot.color()});
            m_list.addRow(std::move(row));
        }
    }

    int MakeThingInfoWindow::quantity() const {
        const std::string& text = m_count.text();
        return text.empty() ? 0 : std::atoi(text.c_str());
    }

    void MakeThingInfoWindow::setQuantity(int quantity) { m_count.setText(std::to_string(quantity)); }

    bool MakeThingInfoWindow::hasQuantityPiece() const { return FurnitureBonus::findById(m_furniture, kQuantityPiece, true) != 0; }

    void MakeThingInfoWindow::refresh() {
        if (!m_hooks.inFloorRoom()) {
            close();
            return;
        }
        if (m_recipe < 1 || m_recipe > m_services.compounds.size() || m_list.rowCount() <= 0) {
            return;
        }
        const formats::CompoundRecipe& recipe = m_services.compounds.at(m_recipe);
        const bool several = hasQuantityPiece() && m_materials.isStackable(recipe.productId) &&
                             FurnitureKinds::checkHomeDiscard(m_services.items, recipe.productId) != FurnitureKinds::Discard::Placeable;
        if (!several) {
            setQuantity(1);
        }
        if (m_count.text().empty()) {
            setQuantity(0);
        }
        m_check = m_materials.check(recipe, quantity());
        if (m_check.quantity != quantity()) {
            setQuantity(m_check.quantity);
        }
        for (int number = 1; number <= recipe.materialCount && number <= m_list.rowCount(); ++number) {
            const auto index = static_cast<std::size_t>(number);
            ui::BlockList::Row& row = m_list.row(number - 1);
            // 4 and 3: digit widths of the have and need columns
            row.texts[1].text = padLeft(std::to_string(m_check.have[index]), 4);
            row.texts[2].text = padLeft(std::to_string(m_check.need[index]), 3);
        }
    }

    int MakeThingInfoWindow::makeMinutes() const {
        return FurnitureBonus::minutes(quantity() * m_services.compounds.at(m_recipe).makeMinutes, FurnitureBonus::flags(m_furniture));
    }

    void MakeThingInfoWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (!m_window.isVisible()) {
            return;
        }
        refresh();
        if (!m_window.isVisible() || m_recipe < 1 || m_recipe > m_services.compounds.size()) {
            return;
        }
        const int rows = m_list.rowCount();
        for (int row = 2; row <= rows; ++row) { // the plus and frame sit between rows, so they start at the second row
            if (m_plus.isValid() && m_grid.isValid()) {
                canvas.drawPicture(m_plus, {at.x + m_plusSpot.at().x, at.y + m_plusSpot.at().y + (row - 2) * m_plusSpot.pitch().y});
                canvas.drawPicture(m_grid, {at.x + m_frameSpot.at().x, at.y + m_frameSpot.at().y + (row - 2) * m_plusSpot.pitch().y});
            }
        }
        const formats::CompoundRecipe& recipe = m_services.compounds.at(m_recipe);
        canvas.drawText(at + m_titleSpot.at(), " Manufacture ", plain(m_titleSpot.color()), 100); // text clip width in pixels
        std::string name;
        if (const auto product = m_services.items.find(recipe.productId)) {
            name = product->name;
        }
        const int each = recipe.productCount + FurnitureBonus::extraProducts(m_furniture, m_services.compounds, recipe.productId, 0);
        if (each > 1) {
            name += "x" + std::to_string(each);
        }
        canvas.drawText(at + m_productSpot.at(), name, plain(m_productSpot.color()), 284); // text clip width in pixels
        canvas.drawText(at + m_neededSpot.at(), "Accessory needed", plain(m_neededSpot.color()), 256); // text clip width in pixels
        canvas.drawText(
            at + m_timeSpot.at(),
            padLeft(std::to_string(makeMinutes()), 3),
            plain(m_timeSpot.color()),
            100
        ); // 3: minutes digit width; 100: text clip width in pixels
        const bool several = hasQuantityPiece();
        m_count.setVisible(several);
        if (several && m_quantity.isValid()) {
            canvas.drawPicture(m_quantity, at + m_quantitySpot.at());
        }
    }

    void MakeThingInfoWindow::paintMarks(gfx::Canvas& canvas, core::Point at) {
        if (!m_window.isVisible() || m_list.rowCount() <= 0 || m_count.text() == "0") {
            return;
        }
        for (int row = 0; row < m_list.rowCount(); ++row) {
            const gfx::PictureId mark = m_check.enough[static_cast<std::size_t>(row + 1)] ? m_enough : m_lack;
            if (mark.isValid()) {
                canvas.drawPicture(mark, {at.x + m_markSpot.at().x, at.y + m_markSpot.at().y + row * m_markSpot.pitch().y});
            }
        }
    }

    void MakeThingInfoWindow::confirm() {
        if (m_recipe < 1 || m_recipe > m_services.compounds.size()) {
            return;
        }
        if (m_hooks.hasMaid() && m_services.compounds.at(m_recipe).makeMinutes > 0) {
            m_hooks.choosePartner([this](int petSlot, int petId) { return submit(true, petSlot, petId); });
            return;
        }
        submit(false, 0, 0);
    }

    bool MakeThingInfoWindow::submit(bool viaPartners, int petSlot, int petId) {
        const formats::CompoundRecipe& recipe = m_services.compounds.at(m_recipe);
        if (m_hooks.bathing()) {
            m_messages.show("Bathing, cannot make items", 2000); // message duration in ms
            return false;
        }
        if (m_jobs.playerSlot() != 0) {
            m_messages.show("Manufacture one item once", 2000); // message duration in ms
            return false;
        }
        if (const std::string why = m_hooks.refusal(); petSlot == 0 && !why.empty()) {
            m_messages.show(why, 2000); // message duration in ms
            return false;
        }
        if (!m_hooks.inFloorRoom()) {
            close();
            return true;
        }
        if (m_jobs.freeSlot() == 0) {
            m_messages.show("There are 5 unfinished articles, unable to manufacture more", 3000); // message duration in ms
            return false;
        }
        refresh();
        if (!m_check.ok) {
            m_messages.show("Not enough materials", 2000); // message duration in ms
            return false;
        }
        if (m_count.text() == "0") {
            m_messages.show("The number of times should not be 0 ", 2000); // message duration in ms
            return false;
        }
        const auto product = m_services.items.find(recipe.productId);
        if (!product) {
            return false;
        }
        const bool placeable = FurnitureKinds::checkHomeDiscard(m_services.items, recipe.productId) == FurnitureKinds::Discard::Placeable;
        if (placeable && (product->furnPlaceFlags & FurnitureKinds::kOnWall) != FurnitureKinds::kOnWall &&
            !FurnitureKinds::isFloorItem(m_services.items, recipe.productId) && !FurnitureKinds::isWallpaperItem(m_services.items, recipe.productId) &&
            product->furnPicture0 == 0) {
            m_messages.show("This item has no picture", 2000); // message duration in ms
            return false;
        }
        m_jobs.setPet(petSlot, petId);
        if (viaPartners) {
            m_hooks.partnerOrdered();
        }
        m_hooks.send(protocol::startMakingRequest(m_piece, m_recipe, m_check.request, m_blueprintSource, m_blueprintSlot, petSlot, petId));
        if (recipe.makeMinutes > 0) {
            close();
            return true;
        }
        return false;
    }

}
