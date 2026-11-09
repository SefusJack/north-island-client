#include "client/viewmodels/login/PasswordForm.h"

namespace client {

    namespace {

        constexpr int kShortMessageMs = 2000; // how long a player message stays up, ms
        const char* const kStars = "**********"; // ten stars shown in the filled-in password fields (the longest password length)

    }

    PasswordForm::PasswordForm(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages
    )
        : m_screen(screen),
          m_messages(messages),
          m_form(screen.load(services.game.layout("PasswordForm.xml"), services.loadContext())),
          m_setMode(screen.get<ui::Element>("PasswordModeSet")),
          m_confirmMode(screen.get<ui::Element>("PasswordModeConfirm")) {
        for (int i = 1; i <= 4; ++i) { // four fields: password, repeat, cipher, repeat
            m_fields.emplace_back(screen.get<ui::TextBox>("PasswordField" + std::to_string(i)));
        }
        screen.on("onPasswordNext", [this](const ui::EventArgs&) { focusNext(); });
        screen.on("onPasswordField", [this](const ui::EventArgs& args) { m_focus = args.source.back() - '0'; });
        screen.on("onPasswordOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onPasswordCancel", [this](const ui::EventArgs&) { cancel(); });
    }

    void PasswordForm::show(Mode mode, std::function<void(const Result&)> onOk, std::function<void()> onCancel) {
        m_onOk = std::move(onOk);
        m_onCancel = std::move(onCancel);
        for (ui::TextBox& field : m_fields) {
            field.clear();
        }
        m_fields[0].get().setText(kStars);
        m_fields[1].get().setText(kStars);
        m_setMode.setVisible(mode == Mode::Set);
        m_confirmMode.setVisible(mode == Mode::Confirm);
        m_form.setVisible(true);
    }

    void PasswordForm::focusNext() {
        ++m_focus;
        if (m_focus > 4) { // past the fourth field: act as OK
            m_focus = 0;
            confirm();
            return;
        }
        m_screen.focus("PasswordField" + std::to_string(m_focus));
    }

    void PasswordForm::confirm() {
        const std::string& cipher = m_fields[2].get().text(); // field index 2: the cipher
        if (m_fields[0].get().text() != m_fields[1].get().text()) {
            m_messages.show("The two passwords are different.", kShortMessageMs);
            return;
        }
        if (cipher != m_fields[3].get().text()) { // field index 3: the cipher repeated
            m_messages.show("The two ciphers are different.", kShortMessageMs);
            return;
        }
        if (cipher.size() < 6) { // shortest cipher
            m_messages.show("The cipher is less than 6 letters.", kShortMessageMs);
            return;
        }
        if (cipher.size() > 10) { // longest cipher
            m_messages.show("The cipher is more than 10 letters.", kShortMessageMs);
            return;
        }
        hide();
        if (m_onOk) {
            m_onOk({m_fields[0].get().text(), cipher});
        }
    }

    void PasswordForm::cancel() {
        hide();
        if (m_onCancel) {
            m_onCancel();
        }
    }

}
