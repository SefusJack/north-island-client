#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_ACTIVITIES_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_ACTIVITIES_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/events/ActivitySchedule.h"
#include "client/viewmodels/events/ActivityInfoWindow.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "protocol/ActivityLists.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <functional>
#include <string>

namespace client {

    /// @brief The event schedule window: lists events five a page and teleports to the chosen one.
    /// @details Original: ActivitiesWindow.xml, TActionListForm. The server sends the schedule and the states of its events (action 90); the chosen event is teleported to (89/1). The Event Schedule button blinks while an event is on and the window shut.
    class ActivitiesWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<std::string(int mapId)> sceneName;
                std::function<int()> ownLevel;
                std::function<bool()> teleportRefused;
                std::function<void(const std::string& text, std::function<void()> onOk)> ask;
                std::function<void(bool blinking)> blink;
            };

            /// @brief Loads the window layout and wires its events.
            /// @param services Game services.
            /// @param screen Screen the window is loaded into.
            /// @param messages Player message line.
            /// @param info Event information window.
            /// @param hooks Owner callbacks.
            ActivitiesWindow(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                ActivityInfoWindow& info,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the window when shut, closes it when open.
            /// @details Original: ToggleFormShown.
            void toggle();

            /// @brief Opens the window on the first page and stops the button blinking.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Closes the window and the information window.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Requests the schedule when the world is entered: loads the schedule kept in a file, resets the states to preparing, sends 89/0 with the version held.
            /// @details Original: RefreshActionList (and LoadActionInfo when the version is 0).
            void requestSchedule();

            /// @brief Takes the schedule from the server and keeps it in a file; the button blinks while an event is on.
            /// @details Original: RecvActionList (90/0).
            /// @param list The received schedule.
            void scheduleReceived(const protocol::ActivityList& list);

            /// @brief Takes the states of the events from the server.
            /// @details Original: RecvActionState (90/1).
            /// @param states The received states.
            void statesReceived(const protocol::ActivityStates& states);

            /// @brief Shows a server notice.
            /// @details Original: RecvActionNotice (90/2).
            /// @param kind 0 the server is busy, 1 the schedule updates.
            void noticeReceived(int kind);

        private:
            static constexpr int kRows = 5;

            /// @brief A row of the page: the number of its event in the schedule (0: none) and the texts.
            struct Row {
                int number = 0;
                std::string name;
                std::string start;
                std::string end;
                int state = 0;
            };

            /// @brief Fills the page with the events not over that are held today, five a page.
            /// @details Original: RefreshPage.
            void refreshPage();

            /// @brief Finds the row under the mouse (its hit rectangles are 39 pixels apart, the highlights 41).
            /// @details Original: HandleMouseMove.
            /// @param screenPoint Mouse position on the screen.
            /// @return Row 1 to 5, or 0 for none.
            int rowAt(core::Point screenPoint) const;

            /// @brief Paints the row highlights, the weekday, the rows' texts and the page number.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top-left of the page area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Checks the chosen event and asks "Go to the event zone?".
            /// @details Original: TeleportClick.
            void transmit();

            /// @brief Runs the teleport checks and sends the teleport request (89/1).
            /// @details Original: DoActionTeleport.
            void teleport();

            /// @brief Turns the page when it stays in range.
            /// @details Original: PrevPage / NextPage.
            /// @param step -1 previous, 1 next.
            void turnPage(int step);

            /// @brief Shows (or hides, when already shown) the information of a row's event.
            /// @details Original: QueryClick.
            /// @param row Row 1 to 5.
            void query(int row);

            /// @brief Gives the file the schedule is kept in.
            /// @return Path of ActionInfo.dat in the user folder.
            std::filesystem::path cacheFile() const;

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            ActivityInfoWindow& m_info;
            Hooks m_hooks;
            ActivitySchedule m_schedule;
            ui::Window& m_window;
            std::array<std::reference_wrapper<ui::ImageButton>, kRows> m_queries;
            std::array<std::reference_wrapper<ui::ImageButton>, 4> m_controls;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_textColorSpot;
            const ui::Spot& m_processingColorSpot;
            const ui::Spot& m_highlightSpot;
            std::array<Row, kRows> m_page{};
            int m_pageNumber = 1;
            int m_pageCount = 1;
            int m_selected = 0;
            int m_hovered = 0;
            int m_queried = 0;
    };

}

#endif
