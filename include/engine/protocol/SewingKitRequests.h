#ifndef NORTH_ISLAND_PROTOCOL_SEWING_KIT_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_SEWING_KIT_REQUESTS_H

#include "core/Bytes.h"

namespace protocol {

    /// @brief Builds request 23/105 sub 1: use a Sewing Tools kit to give one piece the look of another.
    /// @param kitSlot Bag place of the kit.
    /// @param equipSlot Bag place of the piece whose attributes stay.
    /// @param poseSlot Bag place of the piece whose look it takes.
    /// @return The encoded request payload.
    core::Bytes sewingKitRequest(int kitSlot, int equipSlot, int poseSlot);

}

#endif
