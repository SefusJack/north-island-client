#ifndef NORTH_ISLAND_PROTOCOL_NOTICE_CONTENT_H
#define NORTH_ISLAND_PROTOCOL_NOTICE_CONTENT_H

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief An opened notice: subject, text and author id.
    /// @details Original: 82/2, 82/12 (RecvNoticeContent): i32 len + author's name (not read), i32 len + subject, f64 date (not read),
    /// i32 text length, i32 author id, the text.
    struct NoticeContent {
        std::string subject;
        std::string text;
        std::int32_t authorId = 0;
    };

}

#endif
