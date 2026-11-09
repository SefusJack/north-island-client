#include "client/rendering/house/HousePainter.h"

#include "client/rules/FurnitureKinds.h"
#include "client/models/house/HouseGrid.h"
#include "client/models/house/Sandglass.h"
#include "client/rules/ItemInfo.h"
#include "client/rules/PriceColors.h"
#include "client/data/VehicleKinds.h"
#include "gfx/TextStyle.h"

#include <algorithm>
#include <cmath>

namespace client {

    namespace {

        constexpr int kPictureDrop = 15; // px: pictures are drawn this far below their cell centre
        constexpr int kStripWidth = 30; // px: width of one isometric floor-cell strip of a furniture picture
        constexpr int kStripHeight = 300; // px: height of a strip (full picture height)
        constexpr int kStripMiddle = 120; // px: x of the middle strip within the furniture picture
        constexpr int kWindmill = 37202; // Windmill
        constexpr int kLift = 39014; // Elevator
        constexpr int kTreadmill = 39105; // Treadmill
        constexpr int kBicepExerciser = 39100; // Bicep Exerciser
        constexpr int kHoverLight = 15; // brightness added to the picture under the mouse
        constexpr int kDraggedLight = 20; // brightness added to the picture being dragged
        constexpr int kWallLight = 20; // brightness added to a wall/floor picture under the mouse
        constexpr std::uint64_t kFrameMs = 200; // ms per animation frame
        constexpr int kShelfPlaces = 5; // item places on a shelf
        constexpr int kCasePlaces = 2; // item places in a display case
        constexpr int kShelfKind = 1; // shelf kind 1: shelf
        constexpr int kDisplayCaseKind = 3; // shelf kind 3: display case
        const core::Color kLabelColor = core::Color::fromRgb565(2016); // RGB565 0x07E0: green
        constexpr int kCharWidth = 8; // px: width of one text character
        constexpr int kCellSide = 30; // px: side of one house floor cell
        constexpr int kLabelRise = 16; // px: furniture name label sits this far above the piece
        constexpr core::Point kWindmillLabelShift{-20, 40}; // px: extra x/y shift of the Windmill name label
        constexpr int kItemLabelRise = 20; // px: stall item name sits this far above the price line
        constexpr int kPriceGap = 44; // px: x gap between the "Price" word and the price value
        constexpr int kLastRow = 50; // last house grid row scanned
        constexpr int kLastColumn = 44; // last house grid column scanned
        constexpr int kFloorColumns = 4; // floor tile covers 4 grid columns
        constexpr int kFloorRows = 3; // floor tile covers 3 grid rows
        // rows: {vehicle item id, x offset px, y offset px} of the exhibited vehicle picture
        constexpr std::array<std::array<int, 3>, 17> kExhibitOffsets{
            {{48001, 14, 5}, // Hot-air Balloon
             {48002, 9, 0}, // Airship
             {48003, 6, 1}, // Boat
             {48004, 7, 0}, // Mighty Plane
             {48005, 0, 15}, // Jalor
             {48006, -1, 13}, // Steam Ship
             {48007, 7, 0}, // Motorbike
             {48008, 1, 5}, // Magic Carpet
             {48009, 0, 15}, // Turtle Car
             {48010, -3, 20}, // Raft
             {48011, 2, 10}, // Cabriolet
             {48012, 7, 0}, // Yacht
             {48013, 7, 0}, // UFO
             {48014, -3, 2}, // Robot
             {48015, 7, 0}, // lifeboat (untranslated, not opened)
             {48017, 20, 20}, // Submarine
             {48018, -2, 5}} // Space Craft
        };

    }

    void HousePainter::drawClipped(gfx::Canvas& canvas, gfx::PictureId picture, core::Point at, const core::Rect& source) {
        if (!picture.isValid()) {
            return;
        }
        const gfx::Picture& data = canvas.pictures().picture(picture);
        const core::Rect inside = source.intersected({0, 0, data.width(), data.height()});
        if (inside.isEmpty()) {
            return;
        }
        canvas.drawPicture(picture, {at.x + inside.x - source.x, at.y + inside.y - source.y}, inside);
    }

