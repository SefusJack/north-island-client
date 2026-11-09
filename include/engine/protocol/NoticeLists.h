#ifndef NORTH_ISLAND_PROTOCOL_NOTICE_LISTS_H
#define NORTH_ISLAND_PROTOCOL_NOTICE_LISTS_H

#include "core/Bytes.h"
#include "protocol/NoticeContent.h"
#include "protocol/NoticePageInfo.h"
#include "protocol/NoticeRow.h"

#include <optional>
#include <vector>

namespace protocol {

    /// @brief Decodes a page of notice rows (server 82/1, 82/11): up to ten rows, to the end of the packet or to a row with no author.
    /// @param payload The packet payload.
    /// @return The rows.
    std::vector<NoticeRow> decodeNoticeRows(const core::Bytes& payload);

    /// @brief Decodes an opened notice (server 82/2, 82/12).
    /// @param payload The packet payload.
    /// @return The content, or nothing when malformed.
    std::optional<NoticeContent> decodeNoticeContent(const core::Bytes& payload);

    /// @brief Decodes the notice count and page (server 82/3, 82/4, 82/13, 82/14).
    /// @param payload The packet payload.
    /// @return The page info, or nothing when malformed.
    std::optional<NoticePageInfo> decodeNoticePageInfo(const core::Bytes& payload);

}

#endif
