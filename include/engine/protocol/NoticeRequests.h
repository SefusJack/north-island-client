#ifndef NORTH_ISLAND_PROTOCOL_NOTICE_REQUESTS_H
#define NORTH_ISLAND_PROTOCOL_NOTICE_REQUESTS_H

#include "core/Bytes.h"

#include <cstdint>
#include <string>

namespace protocol {

    /// @brief Builds 82/1 (public) or 82/7 (guild): post a notice.
    /// @param guild True for the guild's board.
    /// @param subject Subject.
    /// @param text Text.
    /// @return The encoded payload.
    core::Bytes postNoticeRequest(bool guild, const std::string& subject, const std::string& text);

    /// @brief Builds 82/2 (public) or 82/8 (guild): ask for a page of the board.
    /// @param guild True for the guild's board.
    /// @param page Page number.
    /// @return The encoded payload.
    core::Bytes noticePageRequest(bool guild, int page);

    /// @brief Builds 82/3 (public) or 82/9 (guild): open a notice.
    /// @param guild True for the guild's board.
    /// @param id Notice id.
    /// @param page Page the notice is on.
    /// @param date Notice date.
    /// @return The encoded payload.
    core::Bytes openNoticeRequest(bool guild, std::int32_t id, std::int32_t page, double date);

    /// @brief Builds 82/4 (public) or 82/10 (guild): delete a notice.
    /// @param guild True for the guild's board.
    /// @param id Notice id.
    /// @param page Page the notice is on.
    /// @param date Notice date.
    /// @return The encoded payload.
    core::Bytes deleteNoticeRequest(bool guild, std::int32_t id, std::int32_t page, double date);

    /// @brief Builds 82/5 (public) or 82/11 (guild): ask whether posting is allowed.
    /// @param guild True for the guild's board.
    /// @return The encoded payload.
    core::Bytes mayPostNoticeRequest(bool guild);

    /// @brief Builds 82/6: a mail written to a notice's author.
    /// @param ownId Own player id.
    /// @param author The author's name.
    /// @param text Mail text.
    /// @return The encoded payload.
    core::Bytes mailNoticeAuthorRequest(std::int32_t ownId, const std::string& author, const std::string& text);

}

#endif