    void HousePainter::paintFloorAndWalls(gfx::Canvas& canvas, const Camera& camera) {
        const HouseSurfaces& surfaces = m_interior.surfaces();
        const HouseRoomRect& area = m_interior.furniture().room(m_interior.room());
        const int roomTop = area.top - HouseGrid::kGridRow;
        const auto put = [&](gfx::PictureId picture, int cellX, int cellY, const core::Point& size, bool lit = false) {
            const core::Point world = HousePillars::worldOf(cellX, cellY);
            const core::Point at = camera.toScreen(world);
            drawClipped(canvas, picture, {at.x - size.x / 2, at.y - size.y / 2 + kPictureDrop}, {0, 0, size.x, size.y});
            if (lit) {
                canvas.drawPictureLight(picture, {at.x - size.x / 2, at.y - size.y / 2 + kPictureDrop}, kWallLight, core::Rect{0, 0, size.x, size.y});
            }
        };
        bool over = false;
        HomeFurniture::Span span;
        if (m_light) {
            over = (m_light->rowCol.x == 0 && m_light->rowCol.y > roomTop) || (m_light->rowCol.x > 0 && m_light->rowCol.y == roomTop);
            span = m_interior.furniture().spanOf(m_light->itemId, m_light->direction).value_or(HomeFurniture::Span{});
        }
        const auto sizeOf = [&](gfx::PictureId picture) {
            const gfx::Picture& data = canvas.pictures().picture(picture);
            return core::Point{data.width(), data.height()};
        };
        if (surfaces.floorItem() > 0 && surfaces.floorPicture().isValid()) {
            const core::Point size = sizeOf(surfaces.floorPicture());
            for (int column = 1; column <= area.spanX() / kFloorColumns; ++column) {
                for (int row = 1; row <= area.spanY() / kFloorRows; ++row) {
                    put(surfaces.floorPicture(), 1 + (column - 1) * kFloorColumns, roomTop + 1 + (row - 1) * kFloorRows, size);
                }
            }
            if (area.spanY() % kFloorRows != 0) {
                for (int column = 1; column <= area.spanX() / kFloorColumns; ++column) {
                    put(surfaces.floorPicture(), 1 + (column - 1) * kFloorColumns, area.bottom - area.top + 1 - kFloorRows + roomTop + 1, size);
                }
            }
        }
        if (surfaces.wallItem() > 0 && surfaces.topWallPicture().isValid() && surfaces.leftWallPicture().isValid()) {
            const core::Point size = sizeOf(surfaces.topWallPicture());
            for (int column = 1; column <= area.spanX(); ++column) {
                put(surfaces.topWallPicture(), column, roomTop + 1, size, over && m_light->rowCol.x <= column && m_light->rowCol.x + span.x > column);
            }
            for (int row = 1; row <= area.spanY(); ++row) {
                put(surfaces.leftWallPicture(),
                    1,
                    roomTop + row,
                    size,
                    over && m_light->rowCol.y <= roomTop + row && m_light->rowCol.y + span.y > roomTop + row);
            }
        }
    }

    void HousePainter::paintPillar(gfx::Canvas& canvas, const Camera& camera, int cellX, int cellY) {
        const gfx::PictureId picture = m_interior.surfaces().pillarPicture();
        const int x = cellX - HouseGrid::kGridColumn + 1;
        const int y = cellY - HouseGrid::kGridRow + 1;
        if (!picture.isValid() || !m_interior.pillars().has(x, y, m_interior.room())) {
            return;
        }
        const gfx::Picture& data = canvas.pictures().picture(picture);
        const core::Point at = camera.toScreen(HousePillars::worldOf(x, y));
        const HousePillars& pillars = m_interior.pillars();
        if (pillars.carried() == core::Point{x, y}) {
            canvas.drawPictureLight(
                picture,
                {at.x - data.width() / 2, at.y - data.height() / 2 + kPictureDrop},
                kDraggedLight,
                core::Rect{0, 0, data.width(), data.height()}
            );
            return;
        }
        drawClipped(canvas, picture, {at.x - data.width() / 2, at.y - data.height() / 2 + kPictureDrop}, {0, 0, data.width(), data.height()});
        if (pillarLit(x, y) || pillars.lit() == core::Point{x, y}) {
            canvas.drawPictureLight(
                picture,
                {at.x - data.width() / 2, at.y - data.height() / 2 + kPictureDrop},
                kHoverLight,
                core::Rect{0, 0, data.width(), data.height()}
            );
        }
    }

    void HousePainter::paintPillarEdit(gfx::Canvas& canvas, const Camera& camera, int cellX, int cellY) {
        const HousePillars& pillars = m_interior.pillars();
        const core::Point cell{cellX - HouseGrid::kGridColumn + 1, cellY - HouseGrid::kGridRow + 1};
        const bool cursor = pillars.editMode() == 1 && pillars.marked() == cell;
        const bool carried = pillars.editMode() == 3 && pillars.carried() != core::Point{} && pillars.carriedTo() == cell; // edit mode 3 = moving a pillar
        const gfx::PictureId picture = m_interior.surfaces().pillarPicture();
        if ((!cursor && !carried) || !picture.isValid()) {
            return;
        }
        const gfx::Picture& data = canvas.pictures().picture(picture);
        const core::Point at = camera.toScreen(HousePillars::worldOf(cell.x, cell.y));
        drawClipped(canvas, picture, {at.x - data.width() / 2, at.y - data.height() / 2 + kPictureDrop}, {0, 0, data.width(), data.height()});
    }

