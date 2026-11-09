#ifndef NORTH_ISLAND_CLIENT_MODELS_ITEMS_PUPPET_STATE_H
#define NORTH_ISLAND_CLIENT_MODELS_ITEMS_PUPPET_STATE_H

#include <cstdint>

namespace client {

    /// @brief What the Memory Card's two forms share: training mode, capacity step and the pending cancel requester.
    /// @details Original: TPlayer.FPuppetTraining and FMemCardCapacity, TMemCardForm1.FCancelRequesterId.
    struct PuppetState {
        int mode = 0;
        int capacityStep = 0;
        std::int32_t cancelRequesterId = 0;
    };

}

#endif
