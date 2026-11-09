#ifndef NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_SCALED_ITEM_ICON_H
#define NORTH_ISLAND_CLIENT_RENDERING_PAINTERS_SCALED_ITEM_ICON_H

#include "client/services/GameServices.h"
#include "core/Rect.h"
#include "core/Size.h"
#include "gfx/PictureId.h"

#include <optional>

namespace client {

    /// @brief An item's icon as the manufacture lists show it in an area: a small item's picture as it is; a large item's middle part, shrunk to fit.
    /// @details Original: RE_Menu.pas LoadMakeThingTree and ShowMakeThingRecipe; DrawItemIconInRect for the large item's middle part.
    struct ScaledItemIcon {
        gfx::PictureId picture;
        core::Size size;
        std::optional<core::Rect> source;
    };

    /// @brief Works out how an item's icon is drawn in an area.
    /// @param services Game services (items, pictures).
    /// @param itemId Item id.
    /// @param area Size of the area the icon must fit.
    /// @return The picture, the size to draw it at and the part of it to show.
    ScaledItemIcon scaledItemIcon(GameServices& services, int itemId, core::Size area);

}

#endif