    void HousePainter::updatePillarEdit(gfx::Canvas& canvas, const Camera& camera, const std::optional<core::Point>& mouse) {
        HousePillars& pillars = m_interior.pillars();
        pillars.setMarked({});
        pillars.setLit({});
        if (!mouse || !m_interior.inFloorRoom() || pillars.editMode() == 0) {
            return;
        }
        if (pillars.editMode() == 1) {
            const HouseGrid::Cell cell = HouseGrid::screenToCell(camera.toMap(*mouse));
            const core::Point at{cell.x - HouseGrid::kGridColumn + 1, cell.y - HouseGrid::kGridRow + 1};
            if (at.x >= 1 && at.x <= HousePillars::kColumns && at.y >= 1 && at.y <= HousePillars::kRows) {
                pillars.setMarked(at);
            }
            return;
        }
        const gfx::PictureId picture = m_interior.surfaces().pillarPicture();
        if (!picture.isValid() || pillars.carried() != core::Point{}) {
            return;
        }
        const gfx::Picture& data = canvas.pictures().picture(picture);
        for (int y = HousePillars::kRows; y >= 1; --y) {
            for (int x = HousePillars::kColumns; x >= 1; --x) {
                if (!pillars.has(x, y, m_interior.room())) {
                    continue;
                }
                const core::Point at = camera.toScreen(HousePillars::worldOf(x, y));
                const core::Rect area{at.x - data.width() / 2, at.y - data.height() / 2 + kPictureDrop, data.width(), data.height()};
                if (area.contains(*mouse) && data.isOpaque(mouse->x - area.x, mouse->y - area.y)) {
                    pillars.setLit({x, y});
                    return;
                }
            }
        }
    }

    bool HousePainter::pillarLit(int column, int row) const {
        if (!m_light || m_light->rowCol.x == 0 || m_light->rowCol.y == 0) {
            return false;
        }
        const core::Point at = m_light->rowCol;
        if (column == at.x && row == at.y) {
            return true;
        }
        const HomeFurniture::Span span = m_interior.furniture().spanOf(m_light->itemId, m_light->direction).value_or(HomeFurniture::Span{});
        if (span.x <= 1 && span.y <= 1) {
            return false;
        }
        if (span.x >= span.y) {
            return row == at.y && column >= at.x && column <= at.x + span.x - 1;
        }
        return column == at.x && row >= at.y && row <= at.y + span.y - 1;
    }

    void HousePainter::paintAnimation(
        gfx::Canvas& canvas,
        const Camera& camera,
        int slot,
        int column,
        int row,
        std::uint64_t nowMs
    ) {
        HouseUtensil& utensil = m_interior.utensils().utensil(slot);
        if (!utensil.animPicture.isValid() || utensil.animState == 0) {
            return;
        }
        if ((utensil.animCell.x > 1 || utensil.animCell.y > 1) && (column != utensil.animCell.x || row != utensil.animCell.y)) {
            return;
        }
        const core::Point at = camera.toScreen(utensil.world) + utensil.animOffset;
        if (utensil.animFrame > utensil.animFrames - 1) {
            utensil.animFrame = 0;
            if (utensil.animState == 1) {
                utensil.animState = 0;
            }
            if (utensil.animState == 4) { // animation state 4 steps back to state 3 when the frames wrap
                utensil.animState = 3;
            }
        }
        const core::Rect frame{0, utensil.animHeight * utensil.animFrame, utensil.animWidth, utensil.animHeight};
        drawClipped(canvas, utensil.animPicture, at, frame);
        if (m_hovered == slot && utensil.itemId != kLift) {
            const gfx::Picture& data = canvas.pictures().picture(utensil.animPicture);
            const core::Rect inside = frame.intersected({0, 0, data.width(), data.height()});
            if (!inside.isEmpty()) {
                canvas.drawPictureLight(utensil.animPicture, {at.x + inside.x - frame.x, at.y + inside.y - frame.y}, kHoverLight, inside);
            }
        }
        if (nowMs >= utensil.animMs + kFrameMs) {
            if (utensil.animState != 0 && utensil.animState != 3) { // animation states 0 and 3 are resting, others advance the frame
                ++utensil.animFrame;
            }
            utensil.animMs = nowMs;
        }
    }

