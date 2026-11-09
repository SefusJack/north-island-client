#include "client/viewmodels/events/NoticeReadForm.h"

#include "client/rules/GmIds.h"

namespace client {

    namespace {

        /// @brief Loads NoticeReadForm.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "NoticeReadForm" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("NoticeReadForm.xml"), services.loadContext());
            return screen.get<ui::Window>("NoticeReadForm");
        }

    }

    NoticeReadForm::NoticeReadForm(
        GameServices& services,
        ui::Screen& screen,
        NoticeBoard& publicBoard,
        NoticeBoard& guildBoard,
        Hooks hooks
    )
        : m_screen(screen),
          m_publicBoard(publicBoard),
          m_guildBoard(guildBoard),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_subject(screen.get<ui::TextBox>("NoticeReadSubject")),
          m_text(screen.get<ui::Memo>("NoticeReadText")) {
        screen.on("onNoticeReadClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onNoticeReadDelete", [this](const ui::EventArgs&) { deleteNotice(); });
        screen.on("onNoticeReadMail", [this](const ui::EventArgs&) { mailAuthor(); });
    }

    void NoticeReadForm::expect(bool guild, const protocol::NoticeRow& row) {
        m_guild = guild;
        m_rowAuthorId = row.authorId;
        m_author = row.author;
        close();
    }

    void NoticeReadForm::show(bool guild, const protocol::NoticeContent& content) {
        m_guild = guild;
        m_subject.setText(content.subject);
        m_text.clear();
        m_text.appendLine(content.text);
        m_noticeAuthorId = content.authorId;
        if (m_window.isVisible()) {
            return;
        }
        layoutButtons();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void NoticeReadForm::layoutButtons() {
        const std::int32_t own = m_hooks.ownId();
        const bool author = own == m_noticeAuthorId;
        bool canDelete = author;
        bool deleteGray = !author;
        bool mail = !author;
        bool mailGray = author;
        if (isGmId(own)) {
            canDelete = true;
            mail = false;
            mailGray = true;
        }
        if (m_guild && m_hooks.inGuildLeadership()) {
            canDelete = true;
            deleteGray = false;
        }
        m_screen.get<ui::Element>("NoticeReadDelete").setVisible(canDelete);
        m_screen.get<ui::Element>("NoticeReadDeleteGray").setVisible(deleteGray);
        m_screen.get<ui::Element>("NoticeReadMail").setVisible(mail);
        m_screen.get<ui::Element>("NoticeReadSendGray").setVisible(mailGray);
    }

    void NoticeReadForm::deleteNotice() {
        (m_guild ? m_guildBoard : m_publicBoard).deleteOpened();
        close();
    }

    void NoticeReadForm::mailAuthor() { m_hooks.mailAuthor(m_guild, m_rowAuthorId, m_author); }

}
