#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION74_GPS_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION74_GPS_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 74, the GPS items and the Dreaming Pill's notices.
    /// @details Original: RecvGpsMarkSlot, RecvGpsEnterEffectReset, HandleDreamingPillPacket.
    /// 74/1 (u8 point, u16 map, u16 x, u16 y): a recorded place, all three come at login.
    /// 74/2 (i32 player): another player's arrival light plays again; the port draws none, so nothing is done.
    /// 74/3 (u8 kind: 1 then u16 minutes left, 3 expired, 4 in use): a short message.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction74Gps(const core::Bytes& payload, MessageContext& context);

}

#endif
