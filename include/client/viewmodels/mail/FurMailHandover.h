#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MAIL_FUR_MAIL_HANDOVER_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MAIL_FUR_MAIL_HANDOVER_H

#include "core/Point.h"

namespace client {

    /// @brief What the by-name mail form and its reader pass each other when one takes the other's place: at a mail box or not, and where the old form was.
    /// @details Original: ShowCtrl of TFurMailBoxForm and TReadFurMailBoxForm.
    struct FurMailHandover {
        bool atMailbox = false;
        core::Point position;
    };

}

#endif
