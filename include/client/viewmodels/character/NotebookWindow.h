#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_NOTEBOOK_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_NOTEBOOK_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <functional>

namespace client {

    /// @brief Notebook window: Quest Log, Monster Information and Character Introductions (the star book's sky).
    /// @details Original: NotebookWindow.xml, CH_NoteBook; opened by using a notebook item outside a battle.
    class NotebookWindow {
        public:
            /// @brief Builds the window and wires its buttons.
            /// @param services Game services.
            /// @param screen Screen the window lives on.
            /// @param toggleQuests Opens or closes the quest log.
            /// @param toggleMonsters Opens or closes the monster book.
            /// @param openStars Opens the star book's sky.
            NotebookWindow(
                GameServices& services,
                ui::Screen& screen,
                std::function<void()> toggleQuests,
                std::function<void()> toggleMonsters,
                std::function<void()> openStars
            );

            /// @brief Shows the notebook in front.
            void open();

            /// @brief Hides the notebook.
            /// @details Original: CloseNoteBook.
            void close() { m_window.setVisible(false); }

        private:
            ui::Screen& m_screen;
            ui::Window& m_window;
            std::function<void()> m_toggleQuests;
            std::function<void()> m_toggleMonsters;
            std::function<void()> m_openStars;
    };

}

#endif
