#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_ANNOUNCE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_ANNOUNCE_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/ScrollBar.h"
#include "ui/Window.h"

#include <functional>
#include <string>

namespace client {

    /// @brief The announcement form of an event.
    /// @details Original: AnnounceWindow.xml, BK_Menu.pas TBK_AnnounceForm. 36/1 opens it, 36/2 puts its text in; Close lets the event go on.
    class AnnounceWindow {
        public:
            /// @brief Loads the window layout and wires its events.
            /// @param services Game services.
            /// @param screen Screen the window is loaded into.
            /// @param onClosed Called when the form is closed.
            AnnounceWindow(
                GameServices& services,
                ui::Screen& screen,
                std::function<void()> onClosed
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form (36/1).
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Replaces the text (36/2) and scrolls back to the top.
            /// @details Original: ShowAnnounceText.
            /// @param text The announcement text.
            void setText(const std::string& text);

        private:
            /// @brief Hides the form and lets the event go on.
            /// @details Original: CloseAnnounceForm: FRequestNextResult, HideCtrl.
            void close();

            ui::Screen& m_screen;
            ui::Window& m_window;
            ui::Memo& m_text;
            std::function<void()> m_onClosed;
    };

}

#endif