    core::Rect HousePainter::stripOf(int itemId, int column, int row) {
        const int width = column == 1 && row == 1 ? 2 * kStripWidth : kStripWidth;
        if (column < row) {
            if (itemId == kWindmill && column == 1 && row == 3) {
                return {kStripMiddle - (row - column + 1) * kStripWidth, 0, 2 * kStripWidth, kStripHeight};
            }
            return {kStripMiddle - (row - column) * kStripWidth, 0, width, kStripHeight};
        }
        if (column > row) {
            return {(column - row + 1) * kStripWidth + kStripMiddle, 0, width, kStripHeight};
        }
        return {(column - row) * kStripWidth + kStripMiddle, 0, width, kStripHeight};
    }

    void HousePainter::paintDragged(gfx::Canvas& canvas, const Camera& camera, int column, int row) {
        const core::Point at = camera.toScreen(HouseGrid::cellToScreen(m_drag->drop.x, m_drag->drop.y, m_drag->drop.z));
        core::Rect source{0, 0, m_drag->size.x, m_drag->size.y};
        core::Point place{at.x - m_drag->size.x / 2, at.y - m_drag->size.y / 2 + kPictureDrop};
        if (column != 0 && row != 0) {
            source = stripOf(m_drag->itemId, column, row);
            place.x += source.x;
        }
        drawClipped(canvas, m_drag->direction == 1 && m_drag->turned.isValid() ? m_drag->turned : m_drag->picture, place, source);
    }

    void HousePainter::paintUtensil(
        gfx::Canvas& canvas,
        const Camera& camera,
        int slot,
        int column,
        int row,
        std::uint64_t nowMs
    ) {
        const HouseUtensil& utensil = m_interior.utensils().utensil(slot);
        if (!utensil.shown || m_interior.furniture().piece(utensil.furnIndex).parent != 0) {
            return;
        }
        if (!utensil.picture.isValid()) {
            if (utensil.itemId == FurnitureCarCases::kShowPlatformItem) {
                paintExhibit(canvas, camera, slot);
            }
            return;
        }
        const core::Rect area = m_interior.utensils().pictureArea(slot, camera.origin());
        core::Rect source{0, 0, area.width, area.height};
        core::Point at{area.x, area.y};
        if (column != 0 && row != 0) {
            source = stripOf(utensil.itemId, column, row);
            at.x += source.x;
        }
        drawClipped(canvas, utensil.picture, at, source);
        if (utensil.overlayPicture.isValid() && utensil.furnIndex == m_sprayBath && column != 0 && row != 0 && column != row) {
            constexpr int kStrip = 30; // px: strip width
            constexpr int kMiddle = 150; // px: x of the strip edge where the left and right halves meet
            constexpr int kHeight = 300; // px: strip height
            const core::Rect cut = column > row ? core::Rect{(column - row) * kStrip + kMiddle, 0, kStrip, kHeight}
                                                : core::Rect{kMiddle - (row - column) * kStrip - kStrip, 0, kStrip, kHeight};
            drawClipped(canvas, utensil.overlayPicture, {area.x + cut.x, area.y}, cut);
        }
        paintAnimation(canvas, camera, slot, column, row, nowMs);
        if (m_drag && m_drag->slot == slot) {
            const gfx::Picture& data = canvas.pictures().picture(utensil.picture);
            const core::Rect inside = source.intersected({0, 0, data.width(), data.height()});
            if (!inside.isEmpty()) {
                canvas.drawPictureLight(utensil.picture, {at.x + inside.x - source.x, at.y + inside.y - source.y}, kDraggedLight, inside);
            }
            return;
        }
        if (m_hovered == slot && utensil.itemId != kLift) {
            const gfx::Picture& data = canvas.pictures().picture(utensil.picture);
            const core::Rect inside = source.intersected({0, 0, data.width(), data.height()});
            if (!inside.isEmpty()) {
                canvas.drawPictureLight(utensil.picture, {at.x + inside.x - source.x, at.y + inside.y - source.y}, kHoverLight, inside);
            }
        }
        paintShelfItems(canvas, camera, utensil);
        if (utensil.itemId == FurnitureCarCases::kShowPlatformItem) {
            paintExhibit(canvas, camera, slot);
        }
        paintTraining(canvas, camera, slot);
    }

