#include "client/models/events/NoticeBoard.h"

#include "protocol/NoticeRequests.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kLastPublicPage = 25; // highest page number of the public board
        constexpr int kLastGuildPage = 20; // highest page number of the guild board
        constexpr int kMinPostLevel = 10; // lowest player level that may post on the public board
        constexpr int kMessageMs = 1300; // message display time, ms

    }

    NoticeBoard::NoticeBoard(
        bool guild,
        Hooks hooks
    )
        : m_guild(guild),
          m_hooks(std::move(hooks)) {
        if (guild) {
            resetPaging();
        }
    }

    void NoticeBoard::clearRows() { m_rows.fill({}); }

    void NoticeBoard::setRows(const std::vector<protocol::NoticeRow>& rows) {
        clearRows();
        for (std::size_t i = 0; i < rows.size() && i < m_rows.size(); ++i) {
            m_rows[i] = rows[i];
        }
    }

    void NoticeBoard::setPageInfo(const protocol::NoticePageInfo& info) {
        const int perPage = rowsPerPage();
        m_pageCount = info.count / perPage + (info.count % perPage != 0 ? 1 : 0);
        if (m_pageCount == 0) {
            m_pageCount = 1;
        }
        m_currentPage = info.page;
        m_page = info.page;
    }

    void NoticeBoard::restoreAskedPage() {
        m_page = m_previousPage;
        m_currentPage = m_previousPage;
    }

    void NoticeBoard::requestPage() { m_hooks.send(protocol::noticePageRequest(m_guild, m_page)); }

    void NoticeBoard::reopen() {
        clearRows();
        askFirstPage();
    }

    void NoticeBoard::askFirstPage() {
        m_page = 1;
        requestPage();
    }

    void NoticeBoard::refresh() {
        clearRows();
        m_page = 1;
        m_currentPage = 1;
        requestPage();
    }

    void NoticeBoard::previous() {
        m_previousPage = m_page;
        m_page = std::max(1, m_page - 1);
        requestPage();
    }

    void NoticeBoard::next() {
        if (m_guild ? m_page >= m_pageCount : m_page > m_pageCount) {
            m_hooks.say("Last Page", kMessageMs);
            return;
        }
        m_previousPage = m_page;
        m_page = std::min(m_page + 1, m_guild ? kLastGuildPage : kLastPublicPage);
        requestPage();
    }

    std::optional<protocol::NoticeRow> NoticeBoard::open(int number) {
        if (number < 1 || number > kMostRows) {
            return std::nullopt;
        }
        const protocol::NoticeRow& picked = row(number);
        m_selectedId = picked.id;
        m_selectedDate = picked.date;
        if (picked.author.empty()) {
            return std::nullopt;
        }
        m_hooks.send(protocol::openNoticeRequest(m_guild, m_selectedId, m_currentPage, m_selectedDate));
        return picked;
    }

    void NoticeBoard::askToPost(int ownLevel) {
        if (!m_guild && ownLevel < kMinPostLevel) {
            m_hooks.say("Players whose level less than 10 can't leave any message here.", kMessageMs);
            return;
        }
        m_hooks.send(protocol::mayPostNoticeRequest(m_guild));
    }

    void NoticeBoard::post(const std::string& subject, const std::string& text) { m_hooks.send(protocol::postNoticeRequest(m_guild, subject, text)); }

    void NoticeBoard::deleteOpened() { m_hooks.send(protocol::deleteNoticeRequest(m_guild, m_selectedId, m_currentPage, m_selectedDate)); }

    void NoticeBoard::resetPaging() {
        m_page = 1;
        m_pageCount = 1;
        m_currentPage = 0;
        m_previousPage = 1;
    }

}
