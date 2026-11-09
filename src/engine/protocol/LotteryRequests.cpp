#include "protocol/LotteryRequests.h"

namespace protocol {

    core::Bytes lotteryDrawRequest() { return {104, 1}; } // action 104 (online draw), sub 1: draw animation over

}