    void HousePainter::paintTimeBar(gfx::Canvas& canvas, core::Point at, int percent) const {
        gfx::PictureLibrary& pictures = canvas.pictures();
        if (const auto frame = pictures.find("Panel_makeThingTimeForm_1")) {
            const gfx::Picture& data = pictures.picture(*frame);
            canvas.drawPicture(*frame, {at.x - data.width() / 2, at.y - data.height() / 2});
        }
        if (const auto fill = pictures.find("Panel_makeThingTime_1")) {
            const gfx::Picture& data = pictures.picture(*fill);
            const int width = static_cast<int>(std::ceil(data.width() * (percent / 100.0))); // percent to fraction
            canvas.drawPicture(*fill, {at.x - data.width() / 2, at.y - data.height() / 2 - 1}, core::Rect{0, 0, width, data.height()});
        }
    }

    void HousePainter::paintTraining(gfx::Canvas& canvas, const Camera& camera, int slot) {
        const HouseUtensil& utensil = m_interior.utensils().utensil(slot);
        HouseTraining& training = m_interior.training();
        const core::Point origin = camera.origin();
        const core::Point at{utensil.world.x - origin.x, utensil.world.y - origin.y};
        constexpr int kBarDrop = 40; // px: time bar sits this far below the machine
        if (utensil.itemId == kTreadmill) {
            if (training.ownerUsed() == utensil.furnIndex) {
                if (m_trainingPet) {
                    m_trainingPet(canvas, camera, utensil.furnIndex);
                }
                if (training.bar() >= 0) {
                    paintTimeBar(canvas, {at.x, at.y + kBarDrop}, training.bar());
                }
            }
            return;
        }
        if (utensil.itemId != kBicepExerciser) {
            return;
        }
        const bool inUse = training.ownerUsed() == utensil.furnIndex;
        if (inUse && training.bar() >= 0) {
            paintTimeBar(canvas, {at.x, at.y + kBarDrop}, training.bar());
        }
        const auto record = m_items.find(utensil.itemId);
        const int rotation = m_interior.furniture().piece(utensil.furnIndex).rotation;
        const bool even = !record || (rotation == 0 ? record->furnPicture0 : record->furnPicture1) % 2 == 0;
        gfx::PictureLibrary& pictures = canvas.pictures();
        const int phase = training.phase();
        const auto part = [&](const char* name, int dx, int dy, int top) {
            if (const auto picture = pictures.find(name)) {
                const gfx::Picture& data = pictures.picture(*picture);
                canvas.drawPicture(
                    *picture,
                    {at.x - data.width() / 2 + dx, at.y - data.height() / 2 + dy},
                    core::Rect::fromEdges(0, top, data.width(), data.height())
                );
            }
        };
        constexpr int kBarTop = 20; // px: source top row of the bar picture at rest
        if (even) {
            part("Sport_bar_1", -24, -53, inUse ? kBarTop - phase : kBarTop); // pixel offset of the bar (x -24, y -53)
            part("Sport_poise_1", 15, -4, inUse ? phase : 0); // pixel offset of the poise (x 15, y -4)
        } else {
            part("Sport_bar_2", 24, -53, inUse ? kBarTop - phase : kBarTop); // pixel offset of the bar (x 24, y -53)
            part("Sport_poise_2", -16, -4, inUse ? phase : 0); // pixel offset of the poise (x -16, y -4)
        }
    }

    gfx::PictureId HousePainter::exhibitPicture(gfx::Canvas& canvas) const {
        int itemId = m_interior.exhibit().itemId();
        if (VehicleKinds::isCapsule(itemId)) {
            if (const auto record = m_items.find(itemId)) {
                itemId = record->aliasItemId;
            }
        }
        const auto record = m_items.find(itemId);
        return record ? canvas.pictures().find(std::to_string(record->iconId)).value_or(gfx::PictureId{}) : gfx::PictureId{};
    }

    core::Point HousePainter::exhibitCorner(const gfx::Picture& picture, core::Point floorPoint) const {
        int offsetX = 0;
        int offsetY = 0;
        for (const auto& row : kExhibitOffsets) {
            if (row[0] == m_interior.exhibit().itemId()) {
                offsetX = row[1];
                offsetY = row[2];
            }
        }
        const int excess = picture.height() - 105; // height in pixels above which the picture rises
        return {floorPoint.x - picture.width() / 2 + offsetX, floorPoint.y - picture.height() / 2 + kPictureDrop - excess / 2 + offsetY};
    }

    void HousePainter::paintExhibit(gfx::Canvas& canvas, const Camera& camera, int slot) {
        if (m_interior.exhibit().itemId() == 0) {
            return;
        }
        const gfx::PictureId icon = exhibitPicture(canvas);
        if (!icon.isValid()) {
            return;
        }
        const gfx::Picture& picture = canvas.pictures().picture(icon);
        const core::Point origin = camera.origin();
        const core::Point world = m_interior.utensils().utensil(slot).world;
        const core::Point corner = exhibitCorner(picture, {world.x - origin.x, world.y - origin.y});
        canvas.drawPicture(icon, corner);
        if (m_exhibitHovered) {
            canvas.drawPictureLight(icon, corner, kHoverLight, core::Rect{0, 0, picture.width(), picture.height()});
        }
    }

