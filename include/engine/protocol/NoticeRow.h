#ifndef NORTH_ISLAND_PROTOCOL_NOTICE_ROW_H
#define NORTH_ISLAND_PROTOCOL_NOTICE_ROW_H

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief A row of a notice board's page.
    /// @details Original: 82/1, 82/11 (RecvNoticeList): i32 len + author's name, i32 len + subject, f64 date, i32 author id, i32 notice id.
    struct NoticeRow {
        std::string author;
        std::string subject;
        double date = 0;
        std::int32_t authorId = 0;
        std::int32_t id = 0;
    };

}

#endif
