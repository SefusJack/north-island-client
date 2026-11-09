#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_QUEST_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_QUEST_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/QuestMarks.h"
#include "ui/ListBox.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <string>
#include <vector>

namespace client {

    /// @brief The quest window (QuestWindow.xml, Tjo_TaskView): quests done and going on, ten a page, with a detail text per quest.
    class QuestWindow {
        public:
            /// @brief Loads QuestWindow.xml and wires the window.
            /// @param services Game services (layout, marks table, pictures).
            /// @param screen Screen the window is loaded into.
            /// @param marks Quest flags and counter marks.
            QuestWindow(
                GameServices& services,
                ui::Screen& screen,
                QuestMarks& marks
            );

            /// @brief Whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows or hides the window, refreshing the list when it opens.
            /// @details Original: ToggleTaskViewForm (Ctrl+M).
            void toggle();

            /// @brief Shows or hides the window without refreshing the list (the notebook's Quest Log).
            void toggleFromNotebook();

            /// @brief Rebuilds the list when the marks have changed; called every frame.
            /// @details Original: RefreshTaskList.
            void update();

        private:
            /// @brief One listed quest.
            struct Entry {
                int markId = 0;
                bool done = false;
            };

            /// @brief Rebuilds the list: done quests by flag bit, then those going on by counter mark.
            void refresh();

            /// @brief Adds a counter mark's quest to the list unless it is listed as going on already.
            /// @param startIndex Mark table index of the counter mark.
            /// @details Original: AddTaskListEntry.
            void addGoingOn(int startIndex);

            /// @brief Shows the window on its last page.
            /// @details Original: Tjo_TaskView.ShowCtrl.
            void show();

            /// @brief Shows a quest's name and the step its counter has reached (or its ending once done).
            /// @param index List index of the quest.
            /// @details Original: ShowTaskDetail.
            void showDetail(int index);

            /// @brief Turns the list a page of ten.
            /// @param step Negative for the previous page, else the next.
            /// @details Original: TaskPageUpClick / TaskPageDownClick on the hidden scroll bar.
            void turnPage(int step);

            /// @brief Paints the tick or exclamation icons and the page number.
            /// @param canvas Target canvas.
            /// @param at Origin of the drawing area.
            /// @details Original: DrawControl / RefreshTaskPageIcons.
            void paintMarks(gfx::Canvas& canvas, core::Point at);

            static constexpr int kPageRows = 10; // quest rows on one page

            GameServices& m_services;
            ui::Screen& m_screen;
            QuestMarks& m_marks;
            ui::Window& m_window;
            ui::ListBox& m_list;
            ui::ListBox& m_detail;
            ui::Memo& m_title;
            const ui::Spot& m_doneSpot;
            const ui::Spot& m_goingOnSpot;
            const ui::Spot& m_detailSpot;
            const ui::Spot& m_pageSpot;
            std::vector<Entry> m_entries;
            int m_scrollPos = 0;
    };

}

#endif