    core::Rect HousePainter::areaOf(const StallInFurItem& item, const Camera& camera) {
        const core::Point origin = camera.origin();
        return {
            item.world.x - origin.x - item.size.width / 2,
            item.world.y - origin.y - item.size.height / 2 + kPictureDrop,
            item.size.width,
            item.size.height
        };
    }

    core::Rect HousePainter::shelfItemArea(int number, const Camera& camera) const { return areaOf(m_interior.stalls().shelf(number), camera); }

    core::Rect HousePainter::caseItemArea(int number, const Camera& camera) const { return areaOf(m_interior.stalls().displayed(number), camera); }

    void HousePainter::paintStallItem(gfx::Canvas& canvas, const StallInFurItem& item, const core::Rect& area, bool hovered) const {
        constexpr int kIcon = 32; // px: item icon side
        canvas.drawPicture(item.picture, {area.x, area.y}, core::Rect{0, 0, kIcon, kIcon});
        if (hovered) {
            canvas.drawPictureLight(item.picture, {area.x, area.y}, kHoverLight, core::Rect{0, 0, kIcon, kIcon});
        }
    }

    void HousePainter::paintShelfItems(gfx::Canvas& canvas, const Camera& camera, const HouseUtensil& utensil) {
        const int kind = FurnitureKinds::shelfKind(m_items, utensil.itemId);
        if (kind == kShelfKind) {
            for (int place = 1; place <= kShelfPlaces; ++place) {
                const int number = utensil.contentSlots[static_cast<std::size_t>(place)];
                const StallInFurItem& item = m_interior.stalls().shelf(number);
                if (number != 0 && item.itemId != 0 && item.picture.isValid()) {
                    paintStallItem(canvas, item, shelfItemArea(number, camera), m_hoveredItem == number);
                }
            }
        }
        if (kind == kDisplayCaseKind) {
            for (int place = 1; place <= kCasePlaces; ++place) {
                const int number = utensil.contentSlots[static_cast<std::size_t>(place)];
                const StallInFurItem& item = m_interior.stalls().displayed(number);
                if (number != 0 && item.itemId != 0 && item.picture.isValid()) {
                    paintStallItem(canvas, item, caseItemArea(number, camera), m_hoveredCase == number);
                }
            }
        }
    }

    void HousePainter::paintItemLabel(gfx::Canvas& canvas, const StallInFurItem& item, core::Point origin) const {
        const gfx::TextStyle green{kLabelColor, gfx::TextEffect::Plain, std::nullopt};
        const auto record = m_items.find(item.itemId);
        if (!record) {
            return;
        }
        protocol::ItemInstance shown = item.item;
        shown.itemId = item.itemId;
        const std::string label = stallItemName(m_items, shown) + " x " + std::to_string(item.count);
        const int x = item.world.x - static_cast<int>(label.size()) * kCharWidth / 2 - origin.x;
        const int y = item.world.y - record->furnSizeX * kCellSide - origin.y;
        const std::string price = std::to_string(item.price);
        // 200: text box width in pixels of the label lines
        canvas.drawText({x, y - kItemLabelRise}, label, green, 200);
        canvas.drawText({x, y}, "Price", green, 200);
        canvas.drawText({x + kPriceGap, y}, price, {priceColor(price.size()), gfx::TextEffect::Plain, std::nullopt}, 200);
    }

    void HousePainter::paintLabels(gfx::Canvas& canvas, const Camera& camera) const {
        const core::Point origin = camera.origin();
        const gfx::TextStyle green{kLabelColor, gfx::TextEffect::Plain, std::nullopt};
        if (m_hoveredItem != 0) {
            paintItemLabel(canvas, m_interior.stalls().shelf(m_hoveredItem), origin);
            return;
        }
        if (m_hoveredCase != 0) {
            paintItemLabel(canvas, m_interior.stalls().displayed(m_hoveredCase), origin);
            return;
        }
        if (m_hovered == 0) {
            return;
        }
        const HouseUtensil& utensil = m_interior.utensils().utensil(m_hovered);
        const auto record = m_items.find(utensil.itemId);
        if (!record) {
            return;
        }
        const bool turned = m_interior.furniture().piece(utensil.furnIndex).rotation == 1;
        const int across = (turned ? record->furnSizeY : record->furnSizeX) * kCellSide;
        const int deep = (turned ? record->furnSizeX : record->furnSizeY) * kCellSide;
        core::Point at{
            utensil.world.x - across + (across + deep - static_cast<int>(record->name.size()) * kCharWidth) / 2 - origin.x,
            utensil.world.y - origin.y - record->furnSizeZ * kCellSide - kLabelRise
        };
        if (utensil.itemId == kWindmill) {
            at.x += kWindmillLabelShift.x;
            at.y += kWindmillLabelShift.y;
        }
        std::string label = record->name;
        if (Sandglass::isSandglassItem(m_items, utensil.itemId)) {
            label += Sandglass::remainingLabel(record->maxUses, m_interior.furniture().piece(utensil.furnIndex).usedCount);
        }
        canvas.drawText(at, label, green, static_cast<int>(label.size()) * kCharWidth);
    }

