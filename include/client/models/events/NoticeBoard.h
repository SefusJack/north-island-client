#ifndef NORTH_ISLAND_CLIENT_MODELS_EVENTS_NOTICE_BOARD_H
#define NORTH_ISLAND_CLIENT_MODELS_EVENTS_NOTICE_BOARD_H

#include "core/Bytes.h"
#include "protocol/NoticePageInfo.h"
#include "protocol/NoticeRow.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief Model of a notice board (public or guild): rows, paging and the picked notice.
    /// @details Original: TNoticeBoardForm / TOrgNoticeForm without their windows; the public board has ten rows a page and
    /// twenty-five pages, the guild's five and twenty.
    class NoticeBoard {
        public:
            static constexpr int kMostRows = 10; // rows a page of the public board (the guild's has five)

            /// @brief Callbacks the board needs.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<void(const std::string&, int)> say;
            };

            /// @brief Creates a board.
            /// @param guild True for the guild's board, false for the public one.
            /// @param hooks Send and message callbacks.
            NoticeBoard(
                bool guild,
                Hooks hooks
            );

            /// @brief Rows a page holds.
            /// @return 5 for the guild board, 10 for the public one.
            int rowsPerPage() const { return m_guild ? 5 : kMostRows; } // rows per page on the guild board

            /// @brief A row of the page; an empty one has no author.
            /// @param number Row number (1..10).
            /// @return The row.
            const protocol::NoticeRow& row(int number) const { return m_rows[static_cast<std::size_t>(number - 1)]; }

            /// @brief The page shown.
            /// @return Page number.
            int currentPage() const { return m_currentPage; }

            /// @brief How many pages there are.
            /// @return Page count.
            int pageCount() const { return m_pageCount; }

            /// @brief Empties all rows.
            /// @details Original: ClearNoticeList.
            void clearRows();

            /// @brief Replaces the rows of the page with the ones received.
            /// @details Original: RecvNoticeList.
            /// @param rows Rows received (at most ten are kept).
            void setRows(const std::vector<protocol::NoticeRow>& rows);

            /// @brief Stores the current page and works out the page count from the notice count.
            /// @details Original: RecvNoticePageInfo / RecvOrgNoticePageInfo.
            /// @param info Page number and total notice count.
            void setPageInfo(const protocol::NoticePageInfo& info);

            /// @brief Goes back to the page asked before (server answers 82/3 and 82/13).
            void restoreAskedPage();

            /// @brief Empties the list and asks for the first page.
            /// @details Original: TNoticeBoardForm.ShowCtrl.
            void reopen();

            /// @brief Asks for the first page, keeping the rows as they are.
            /// @details Original: ShowOrgNoticePage / RequestOrgNotices.
            void askFirstPage();

            /// @brief Empties the list and asks for the first page again.
            /// @details Original: RefreshNoticeBoard / RefreshOrgNoticeBoard.
            void refresh();

            /// @brief Asks for the previous page.
            /// @details Original: PrevNoticePage.
            void previous();

            /// @brief Asks for the next page, or says "Last Page" on the last.
            /// @details Original: NextNoticePage.
            void next();

            /// @brief Picks a row and asks the server for the notice when the row holds one.
            /// @details Original: HandleMouseUp / HandleDblClick.
            /// @param number Row picked (1..10).
            /// @return The row opened, or nothing when it is empty or out of range.
            std::optional<protocol::NoticeRow> open(int number);

            /// @brief Asks whether a notice may be posted (the public board wants level 10).
            /// @details Original: RequestAddNotice / AddOrgNoticeClick.
            /// @param ownLevel Level of the own player.
            void askToPost(int ownLevel);

            /// @brief Sends a new notice.
            /// @details Original: SubmitNotice.
            /// @param subject Subject line.
            /// @param text Body text.
            void post(const std::string& subject, const std::string& text);

            /// @brief Deletes the notice opened last.
            /// @details Original: DeleteNotice.
            void deleteOpened();

            /// @brief Resets the paging state to the first page.
            /// @details Original: ResetOrgNoticePaging.
            void resetPaging();

        private:
            /// @brief Sends the request for the current page.
            void requestPage();

            bool m_guild;
            Hooks m_hooks;
            std::array<protocol::NoticeRow, kMostRows> m_rows;
            int m_page = 1;
            int m_pageCount = 0;
            int m_currentPage = 0;
            int m_previousPage = 1;
            std::int32_t m_selectedId = 0;
            double m_selectedDate = 0;
    };

}

#endif
