#include "client/models/house/HouseSurfaces.h"

#include <string>

namespace client {

    namespace {

        constexpr int kNoFloor = 39062; // item 39062: Wood Floor, the default floor (drawn as no floor picture)
        constexpr int kDefaultPillar = 39063; // item 39063: Pillar, the default pillar

        /// @brief The pillar item that goes with a wall item.
        /// @details Original: SetWallItem's pillar table.
        /// @param wallItem The wall item id.
        /// @return The pillar item id (the default pillar for a wall not in the table).
        int pillarOfWall(int wallItem) {
            switch (wallItem) {
                case 39041: return 39037; // Wallpaper -> Pillar
                case 39042: return 39038; // Wallpaper -> Pillar
                case 39043: return 39039; // Wallpaper -> Pillar
                case 39044: return 39040; // Wallpaper -> Pillar
                case 39064: return 39063; // Wallpaper -> Pillar (the default pillar)
                case 39128: return 39127; // Wallpaper -> Pillar
                case 39130: return 39129; // Wallpaper -> Pillar
                case 39136: return 39135; // Wallpaper -> Pillar
                case 39138: return 39137; // Wallpaper -> Pillar
                case 39140: return 39139; // Wallpaper -> Pillar
                case 39142: return 39141; // Wallpaper -> Pillar
                case 39144: return 39143; // Wallpaper -> Pillar
                case 39169: return 39168; // Wallpaper -> Pillar
                case 39171: return 39170; // Wallpaper -> Pillar
                case 39174: return 39173; // Sky Wallpaper -> Pillar
                case 39176: return 39175; // Snowflake Wallpaper -> Pillar
                case 39181: return 39180; // Wallpaper -> Pillar
                case 39193: return 39192; // Wallpaper -> Pillar
                case 39195: return 39194; // Wallpaper -> Pillar
                case 39197: return 39196; // Wallpaper -> Pillar
                case 39199: return 39198; // Wallpaper -> Pillar
                case 39213: return 39212; // Snowy Wallpaper -> Pillar
                default: return kDefaultPillar; // any other wall uses the default pillar
            }
        }

    }

    gfx::PictureId HouseSurfaces::pictureOf(int number) const { return m_pictures.find(std::to_string(number)).value_or(gfx::PictureId{}); }

    gfx::PictureId HouseSurfaces::pillarPicture() const {
        if (m_pillarPicture.isValid()) {
            return m_pillarPicture;
        }
        const auto pillar = m_items.find(kDefaultPillar);
        return pillar ? pictureOf(pillar->furnPicture0) : gfx::PictureId{};
    }

    void HouseSurfaces::setFloorItem(int itemId) {
        m_floorItem = itemId;
        if (itemId == 0 || itemId == kNoFloor) {
            m_floorPicture = {};
        } else if (const auto item = m_items.find(itemId)) {
            m_floorPicture = pictureOf(item->furnPicture0);
        }
    }

    void HouseSurfaces::setWallItem(int itemId) {
        m_wallItem = itemId;
        int pillarItem = kDefaultPillar;
        if (itemId == 0) {
            m_topWallPicture = {};
            m_leftWallPicture = {};
        } else if (const auto item = m_items.find(itemId)) {
            m_topWallPicture = pictureOf(item->furnPicture0);
            m_leftWallPicture = pictureOf(item->furnPicture1);
            pillarItem = pillarOfWall(itemId);
        }
        if (const auto pillar = m_items.find(pillarItem)) {
            m_pillarPicture = pictureOf(pillar->furnPicture0);
        }
    }

    void HouseSurfaces::clear() {
        m_floorItem = 0;
        m_wallItem = 0;
    }

}
