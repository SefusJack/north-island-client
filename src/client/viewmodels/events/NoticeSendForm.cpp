#include "client/viewmodels/events/NoticeSendForm.h"

#include "client/rules/TextFilter.h"
#include "core/StringUtil.h"

namespace client {

    namespace {

        constexpr int kMessageMs = 1300; // notice display time, ms

        /// @brief Loads NoticeSendForm.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the layout is loaded into.
        /// @return The "NoticeSendForm" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("NoticeSendForm.xml"), services.loadContext());
            return screen.get<ui::Window>("NoticeSendForm");
        }

    }

    NoticeSendForm::NoticeSendForm(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        NoticeBoard& publicBoard,
        NoticeBoard& guildBoard
    )
        : m_screen(screen),
          m_messages(messages),
          m_publicBoard(publicBoard),
          m_guildBoard(guildBoard),
          m_window(loadWindow(services, screen)),
          m_subject(screen.get<ui::TextBox>("NoticeSendSubject")),
          m_text(screen.get<ui::Memo>("NoticeSendText")) {
        screen.on("onNoticeSendConfirm", [this](const ui::EventArgs&) { submit(); });
        screen.on("onNoticeSendClose", [this](const ui::EventArgs&) { close(); });
    }

    void NoticeSendForm::open(bool guild) {
        if (m_window.isVisible()) {
            return;
        }
        m_guild = guild;
        m_subject.clear();
        m_text.clear();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_screen.focus(m_text.name());
    }

    void NoticeSendForm::submit() {
        if (!isTextClean(m_subject.text()) || !isTextClean(m_text.text())) {
            m_messages.show(kIllegalWords, 2000); // message time in ms
            return;
        }
        const std::string subject = core::trim(m_subject.text());
        const std::string text = core::trim(m_text.text());
        if (subject.empty()) {
            m_messages.show("Title can't be void", kMessageMs);
            return;
        }
        if (text.empty()) {
            m_messages.show("Content can't be void", kMessageMs);
            return;
        }
        (m_guild ? m_guildBoard : m_publicBoard).post(subject, text);
        m_subject.clear();
        m_text.clear();
        close();
    }

}
