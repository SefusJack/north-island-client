#ifndef NORTH_ISLAND_CLIENT_DATA_EQUIP_DRAW_ORDER_H
#define NORTH_ISLAND_CLIENT_DATA_EQUIP_DRAW_ORDER_H

#include <array>

namespace client {

    struct CharacterLook;

    /// @brief Gives the order a figure's layers are drawn in after the body.
    /// @details Original: GetEquipDrawSlot and its table of 20 rows (EquipDrawSlotOrder): the equipment slot 1..6 of each layer, 0 for the head.
    /// The row follows the weapon kind of the clothes and, for the special-costume looks (2332..2335 and the like), the facing. The manteau looks
    /// (2347 and the like, GetEquipDrawSlotX347) draw behind the body facing away, beside it for some frames of the side poses and in front of it otherwise, so they also read the frame (FAnimFrame).
    /// @param look The figure's look.
    /// @param direction The figure's animation slot (THuman.FDirection).
    /// @param frame The animation frame (FAnimFrame).
    /// @return The seven layers in draw order.
    std::array<int, 7> equipDrawOrder(const CharacterLook& look, int direction, int frame = 0);

    /// @brief Tells whether an equipment slot is drawn for a look.
    /// @details Original: CanDrawEquipSlot; the cape look 2331 (3331, 4331, 5331 by body type) hides the headgear, clothes, slot 4 and shoes.
    /// @param look The figure's look.
    /// @param slot Equipment slot (1..6).
    /// @return False when the look hides the slot.
    bool canDrawEquipSlot(const CharacterLook& look, int slot);

}

#endif
