#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_GROUND_ITEMS_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_GROUND_ITEMS_H

#include "client/models/world/Camera.h"
#include "core/Point.h"
#include "core/Rect.h"
#include "formats/ItemTable.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"
#include "protocol/GroundItemRecord.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>

namespace client {

    /// @brief The items and piles of gold lying on the map, by index (1..255).
    /// @details Original: GdThings, TGdThingManage. Each shows its icon (a large item the middle of its picture, gold one of five piles
    /// by the amount) centred on its place; the one under the mouse lights up and names itself; weapons, armour (item types 1..14)
    /// and gold glint now and then.
    class GroundItems {
        public:
            /// @brief One thing on the ground.
            struct Item {
                bool gold = false;
                int itemId = 0;
                std::int32_t amount = 0;
                core::Point position;
                std::int32_t extra = 0;
                gfx::PictureId picture;
                gfx::PictureId flightPicture;
                bool active = true;
                core::Rect source;
                bool pixelHit = false;
                bool glints = false;
            };

            /// @brief Creates the ground item store.
            /// @param pictures Picture library for icons.
            /// @param items Item table.
            GroundItems(
                gfx::PictureLibrary& pictures,
                const formats::ItemTable& items
            )
                : m_pictures(pictures),
                  m_items(items) {}

            /// @brief Replaces everything on the map with the server's list.
            /// @details Original: RecvGdThingList.
            /// @param records The ground items.
            void setList(const std::vector<protocol::GroundItemRecord>& records);

            /// @brief Adds an item into the first free index.
            /// @details Original: RecvGdThingAdd.
            /// @param itemId Item id.
            /// @param position Map position.
            /// @param extra Extra value (also added to the vertical sort order).
            /// @return The index, 0 when none is free.
            int addItem(int itemId, core::Point position, std::int32_t extra);

            /// @brief Adds a pile of gold into the first free index.
            /// @details Original: RecvGdMoneyAdd.
            /// @param amount Gold amount.
            /// @param position Map position.
            /// @return The index, 0 when none is free.
            int addGold(std::int32_t amount, core::Point position);

            /// @brief Sets whether a thing shows and takes the mouse (not while it flies in).
            /// @param index Thing index.
            /// @param active True once it has landed.
            void setActive(int index, bool active);

            /// @brief Removes a thing.
            /// @details Original: RemoveGdThing.
            /// @param index Thing index.
            /// @return What was there, nothing if the index was empty.
            std::optional<Item> remove(int index);

            /// @brief Removes everything.
            void clear() { m_things.clear(); }

            /// @brief The things by index.
            /// @return Read-only map of index to thing.
            const std::map<int, Item>& things() const { return m_things; }

            /// @brief Finds the thing under a screen point (the last drawn wins).
            /// @param screenPoint Screen point.
            /// @param camera The camera.
            /// @return The index, 0 for nothing.
            int indexAt(core::Point screenPoint, const Camera& camera) const;

            /// @brief Gives where a thing's picture lies on the screen.
            /// @param item The thing.
            /// @param camera The camera.
            /// @return The screen rectangle.
            core::Rect areaOf(const Item& item, const Camera& camera) const;

            /// @brief Draws a thing, lit when hovered and glinting for weapons, armour and gold.
            /// @details Original: TGdThing.DrawControl.
            /// @param canvas Target canvas.
            /// @param item The thing.
            /// @param camera The camera.
            /// @param hovered Whether the mouse is on it.
            /// @param nowMs Current time in ms.
            void draw(
                gfx::Canvas& canvas,
                const Item& item,
                const Camera& camera,
                bool hovered,
                std::uint64_t nowMs
            ) const;

            /// @brief Draws the name (or the gold amount) beside the mouse.
            /// @details Original: DrawGdThingHint.
            /// @param canvas Target canvas.
            /// @param index Thing index.
            /// @param mouse Mouse position.
            void drawHint(gfx::Canvas& canvas, int index, core::Point mouse) const;

        private:
            /// @brief Stores a thing at an index, setting its picture and visible part.
            /// @details Original: InitGdItem / InitGdMoney.
            /// @param index Thing index.
            /// @param item The thing.
            void place(int index, Item item);

            /// @brief Finds the first free index.
            /// @return The index, 0 when none is free.
            int freeIndex() const;

            /// @brief Gives the screen position of a thing's top-left corner.
            /// @param item The thing.
            /// @param camera The camera.
            /// @return The screen point.
            core::Point topLeft(const Item& item, const Camera& camera) const;

            gfx::PictureLibrary& m_pictures;
            const formats::ItemTable& m_items;
            std::map<int, Item> m_things;
    };

}

#endif
