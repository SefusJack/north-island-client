#include "client/viewmodels/login/ChangePasswordWindow.h"

#include "protocol/ChangePasswordRequest.h"
#include "ui/TextBox.h"

namespace client {

    namespace {

        constexpr int kFields = 6; // text fields of the form: 1..3 the account password, 4..6 the secret code
        constexpr int kOldPassword = 1; // field 1: old account password
        constexpr int kNewPassword = 2; // field 2: new account password
        constexpr int kPasswordAgain = 3; // field 3: new account password again
        constexpr int kOld = 4; // field 4: old secret code
        constexpr int kNew = 5; // field 5: new secret code
        constexpr int kAgain = 6; // field 6: new secret code again
        constexpr std::size_t kShortest = 6; // shortest allowed secret code, characters
        constexpr std::size_t kLongest = 10; // longest allowed secret code, characters
        constexpr std::size_t kShortestPassword = 4; // shortest allowed account password, characters (as at sign-up)
        constexpr std::size_t kLongestPassword = 14; // longest allowed account password, characters (as at sign-up)
        constexpr int kMessageMs = 2000; // message display time in ms

        /// @brief Loads ChangePasswordWindow.xml into the screen.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ChangePasswordWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ChangePassword");
        }

        /// @brief Element name of a field.
        /// @param field Field number 1..6.
        /// @return The element name.
        std::string fieldName(int field) { return "ChangePassword" + std::to_string(field); }

    }

    ChangePasswordWindow::ChangePasswordWindow(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        Ask ask
    )
        : m_services(services),
          m_screen(screen),
          m_messages(messages),
          m_ask(std::move(ask)),
          m_window(loadWindow(services, screen)) {
        screen.on("onChangePasswordOk", [this](const ui::EventArgs&) { submit(); });
        screen.on("onChangePasswordCancel", [this](const ui::EventArgs&) { close(); });
        screen.on("onChangePasswordNext", [this](const ui::EventArgs& args) { next(args.source); });
    }

    std::string ChangePasswordWindow::text(int field) { return m_screen.get<ui::TextBox>(fieldName(field)).text(); }

    void ChangePasswordWindow::open() {
        for (int field = 1; field <= kFields; ++field) {
            m_screen.get<ui::TextBox>(fieldName(field)).setText("");
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_screen.focus(fieldName(kOldPassword));
    }

    void ChangePasswordWindow::next(const std::string& field) {
        const int number = field.back() - '0';
        if (number >= kFields) {
            submit();
            m_screen.focus(fieldName(kOldPassword));
            return;
        }
        m_screen.focus(fieldName(number + 1));
    }

    void ChangePasswordWindow::submit() {
        const std::string oldPassword = text(kOldPassword);
        const std::string newPassword = text(kNewPassword);
        const std::string old = text(kOld);
        const std::string fresh = text(kNew);
        // a section counts as filled in when any of its three rows has text
        const bool password = !oldPassword.empty() || !newPassword.empty() || !text(kPasswordAgain).empty();
        const bool code = !old.empty() || !fresh.empty() || !text(kAgain).empty();
        if (!password && !code) {
            m_messages.show("Fill in the password rows, the secret code rows, or both.", kMessageMs);
            return;
        }
        if (password) {
            if (oldPassword.empty()) {
                m_messages.show("Enter your original password.", kMessageMs);
                return;
            }
            if (newPassword != text(kPasswordAgain)) {
                m_messages.show("The two new passwords are not the same.", kMessageMs);
                return;
            }
            if (newPassword.size() < kShortestPassword || newPassword.size() > kLongestPassword) {
                m_messages.show("The new password needs 4 to 14 characters.", kMessageMs);
                return;
            }
        }
        if (code) {
            if (old.size() < kShortest || old.size() > kLongest) {
                m_messages.show("Old code The length should be within 6-10 letters", kMessageMs);
                return;
            }
            if (fresh != text(kAgain)) {
                m_messages.show("Two guilds New code Not matched", kMessageMs);
                return;
            }
            if (fresh.size() < kShortest || fresh.size() > kLongest) {
                m_messages.show("New code The length should be within 6-10 letters", kMessageMs);
                return;
            }
        }
        const char* question = password && code ? "Are you sure you want to change the password and the secret code?"
                               : password       ? "Are you sure you want to change the password?"
                                                : "Are you sure you want to change the cipher?";
        m_ask(
            question,
            [this, password, code, oldPassword, newPassword, old, fresh] {
                if (password) {
                    m_services.connection.send(protocol::ChangeAccountPasswordRequest{oldPassword, newPassword}.encode());
                }
                if (code) {
                    m_services.connection.send(protocol::ChangePasswordRequest{old, fresh}.encode());
                }
                close();
            },
            [] {}
        );
    }

    void ChangePasswordWindow::showResult(int result) {
        // grouped: when both sections were sent, the password and secret code results stack in one box
        switch (result) {
            // every message below is shown for 2000 ms
            case 0: m_messages.showGrouped("Modify failed", 2000); break; // 35/1 0: modify failed
            case 1: m_messages.showGrouped("Modify succeeded", 2000); break; // 35/1 1: modify succeeded
            case 2: m_messages.showGrouped("Wrong old password", 2000); break; // 35/1 2: wrong old password
            case 3: m_messages.showGrouped("Wrong old cipher", 2000); break; // 35/1 3: wrong old cipher
            case 4: m_messages.showGrouped("The password is too short.", 2000); break; // 35/1 4: the password is too short
            case 5: m_messages.showGrouped("Unable to modify the password and cipher in this server.", 2000); break; // 35/1 5: not allowed on this server
            case 6: m_messages.showGrouped("The server is busy now. Try later.", 2000); break; // 35/1 6: server busy
            default: break; // unknown result: nothing shown
        }
    }

    void ChangePasswordWindow::showAccountResult(int result) {
        // grouped, like showResult
        switch (result) {
            case 1: m_messages.showGrouped("Password changed. Use the new password to log in next time.", kMessageMs); break; // 35/12 1: changed
            case 2: m_messages.showGrouped("Wrong original password.", kMessageMs); break; // 35/12 2: wrong old password
            case 4: m_messages.showGrouped("The new password needs 4 to 14 characters.", kMessageMs); break; // 35/12 4: bad new password length
            default: m_messages.showGrouped("The password was not changed.", kMessageMs); break; // any other result: failed
        }
    }

}
