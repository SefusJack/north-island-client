#ifndef NORTH_ISLAND_CLIENT_RENDERING_HOUSE_HOUSE_PAINTER_H
#define NORTH_ISLAND_CLIENT_RENDERING_HOUSE_HOUSE_PAINTER_H

#include "client/models/house/HouseDrag.h"
#include "client/models/house/HouseEntry.h"
#include "client/models/house/HouseInterior.h"
#include "client/models/house/HouseWallLight.h"
#include "client/models/world/Camera.h"
#include "core/Point.h"
#include "core/Rect.h"
#include "formats/ItemTable.h"
#include "gfx/Canvas.h"
#include "gfx/PictureId.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace client {

    /// @brief Draws the inside of a house's floor room: floor, walls, furniture, pillars and the people standing on each cell.
    /// @details Original: DZ_Thing.pas DrawRoomFloorAndWalls, DrawUtensil, DrawPillar and the house branch of DZ_Human.pas DrawSceneObjects.
    class HousePainter {
        public:
            /// @brief Creates a painter over a house interior and the item table.
            /// @param interior The house interior state to draw (and to update hover state in).
            /// @param items Item table used for furniture sizes, names and pictures.
            HousePainter(
                HouseInterior& interior,
                const formats::ItemTable& items
            )
                : m_interior(interior),
                  m_items(items) {}

            /// @brief Draws the floor and wall pictures under everything else.
            /// @details Original: DrawRoomFloorAndWalls.
            /// @param canvas Target canvas.
            /// @param camera Camera used for world to screen conversion.
            void paintFloorAndWalls(gfx::Canvas& canvas, const Camera& camera);

            /// @brief Draws the room cell by cell from the back with furniture, pillars and the entries standing on each cell.
            /// @param canvas Target canvas.
            /// @param camera Camera used for world to screen conversion.
            /// @param entries People (or other things) to draw, each at the cell where it stands.
            /// @param nowMs Current time in milliseconds, drives furniture animation.
            void paintCells(gfx::Canvas& canvas, const Camera& camera, const std::vector<HouseEntry>& entries, std::uint64_t nowMs);

            /// @brief Finds the shelf item, case item or furniture piece under the mouse.
            /// @details Original: UpdateUtensilHover.
            /// @param canvas Canvas whose picture library is used for hit tests.
            /// @param camera Camera used for world to screen conversion.
            /// @param mouse Screen point of the mouse, none when it is off the map.
            void updateHover(gfx::Canvas& canvas, const Camera& camera, const std::optional<core::Point>& mouse);

            /// @brief While the owner edits pillars, marks the cell or lights the pillar under the mouse.
            /// @details Original: UpdatePillarHover.
            /// @param canvas Canvas whose picture library is used for hit tests.
            /// @param camera Camera used for world to screen conversion.
            /// @param mouse Screen point of the mouse, none when it is off the map.
            void updatePillarEdit(gfx::Canvas& canvas, const Camera& camera, const std::optional<core::Point>& mouse);

            /// @brief The utensil slot under the mouse.
            /// @return Slot number, 0 for none.
            int hoveredSlot() const { return m_hovered; }

            /// @brief The number of the shelf item under the mouse.
            /// @return Shelf item number, 0 for none.
            int hoveredShelfItem() const { return m_hoveredItem; }

            /// @brief The number of the display case item under the mouse.
            /// @return Display case item number, 0 for none.
            int hoveredCaseItem() const { return m_hoveredCase; }

            /// @brief Finds the shelf item at a screen point.
            /// @details Original: part of UpdateUtensilHover.
            /// @param pictures Picture library for opacity tests.
            /// @param camera Camera used for world to screen conversion.
            /// @param mouse Screen point.
            /// @return Shelf item number, 0 for none.
            int shelfItemAt(const gfx::PictureLibrary& pictures, const Camera& camera, core::Point mouse) const;

            /// @brief Finds the display case item at a screen point.
            /// @details Original: part of UpdateUtensilHover.
            /// @param pictures Picture library for opacity tests.
            /// @param camera Camera used for world to screen conversion.
            /// @param mouse Screen point.
            /// @return Display case item number, 0 for none.
            int caseItemAt(const gfx::PictureLibrary& pictures, const Camera& camera, core::Point mouse) const;

            /// @brief Finds the furniture piece at a screen point, searching the front rows first.
            /// @details Original: part of UpdateUtensilHover.
            /// @param pictures Picture library for opacity tests.
            /// @param camera Camera used for world to screen conversion.
            /// @param mouse Screen point.
            /// @return Utensil slot, 0 for none.
            int pieceSlotAt(const gfx::PictureLibrary& pictures, const Camera& camera, core::Point mouse) const;

            /// @brief Where a shelf item lies on the screen.
            /// @param number Shelf item number.
            /// @param camera Camera used for world to screen conversion.
            /// @return Screen rectangle of the item.
            core::Rect shelfItemArea(int number, const Camera& camera) const;

            /// @brief Where a display case item lies on the screen.
            /// @param number Display case item number.
            /// @param camera Camera used for world to screen conversion.
            /// @return Screen rectangle of the item.
            core::Rect caseItemArea(int number, const Camera& camera) const;

            /// @brief Draws the name of the hovered piece above it, or name, count and price of the hovered shelf item.
            /// @details Original: DrawUtensilName and DrawStallItemLabel.
            /// @param canvas Target canvas.
            /// @param camera Camera used for world to screen conversion.
            void paintLabels(gfx::Canvas& canvas, const Camera& camera) const;

            /// @brief Sets the piece being carried (drawn where it would stand, its place lit).
            /// @param drag The carried piece, none when nothing is carried.
            void setDrag(const std::optional<HouseDrag>& drag) { m_drag = drag; }

            /// @brief Sets the wall pieces and pillars that a carried wall piece lights.
            /// @param light The light description, none when no wall piece is carried.
            void setWallLight(const std::optional<HouseWallLight>& light) { m_light = light; }

            /// @brief Sets the spray tub piece the owner bathes in; its front is drawn over the bather.
            /// @param furnIndex Furniture index of the tub, 0 for none.
            void setSprayBath(int furnIndex) { m_sprayBath = furnIndex; }

            /// @brief Sets the callback that draws the pet the owner trains on the Treadmill.
            /// @param painter Callback taking the canvas, camera and the furniture index of the piece.
            void setTrainingPetPainter(std::function<void(gfx::Canvas&, const Camera&, int furnIndex)> painter) { m_trainingPet = std::move(painter); }

        private:
            /// @brief Draws one furniture piece, whole or as the strip of one cell, with its overlays.
            /// @details Original: DrawUtensil.
            /// @param canvas Target canvas.
            /// @param camera Camera used for world to screen conversion.
            /// @param slot Utensil slot of the piece.
            /// @param column Cell column within the piece (0 with row 0: the whole picture).
            /// @param row Cell row within the piece.
            /// @param nowMs Current time in milliseconds.
            void paintUtensil(
                gfx::Canvas& canvas,
                const Camera& camera,
                int slot,
                int column,
                int row,
                std::uint64_t nowMs
            );

            /// @brief Draws the items on the shelves or in the display case of a piece.
            /// @details Original: DrawUtensil's loops over its places.
            /// @param canvas Target canvas.
            /// @param camera Camera used for world to screen conversion.
            /// @param utensil The shelf or display case piece.
            void paintShelfItems(gfx::Canvas& canvas, const Camera& camera, const HouseUtensil& utensil);

            /// @brief Draws one stall item (the first 32 x 32 of its picture), lit when hovered.
            /// @details Original: DrawStallFurItem.
            /// @param canvas Target canvas.
            /// @param item The item to draw.
            /// @param area Screen area of the item.
            /// @param hovered True when the mouse is over the item.
            void paintStallItem(gfx::Canvas& canvas, const StallInFurItem& item, const core::Rect& area, bool hovered) const;

            /// @brief Draws "<name> x <count>" over "Price" and the price in its colour.
            /// @details Original: DrawStallItemLabel.
            /// @param canvas Target canvas.
            /// @param item The item to label.
            /// @param origin Camera origin subtracted from world positions.
            void paintItemLabel(gfx::Canvas& canvas, const StallInFurItem& item, core::Point origin) const;

            /// @brief Where an item lies on the screen.
            /// @param item The stall item.
            /// @param camera Camera providing the origin.
            /// @return Screen rectangle of the item.
            static core::Rect areaOf(const StallInFurItem& item, const Camera& camera);

            /// @brief Draws the vehicle on show over a Show Platform, lit when the mouse is over it.
            /// @details Original: DrawExhibitItem.
            /// @param canvas Target canvas.
            /// @param camera Camera used for world to screen conversion.
            /// @param slot Utensil slot of the platform.
            void paintExhibit(gfx::Canvas& canvas, const Camera& camera, int slot);

            /// @brief Finds the picture of the vehicle on show (a capsule shows the vehicle it holds).
            /// @param canvas Canvas whose picture library is searched.
            /// @return The picture id, invalid when there is none.
            gfx::PictureId exhibitPicture(gfx::Canvas& canvas) const;

            /// @brief Where the first pixel of the vehicle on show lies when the platform is at a floor point.
            /// @details Original: ItemDrawOffsetTable.
            /// @param picture The vehicle's picture.
            /// @param floorPoint The platform's floor point on screen.
            /// @return Top-left screen corner of the picture.
            core::Point exhibitCorner(const gfx::Picture& picture, core::Point floorPoint) const;

            /// @brief Draws the percent bar over a piece the owner is at, and the Bicep Exerciser's bar and poise.
            /// @details Original: DrawExhibitItem's Bicep Exerciser branch and the Treadmill's bar.
            /// @param canvas Target canvas.
            /// @param camera Camera used for world to screen conversion.
            /// @param slot Utensil slot of the piece.
            void paintTraining(gfx::Canvas& canvas, const Camera& camera, int slot);

            /// @brief Draws the empty bar centred on a point and the part of its fill the percent says.
            /// @details Original: DrawMakeTimeBar.
            /// @param canvas Target canvas.
            /// @param at Screen point the bar is centred on.
            /// @param percent Fill in percent.
            void paintTimeBar(gfx::Canvas& canvas, core::Point at, int percent) const;

            /// @brief Draws the animation frame of a piece at its offset from the floor point and advances the frame.
            /// @details Original: DrawUtensilAnim.
            /// @param canvas Target canvas.
            /// @param camera Camera used for world to screen conversion.
            /// @param slot Utensil slot of the piece.
            /// @param column Cell column within the piece.
            /// @param row Cell row within the piece.
            /// @param nowMs Current time in milliseconds.
            void paintAnimation(
                gfx::Canvas& canvas,
                const Camera& camera,
                int slot,
                int column,
                int row,
                std::uint64_t nowMs
            );

            /// @brief Draws the pillar of the wall set in a cell.
            /// @details Original: DrawPillar.
            /// @param canvas Target canvas.
            /// @param camera Camera used for world to screen conversion.
            /// @param cellX Cell column on the room's grid.
            /// @param cellY Cell row on the room's grid.
            void paintPillar(gfx::Canvas& canvas, const Camera& camera, int cellX, int cellY);

            /// @brief Draws the add-pillar cursor or the pillar picked up while moving, at a cell.
            /// @details Original: DrawPillarCursor.
            /// @param canvas Target canvas.
            /// @param camera Camera used for world to screen conversion.
            /// @param cellX Cell column on the room's grid.
            /// @param cellY Cell row on the room's grid.
            void paintPillarEdit(gfx::Canvas& canvas, const Camera& camera, int cellX, int cellY);

            /// @brief Tells whether the carried wall piece lights the pillar of a cell.
            /// @param column Pillar column.
            /// @param row Pillar row.
            /// @return True when the pillar is lit.
            bool pillarLit(int column, int row) const;

            /// @brief Draws the carried piece's strip for a cell (the whole picture for 0, 0).
            /// @details Original: DrawDragItem.
            /// @param canvas Target canvas.
            /// @param camera Camera used for world to screen conversion.
            /// @param column Cell column within the piece.
            /// @param row Cell row within the piece.
            void paintDragged(gfx::Canvas& canvas, const Camera& camera, int column, int row);

            /// @brief The strip of a piece's picture that belongs to a cell.
            /// @param itemId Item id of the piece.
            /// @param column Cell column within the piece.
            /// @param row Cell row within the piece.
            /// @return Source rectangle in the piece's picture.
            static core::Rect stripOf(int itemId, int column, int row);

            /// @brief Draws the part of a picture inside its bounds, placed where its top-left corner would be.
            /// @param canvas Target canvas.
            /// @param picture Picture to draw.
            /// @param at Destination of the source rectangle's top-left corner.
            /// @param source Source rectangle, clipped to the picture.
            static void drawClipped(gfx::Canvas& canvas, gfx::PictureId picture, core::Point at, const core::Rect& source);

            HouseInterior& m_interior;
            const formats::ItemTable& m_items;
            int m_hovered = 0;
            int m_sprayBath = 0;
            int m_hoveredItem = 0;
            int m_hoveredCase = 0;
            bool m_exhibitHovered = false;
            std::function<void(gfx::Canvas&, const Camera&, int)> m_trainingPet;
            std::optional<HouseDrag> m_drag;
            std::optional<HouseWallLight> m_light;
    };

}

#endif
