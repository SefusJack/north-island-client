#include "protocol/NoticeRequests.h"

#include "core/ByteWriter.h"

namespace protocol {

    namespace {

        constexpr std::uint8_t kBoards = 82; // action 82: notice boards
        constexpr int kGuildOffset = 6; // the guild board's sub ids are the public board's plus 6

        /// @brief Gives the board kind byte.
        /// @param guild True for the guild's board.
        /// @return 2 for the guild's board, 1 for the public one.
        std::uint8_t kindOf(bool guild) { return guild ? 2 : 1; }

        /// @brief Builds a notice request that carries an id, page and date (open or delete).
        /// @param sub Public board sub code; the guild's is this plus six.
        /// @param guild True for the guild's board.
        /// @param id Notice id.
        /// @param page Page the notice is on.
        /// @param date Notice date.
        /// @return The encoded payload.
        core::Bytes noticeRequest(
            std::uint8_t sub,
            bool guild,
            std::int32_t id,
            std::int32_t page,
            double date
        ) {
            core::ByteWriter writer;
            writer.u8(kBoards).u8(static_cast<std::uint8_t>(sub + (guild ? kGuildOffset : 0))).i32(id).i32(page).f64(date).u8(kindOf(guild));
            return writer.take();
        }

    }

    core::Bytes postNoticeRequest(bool guild, const std::string& subject, const std::string& text) {
        core::ByteWriter writer;
        writer.u8(kBoards).u8(guild ? 7 : 1).longText(subject).longText(text).u8(kindOf(guild)); // sub 7: guild post, 1: public post
        return writer.take();
    }

    core::Bytes noticePageRequest(bool guild, int page) {
        core::ByteWriter writer;
        writer.u8(kBoards).u8(guild ? 8 : 2).u8(static_cast<std::uint8_t>(page)).u8(kindOf(guild)); // sub 8: guild page, 2: public page
        return writer.take();
    }

    core::Bytes openNoticeRequest(bool guild, std::int32_t id, std::int32_t page, double date) {
        return noticeRequest(3, guild, id, page, date); // sub 3 (guild 9): open a notice
    }

    core::Bytes deleteNoticeRequest(bool guild, std::int32_t id, std::int32_t page, double date) {
        return noticeRequest(4, guild, id, page, date); // sub 4 (guild 10): delete a notice
    }

    core::Bytes mayPostNoticeRequest(bool guild) {
        core::ByteWriter writer;
        writer.u8(kBoards).u8(guild ? 11 : 5).u8(kindOf(guild)); // sub 11: guild may-post, 5: public may-post
        return writer.take();
    }

    core::Bytes mailNoticeAuthorRequest(std::int32_t ownId, const std::string& author, const std::string& text) {
        core::ByteWriter writer;
        // sub 6: mail to the notice's author
        writer.u8(kBoards).u8(6).i32(ownId).shortText(author).text(text);
        return writer.take();
    }

}
