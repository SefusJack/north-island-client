#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_INN_ANSWER_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHAT_INN_ANSWER_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <functional>
#include <string>

namespace client {

    /// @brief The inn's answer window (InnAnswerWindow.xml, TINNAnsForm): a line of the innkeeper's and Close.
    class InnAnswerWindow {
        public:
            /// @brief Loads InnAnswerWindow.xml and wires the window.
            /// @param services Game services (layout).
            /// @param screen Screen the window is loaded into.
            InnAnswerWindow(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Shows the innkeeper's line.
            /// @param line Text to show.
            /// @param onClosed Called when the window is closed (the event goes on).
            /// @details Original: ShowCtrl with the line; onClosed is CloseInnAnswer.
            void show(const std::string& line, std::function<void()> onClosed);

        private:
            /// @brief Hides the window and runs the closed handler once.
            void close();

            ui::Screen& m_screen;
            ui::Window& m_window;
            ui::Memo& m_text;
            std::function<void()> m_onClosed;
    };

}

#endif
