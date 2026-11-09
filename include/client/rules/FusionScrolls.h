#ifndef NORTH_ISLAND_CLIENT_RULES_FUSION_SCROLLS_H
#define NORTH_ISLAND_CLIENT_RULES_FUSION_SCROLLS_H

namespace client {

    /// @brief Tells whether an item is a scroll that raises the fusion level of a piece of gear: the Seraph 34238, Demon 34239, Cherubim 34243, Ofanim 34244, Virtues 34245, Principality 34246, Small Demon 34247 and Archangel 34255 scrolls.
    /// @details Original: IsReinforceItem, AC_Item.pas:1905.
    /// @param itemId Item id.
    /// @return True for a fusion scroll.
    bool isFusionScroll(int itemId);

}

#endif
