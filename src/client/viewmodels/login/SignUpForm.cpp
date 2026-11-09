#include "client/viewmodels/login/SignUpForm.h"

#include "client/rules/SignUpRules.h"

namespace client {

    SignUpForm::SignUpForm(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        Callbacks callbacks
    )
        : m_screen(screen),
          m_messages(messages),
          m_callbacks(std::move(callbacks)),
          m_form(screen.load(services.game.layout("SignUpForm.xml"), services.loadContext())),
          m_account(screen.get<ui::TextBox>("SignUpAccount")),
          m_password(screen.get<ui::TextBox>("SignUpPassword")),
          m_again(screen.get<ui::TextBox>("SignUpAgain")) {
        screen.on("onSignUp", [this](const ui::EventArgs&) { submit(); });
        screen.on("onSignUpNext", [this](const ui::EventArgs& args) { next(args.source); });
        screen.on("onSignUpBack", [this](const ui::EventArgs&) {
            if (isVisible()) {
                hide();
                m_callbacks.back();
            }
        });
    }

    void SignUpForm::show(const std::string& account) {
        m_account.setText(account);
        m_password.clear();
        m_again.clear();
        m_form.setVisible(true);
        m_screen.focus(account.empty() ? "SignUpAccount" : "SignUpPassword");
    }

    void SignUpForm::hide() {
        m_form.setVisible(false);
        m_password.clear();
        m_again.clear();
    }

    void SignUpForm::next(const std::string& from) {
        if (from == "SignUpAccount") {
            m_screen.focus("SignUpPassword");
        } else if (from == "SignUpPassword") {
            m_screen.focus("SignUpAgain");
        } else {
            m_screen.focus("SignUpAccount");
        }
    }

    void SignUpForm::submit() {
        if (!isVisible()) {
            return;
        }
        if (const auto problem = signUpProblem(m_account.text(), m_password.text(), m_again.text())) {
            m_messages.show(*problem);
            return;
        }
        const std::string account = m_account.text();
        const std::string password = m_password.text();
        hide();
        m_callbacks.signUp(account, password);
    }

}
