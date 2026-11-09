#ifndef NORTH_ISLAND_PROTOCOL_PET_INFO_BLOCK_READER_H
#define NORTH_ISLAND_PROTOCOL_PET_INFO_BLOCK_READER_H

#include "core/ByteReader.h"
#include "protocol/PetInfoBlock.h"

namespace protocol {

    /// @brief Reads the 86-byte pet info block.
    /// @details Original: TPetBlk_718: u16 kind at 0, i32 HP at 23, u16 SP, Str, Con, Int, Wis and Agi at 27 to 37, u8 level at 39, u8 amity at 47.
    /// @param reader Reader positioned at the block; advanced past it.
    /// @return The decoded block.
    PetInfoBlock readPetInfoBlock(core::ByteReader& reader);

}

#endif
