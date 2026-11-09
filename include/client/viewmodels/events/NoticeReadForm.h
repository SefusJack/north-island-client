#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_NOTICE_READ_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_NOTICE_READ_FORM_H

#include "client/services/GameServices.h"
#include "client/models/events/NoticeBoard.h"
#include "protocol/NoticeContent.h"
#include "protocol/NoticeRow.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief Form that shows the subject and text of a notice, with Delete and Send Mail.
    /// @details Original: NoticeReadForm.xml, TReadNoticeForm / TOrgReadNoticeForm.
    class NoticeReadForm {
        public:
            /// @brief Callbacks into the rest of the game that the form needs.
            struct Hooks {
                std::function<std::int32_t()> ownId;
                std::function<bool()> inGuildLeadership;
                std::function<void(bool guild, std::int32_t authorId, const std::string& author)> mailAuthor;
            };

            /// @brief Loads the layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param publicBoard The public notice board model.
            /// @param guildBoard The guild notice board model.
            /// @param hooks Callbacks into the game.
            NoticeReadForm(
                GameServices& services,
                ui::Screen& screen,
                NoticeBoard& publicBoard,
                NoticeBoard& guildBoard,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Remembers the picked row and puts the form away until the notice arrives.
            /// @details Original: OpenNotice.
            /// @param guild True for the guild board, false for the public one.
            /// @param row The row picked on the board.
            void expect(bool guild, const protocol::NoticeRow& row);

            /// @brief Shows the arrived notice.
            /// @details Original: RecvNoticeContent and ShowCtrl.
            /// @param guild True for the guild board, false for the public one.
            /// @param content Subject, text and author of the notice.
            void show(bool guild, const protocol::NoticeContent& content);

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Deletes the opened notice and closes the form.
            /// @details Original: DeleteNotice.
            void deleteNotice();

            /// @brief Opens the mail form for the notice's author.
            /// @details Original: MailNoticeAuthor.
            void mailAuthor();

            /// @brief Shows or hides the Delete and Send Mail buttons (and their grey twins) for the viewer.
            /// @details Original: ShowCtrl.
            void layoutButtons();

            ui::Screen& m_screen;
            NoticeBoard& m_publicBoard;
            NoticeBoard& m_guildBoard;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_subject;
            ui::Memo& m_text;
            bool m_guild = false;
            std::int32_t m_rowAuthorId = 0;
            std::string m_author;
            std::int32_t m_noticeAuthorId = 0;
    };

}

#endif
