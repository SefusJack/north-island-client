#ifndef NORTH_ISLAND_CLIENT_ACTIONS_ACTION80_POINT_STORE_H
#define NORTH_ISLAND_CLIENT_ACTIONS_ACTION80_POINT_STORE_H

#include "client/actions/MessageContext.h"
#include "core/Bytes.h"

namespace client {

    /// @brief Handles server action 80, the point store.
    /// @details Original: TEightTwelveForm, DZ_Connect.pas action 80.
    /// 80/1 (u8 n, n x u16 item, i32 price, u8 count): the rows, which open the form.
    /// 80/2, 80/7, 80/8: not enough points, not enough points, the server is busy.
    /// 80/3 (u16 item, i32 points spent, u8 count): a line bought.
    /// 80/21 to 80/23, 80/31, 80/41: fill forms nothing shows or flags nothing reads; accepted and ignored.
    /// 80/4 (u16 item, i32 points, u8 count), 80/5 and 80/6 (i32 points): the points taken.
    /// The other arms of the action (the Obtain list, the birth check) are not built.
    /// @param payload Whole message payload.
    /// @param context Message context with the event sink.
    void handleAction80PointStore(const core::Bytes& payload, MessageContext& context);

}

#endif
