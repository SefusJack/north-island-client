#include "protocol/NoticeLists.h"

#include "core/ByteReader.h"

namespace protocol {

    namespace {

        constexpr int kMostRows = 10; // rows per notice board page

    }

    std::vector<NoticeRow> decodeNoticeRows(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skip action and sub
        std::vector<NoticeRow> rows;
        while (static_cast<int>(rows.size()) < kMostRows && reader.remaining() > 0) {
            NoticeRow row;
            row.author = reader.longText();
            row.subject = reader.longText();
            row.date = reader.f64();
            row.authorId = reader.i32();
            row.id = reader.i32();
            if (reader.failed() || row.author.empty()) {
                break;
            }
            rows.push_back(row);
        }
        return rows;
    }

    std::optional<NoticeContent> decodeNoticeContent(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skip action and sub
        NoticeContent content;
        reader.longText();
        content.subject = reader.longText();
        reader.f64();
        const std::int32_t length = reader.i32();
        content.authorId = reader.i32();
        content.text = reader.text(length > 0 ? static_cast<std::size_t>(length) : 0);
        if (reader.failed()) {
            return std::nullopt;
        }
        return content;
    }

    std::optional<NoticePageInfo> decodeNoticePageInfo(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // skip action and sub
        NoticePageInfo info;
        info.count = reader.i32();
        info.page = reader.u8();
        if (reader.failed()) {
            return std::nullopt;
        }
        return info;
    }

}
