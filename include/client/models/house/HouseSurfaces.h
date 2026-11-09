#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_SURFACES_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_SURFACES_H

#include "formats/ItemTable.h"
#include "gfx/PictureId.h"
#include "gfx/PictureLibrary.h"

namespace client {

    /// @brief The floor, wall and pillar items of a house room and the pictures they are drawn with.
    /// @details Original: DZ_Thing.pas TUtensilManage.FMoveState: SetFloorItem, SetWallItem. The floor is one picture laid in blocks of 4 x 3 cells.
    class HouseSurfaces {
        public:
            /// @brief Creates the surfaces with no floor and wall item.
            /// @param pictures Picture library.
            /// @param items Item table.
            HouseSurfaces(
                gfx::PictureLibrary& pictures,
                const formats::ItemTable& items
            )
                : m_pictures(pictures),
                  m_items(items) {}

            /// @brief Sets the floor item.
            /// @details Original: server action 62/14, SetFloorItem (0 or the default floor 39062: none).
            /// @param itemId The floor item id.
            void setFloorItem(int itemId);

            /// @brief Sets the wall item and the pillar that goes with it.
            /// @details Original: server action 62/15, SetWallItem (0: none, with the default pillar 39063).
            /// @param itemId The wall item id.
            void setWallItem(int itemId);

            /// @brief Forgets the floor and wall item (their pictures stay).
            /// @details Original: ClearAllUtensils.
            void clear();

            /// @brief The floor item id.
            /// @return Item id, 0 for none.
            int floorItem() const { return m_floorItem; }

            /// @brief The wall item id.
            /// @return Item id, 0 for none.
            int wallItem() const { return m_wallItem; }

            /// @brief The floor picture.
            /// @return The picture, invalid for none.
            gfx::PictureId floorPicture() const { return m_floorPicture; }

            /// @brief The wall picture along the top edge of the room (the item's first facing).
            /// @return The picture.
            gfx::PictureId topWallPicture() const { return m_topWallPicture; }

            /// @brief The wall picture along the left edge of the room.
            /// @return The picture.
            gfx::PictureId leftWallPicture() const { return m_leftWallPicture; }

            /// @brief The pillar picture of the wall set; the default pillar (39063, FMoveState.W[2] from the start) until a wall item says otherwise.
            /// @return The picture.
            gfx::PictureId pillarPicture() const;

        private:
            /// @brief The picture of a picture number.
            /// @param number The picture number.
            /// @return The picture, invalid when missing.
            gfx::PictureId pictureOf(int number) const;

            gfx::PictureLibrary& m_pictures;
            const formats::ItemTable& m_items;
            int m_floorItem = 0;
            int m_wallItem = 0;
            gfx::PictureId m_floorPicture;
            gfx::PictureId m_topWallPicture;
            gfx::PictureId m_leftWallPicture;
            gfx::PictureId m_pillarPicture;
    };

}

#endif