    void HousePainter::paintCells(gfx::Canvas& canvas, const Camera& camera, const std::vector<HouseEntry>& entries, std::uint64_t nowMs) {
        const HomeFurniture& furniture = m_interior.furniture();
        const HouseUtensils& utensils = m_interior.utensils();
        const int room = m_interior.room();
        for (const int slot : utensils.carpets()) {
            if (slot != 0 && furniture.piece(utensils.utensil(slot).furnIndex).room == room) {
                paintUtensil(canvas, camera, slot, 0, 0, nowMs);
            }
        }
        const HouseRoomRect& area = furniture.room(room);
        std::array<bool, HomeFurniture::kMost + 1> done{};
        std::vector<bool> drawn(entries.size(), false);
        bool dragDrawn = false;
        for (int y = area.top; y <= kLastRow; ++y) {
            for (int x = area.left; x <= kLastColumn; ++x) {
                for (int z = 1; z <= HomeFurniture::kLevels; ++z) {
                    const int held = furniture.houseCell(x, y, z, room);
                    if (held >= 1 && held <= HomeFurniture::kMost) {
                        const FurnitureRecord& record = furniture.piece(held);
                        const int slot = utensils.slotOfFurniture(held);
                        if (record.room == room && !done[static_cast<std::size_t>(held)] && slot != 0) {
                            const auto span = furniture.spanOf(record.itemId, record.rotation);
                            if (span && (span->x > 1 || span->y > 1)) {
                                const int column = x - record.cellX + 1;
                                const int row = y - record.cellY + 1;
                                if (record.cellZ == z && column > 0 && row > 0) {
                                    const bool onFirstColumn = record.cellX == x && record.cellY <= y && record.cellY + span->y - 1 >= y;
                                    const bool onFirstRow = record.cellY == y && record.cellX <= x && record.cellX + span->x - 1 >= x;
                                    if (onFirstColumn || onFirstRow) {
                                        paintUtensil(canvas, camera, slot, column, row, nowMs);
                                    }
                                    if (record.cellX + span->x - 1 == x && record.cellY + span->y - 1 == y) {
                                        done[static_cast<std::size_t>(held)] = true;
                                    }
                                }
                            } else {
                                paintUtensil(canvas, camera, slot, 0, 0, nowMs);
                                done[static_cast<std::size_t>(held)] = true;
                            }
                        }
                    } else if (held == HomeFurniture::kPillarCell && z == 1) {
                        paintPillar(canvas, camera, x, y);
                    }
                    if (z == 1) {
                        paintPillarEdit(canvas, camera, x, y);
                    }
                    if (m_drag && !dragDrawn && (m_drag->drop.x != 0 || m_drag->drop.y != 0 || m_drag->drop.z != 0)) {
                        const auto span = furniture.spanOf(m_drag->itemId, m_drag->direction).value_or(HomeFurniture::Span{});
                        const HouseGrid::Cell& drop = m_drag->drop;
                        if (x >= drop.x && x <= drop.x + span.x - 1 && y >= drop.y && y <= drop.y + span.y - 1 && drop.z <= z) {
                            paintDragged(canvas, camera, x - drop.x + 1, y - drop.y + 1);
                            dragDrawn = x == drop.x + span.x - 1 && y == drop.y + span.y - 1 && z == drop.z + span.z - 1;
                        }
                    }
                    for (std::size_t index = 0; index < entries.size(); ++index) {
                        if (!drawn[index] && entries[index].cell.x == x && entries[index].cell.y == y) {
                            drawn[index] = true;
                            entries[index].draw();
                        }
                    }
                }
            }
        }
    }

