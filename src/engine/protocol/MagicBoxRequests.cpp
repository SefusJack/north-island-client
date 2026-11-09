#include "protocol/MagicBoxRequests.h"

namespace protocol {

    core::Bytes magicBoxRequest(int boxSlot, int pieceSlot) {
        return {23, 105, 2, static_cast<std::uint8_t>(boxSlot), static_cast<std::uint8_t>(pieceSlot)}; // action 23, sub 105 (Rebirth Box), mode 2
    }

}
