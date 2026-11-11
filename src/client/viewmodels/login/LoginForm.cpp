#include "client/viewmodels/login/LoginForm.h"

#include "core/FileSystem.h"
#include "core/Log.h"

#include <cctype>
#include <system_error>

namespace client {

    namespace {

        const char* const kAccountListFile = "AccountList.dat";

    }

    LoginForm::LoginForm(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        Callbacks callbacks
    )
        : m_services(services),
          m_screen(screen),
          m_messages(messages),
          m_callbacks(std::move(callbacks)),
          m_form(screen.load(services.game.layout("LoginForm.xml"), services.loadContext())),
          m_account(screen.get<ui::ComboBox>("Account")),
          m_password(screen.get<ui::TextBox>("Password")),
          m_remember(screen.get<ui::CheckBox>("RememberAccount")) {
        screen.on("onAccountEntered", [this](const ui::EventArgs&) {
            if (checkAccount()) {
                m_screen.focus("Password");
            }
        });
        screen.on("onLogin", [this](const ui::EventArgs&) { submit(); });
        screen.on("onPrevious", [this](const ui::EventArgs&) { m_callbacks.previous(); });
        screen.on("onSignUpPage", [this](const ui::EventArgs&) {
            if (isVisible() && m_callbacks.signUp) {
                m_callbacks.signUp();
            }
        });
        screen.on("onAbout", [this](const ui::EventArgs&) { m_messages.show("The introduction video is not available."); });
        screen.on("onForgetAccount", [this](const ui::EventArgs& e) {
            m_account.removeItem(e.index);
            saveHistory();
        });
    }

    void LoginForm::show() {
        m_form.setVisible(true);
        m_password.clear();
        loadHistory();
        m_screen.focus("Account");
    }

    void LoginForm::hide() {
        m_form.setVisible(false);
        m_password.clear();
    }

    void LoginForm::clearFields() {
        m_account.clear();
        m_password.clear();
    }

    bool LoginForm::checkAccount() {
        const std::string& account = m_account.text();
        bool valid = account.size() >= 4 && account.size() <= 14; // account length limits 4..14
        for (char c : account) {
            valid = valid && std::isalnum(static_cast<unsigned char>(c)) != 0;
        }
        if (!valid) {
            m_messages.show("wrong account");
            m_account.clear();
        }
        return valid;
    }

    void LoginForm::submit() {
        if (!isVisible() || !checkAccount()) {
            return;
        }
        if (m_password.text().empty()) {
            m_messages.show("Please enter the password");
            return;
        }
        const std::string password = m_password.text();
        hide();
        m_callbacks.login(m_account.text(), password);
    }

    void LoginForm::rememberAccount() {
        if (m_remember.isChecked()) {
            m_account.addToHistory(m_account.text());
            saveHistory();
        }
    }

    void LoginForm::rememberAccount(const std::string& account) {
        loadHistory();
        m_account.addToHistory(account);
        saveHistory();
    }

    void LoginForm::loadHistory() {
        std::vector<std::string> accounts;
        if (const auto lines = core::readLines(m_services.game.user(kAccountListFile))) {
            accounts = *lines;
        }
        m_account.setItems(accounts);
    }

    void LoginForm::saveHistory() {
        std::error_code error;
        if (m_account.items().empty()) {
            std::filesystem::remove(m_services.game.user(kAccountListFile), error);
            return;
        }
        std::filesystem::create_directories(m_services.game.userRoot(), error);
        if (!core::writeLines(m_services.game.user(kAccountListFile), m_account.items())) {
            core::logWarning("cannot save the account list");
        }
    }

}
