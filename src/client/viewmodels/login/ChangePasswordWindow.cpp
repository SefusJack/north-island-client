#include "client/viewmodels/login/ChangePasswordWindow.h"

#include "protocol/ChangePasswordRequest.h"
#include "ui/TextBox.h"

namespace client {

    namespace {

        constexpr int kFields = 6; // text fields of the form: 1..3 are the three star-filled ones, 4..6 the real inputs
        constexpr int kOld = 4; // field 4: old password
        constexpr int kNew = 5; // field 5: new password
        constexpr int kAgain = 6; // field 6: new password again
        constexpr std::size_t kShortest = 6; // shortest allowed password, characters
        constexpr std::size_t kLongest = 10; // longest allowed password, characters

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
            // the first three fields hold 10 stars (the longest password length)
            m_screen.get<ui::TextBox>(fieldName(field)).setText(field < kOld ? "**********" : "");
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void ChangePasswordWindow::next(const std::string& field) {
        const int number = field.back() - '0';
        if (number >= kFields) {
            submit();
            m_screen.focus(fieldName(kOld));
            return;
        }
        m_screen.focus(fieldName(number + 1));
    }

    void ChangePasswordWindow::submit() {
        const std::string old = text(kOld);
        const std::string fresh = text(kNew);
        if (old.size() < kShortest || old.size() > kLongest) {
            m_messages.show("Old code The length should be within 6-10 letters", 2000); // message display time in ms
            return;
        }
        if (fresh != text(kAgain)) {
            m_messages.show("Two guilds New code Not matched", 2000); // message display time in ms
            return;
        }
        if (fresh.size() < kShortest || fresh.size() > kLongest) {
            m_messages.show("New code The length should be within 6-10 letters", 2000); // message display time in ms
            return;
        }
        m_ask(
            "Are you sure you want to change the cipher?",
            [this, old, fresh] {
                m_services.connection.send(protocol::ChangePasswordRequest{old, fresh}.encode());
                close();
            },
            [] {}
        );
    }

    void ChangePasswordWindow::showResult(int result) {
        switch (result) {
            // every message below is shown for 2000 ms
            case 0: m_messages.show("Modify failed", 2000); break; // 35/1 0: modify failed
            case 1: m_messages.show("Modify succeeded", 2000); break; // 35/1 1: modify succeeded
            case 2: m_messages.show("Wrong old password", 2000); break; // 35/1 2: wrong old password
            case 3: m_messages.show("Wrong old cipher", 2000); break; // 35/1 3: wrong old cipher
            case 4: m_messages.show("The password is too short.", 2000); break; // 35/1 4: the password is too short
            case 5: m_messages.show("Unable to modify the password and cipher in this server.", 2000); break; // 35/1 5: not allowed on this server
            case 6: m_messages.show("The server is busy now. Try later.", 2000); break; // 35/1 6: server busy
            default: break; // unknown result: nothing shown
        }
    }

}
