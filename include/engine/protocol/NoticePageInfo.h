#ifndef NORTH_ISLAND_PROTOCOL_NOTICE_PAGE_INFO_H
#define NORTH_ISLAND_PROTOCOL_NOTICE_PAGE_INFO_H

#include <cstdint>

namespace protocol {

    /// @brief How many notices a board holds and the page the rows are of.
    /// @details Original: 82/3, 82/4, 82/13, 82/14 (RecvNoticePageInfo): i32 count, u8 page.
    struct NoticePageInfo {
        std::int32_t count = 0;
        int page = 0;
    };

}

#endif
