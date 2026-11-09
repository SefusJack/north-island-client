#include "client/viewmodels/instances/InstanceMessageForm.h"

#include "core/StringUtil.h"

namespace client {

    namespace {

        /// @brief Loads the form's layout and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the form is loaded into.
        /// @return The form's window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("InstanceMessageForm.xml"), services.loadContext());
            return screen.get<ui::Window>("InstanceMessageForm");
        }

    }

    InstanceMessageForm::InstanceMessageForm(
        GameServices& services,
        ui::Screen& screen,
        std::function<void(const std::string&)> onConfirm
    )
        : m_screen(screen),
          m_confirm(std::move(onConfirm)),
          m_window(loadWindow(services, screen)),
          m_text(screen.get<ui::Memo>("InstanceMessageText")) {
        screen.on("onInstanceMessageCancel", [this](const ui::EventArgs&) { cancel(); });
        screen.on("onInstanceMessageConfirm", [this](const ui::EventArgs&) { confirm(); });
    }

    void InstanceMessageForm::open(core::Point at) {
        place(at);
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_screen.focus(m_text.name());
    }

    void InstanceMessageForm::place(core::Point at) {
        const core::Rect bounds = m_window.bounds();
        m_window.setBounds({at.x, at.y, bounds.width, bounds.height});
        m_window.fitToScreen();
    }

    void InstanceMessageForm::cancel() {
        close();
        m_text.clear();
    }

    void InstanceMessageForm::confirm() {
        m_confirm(core::trim(m_text.text()));
        close();
    }

}
