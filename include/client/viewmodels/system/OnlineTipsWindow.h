#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_ONLINE_TIPS_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_ONLINE_TIPS_WINDOW_H

#include "client/services/GameServices.h"
#include "ui/ListBox.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <string>
#include <vector>

namespace client {

    /// @brief Online Info window listing help topics and showing a topic's text.
    /// @details Original: OnlineTipsWindow.xml, Tjo_OnLineTip (Readme in the system window). Topics come from txt/hotkeyt.txt, eight a page; texts from txt/hotkeye.txt.
    class OnlineTipsWindow {
        public:
            /// @brief Loads the window and the topic and text files, and wires the events.
            /// @param services Shared game services.
            /// @param screen Screen the window is loaded into.
            OnlineTipsWindow(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Opens the window or closes it.
            /// @details Original: ToggleOnLineTipForm.
            void toggle();

        private:
            /// @brief Shows the lines of a topic in the text memo.
            /// @details Original: ShowOnlineTipTopic.
            /// @param index Zero-based topic index in the list.
            void showTopic(int index);

            /// @brief Goes back from a topic's text to the topic list.
            /// @details Original: PrevTipPageClick.
            void showTopics();

            /// @brief Pages the topic list up or down by one page.
            /// @details Original: TipPageUpClick / TipPageDownClick (ScrollBarPageUp / ScrollBarPageDownRaw); the list's scroll bar is hidden.
            /// @param step Negative for a page up, positive for a page down.
            void turnTopics(int step);

            /// @brief Switches the window between the topic list and the text view.
            /// @param text True for the text view, false for the topic list.
            void showTextControls(bool text);

            ui::Screen& m_screen;
            ui::Window& m_window;
            ui::ListBox& m_topics;
            ui::Memo& m_text;
            const ui::Spot& m_closeTopicsSpot;
            const ui::Spot& m_closeTextSpot;
            std::vector<int> m_topicLines;
            std::vector<std::string> m_lines;
    };

}

#endif