    void HousePainter::updateHover(gfx::Canvas& canvas, const Camera& camera, const std::optional<core::Point>& mouse) {
        m_hovered = 0;
        m_hoveredItem = 0;
        m_hoveredCase = 0;
        m_exhibitHovered = false;
        if (!mouse || !m_interior.inFloorRoom()) {
            return;
        }
        if (m_interior.exhibit().itemId() != 0 && m_interior.exhibit().cell() == 0) {
            const gfx::PictureId icon = exhibitPicture(canvas);
            for (int slot = HouseUtensils::kMost; slot >= 1 && icon.isValid(); --slot) {
                const HouseUtensil& platform = m_interior.utensils().utensil(slot);
                if (platform.itemId != FurnitureCarCases::kShowPlatformItem) {
                    continue;
                }
                const gfx::Picture& picture = canvas.pictures().picture(icon);
                const core::Rect area{
                    platform.world.x - camera.origin().x - picture.width() / 2,
                    platform.world.y - camera.origin().y - picture.height() / 2 + kPictureDrop,
                    picture.width(),
                    picture.height()
                };
                m_exhibitHovered = area.contains(*mouse) && picture.isOpaque(mouse->x - area.x, mouse->y - area.y);
                break;
            }
        }
        m_hoveredItem = shelfItemAt(canvas.pictures(), camera, *mouse);
        if (m_hoveredItem == 0) {
            m_hoveredCase = caseItemAt(canvas.pictures(), camera, *mouse);
        }
        if (m_hoveredItem != 0 || m_hoveredCase != 0) {
            return;
        }
        m_hovered = pieceSlotAt(canvas.pictures(), camera, *mouse);
    }

    int HousePainter::shelfItemAt(const gfx::PictureLibrary& pictures, const Camera& camera, core::Point mouse) const {
        for (int number = 1; number <= FurnitureStalls::kShelfItems; ++number) {
            const StallInFurItem& item = m_interior.stalls().shelf(number);
            if (item.itemId == 0 || !item.picture.isValid() || m_interior.furniture().piece(item.furnIndex).room != m_interior.room()) {
                continue;
            }
            const core::Rect area = shelfItemArea(number, camera);
            if (area.contains(mouse) && pictures.picture(item.picture).isOpaque(mouse.x - area.x, mouse.y - area.y)) {
                return number;
            }
        }
        return 0;
    }

    int HousePainter::caseItemAt(const gfx::PictureLibrary& pictures, const Camera& camera, core::Point mouse) const {
        for (int number = 1; number <= FurnitureStalls::kCases; ++number) {
            const StallInFurItem& item = m_interior.stalls().displayed(number);
            if (item.itemId == 0 || !item.picture.isValid() || m_interior.furniture().piece(item.furnIndex).room != m_interior.room()) {
                continue;
            }
            const core::Rect area = caseItemArea(number, camera);
            if (area.contains(mouse) && pictures.picture(item.picture).isOpaque(mouse.x - area.x, mouse.y - area.y)) {
                return number;
            }
        }
        return 0;
    }

    int HousePainter::pieceSlotAt(const gfx::PictureLibrary& pictures, const Camera& camera, core::Point mouse) const {
        const HomeFurniture& furniture = m_interior.furniture();
        const int room = m_interior.room();
        const auto hit = [&](int slot) {
            const core::Rect area = m_interior.utensils().pictureArea(slot, camera.origin());
            if (!m_interior.utensils().utensil(slot).shown || !area.contains(mouse)) {
                return false;
            }
            const gfx::Picture& data = pictures.picture(m_interior.utensils().utensil(slot).picture);
            return data.isOpaque(mouse.x - area.x, mouse.y - area.y);
        };
        std::array<bool, HomeFurniture::kMost + 1> seen{};
        const HouseRoomRect& area = furniture.room(room);
        if (furniture.count() > 0) {
            for (int z = HomeFurniture::kLevels; z >= 1; --z) {
                for (int y = area.bottom; y >= area.top; --y) {
                    for (int x = area.right; x >= area.left; --x) {
                        const int held = furniture.houseCell(x, y, z, room);
                        if (held >= 1 && held <= HomeFurniture::kMost && !seen[static_cast<std::size_t>(held)]) {
                            seen[static_cast<std::size_t>(held)] = true;
                            const int slot = m_interior.utensils().slotOfFurniture(held);
                            if (slot != 0 && !(m_drag && m_drag->slot == slot) && hit(slot)) {
                                return slot;
                            }
                        }
                    }
                }
            }
        }
        for (const int slot : m_interior.utensils().carpets()) {
            if (slot != 0 && furniture.piece(m_interior.utensils().utensil(slot).furnIndex).room == room && hit(slot)) {
                return slot;
            }
        }
        return 0;
    }

}
