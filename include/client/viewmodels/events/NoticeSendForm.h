#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_NOTICE_SEND_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_NOTICE_SEND_FORM_H

#include "client/services/GameServices.h"
#include "client/models/events/NoticeBoard.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

namespace client {

    /// @brief Form to write a notice for the public or guild board.
    /// @details Original: NoticeSendForm.xml, TSendNoticeForm / TOrgSendNoticeForm; the server opens it with 82/8.
    class NoticeSendForm {
        public:
            /// @brief Loads the layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param messages Sink for short player messages.
            /// @param publicBoard The public notice board model.
            /// @param guildBoard The guild notice board model.
            NoticeSendForm(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                NoticeBoard& publicBoard,
                NoticeBoard& guildBoard
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form empty for a board and gives the text the keyboard; an open form stays as it is.
            /// @details Original: ShowCtrl.
            /// @param guild True for the guild board, false for the public one.
            void open(bool guild);

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Posts the notice when subject and text are clean and not empty.
            /// @details Original: SubmitNotice.
            void submit();

            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            NoticeBoard& m_publicBoard;
            NoticeBoard& m_guildBoard;
            ui::Window& m_window;
            ui::TextBox& m_subject;
            ui::Memo& m_text;
            bool m_guild = false;
    };

}

#endif
