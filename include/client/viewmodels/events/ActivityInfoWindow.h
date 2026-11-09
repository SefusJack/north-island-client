#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_ACTIVITY_INFO_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_EVENTS_ACTIVITY_INFO_WINDOW_H

#include "client/services/GameServices.h"
#include "core/Point.h"
#include "protocol/ActivityLists.h"
#include "ui/DrawingArea.h"
#include "ui/Panel.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <string>

namespace client {

    /// @brief Panel telling what an event of the schedule is: start, end, location, level and introduction.
    /// @details Original: ActivityInfoWindow.xml, TShowActionInfoForm. Shown beside the arrow of its row.
    class ActivityInfoWindow {
        public:
            /// @brief Loads the window layout and wires its events.
            /// @param services Game services.
            /// @param screen Screen the window is loaded into.
            ActivityInfoWindow(
                GameServices& services,
                ui::Screen& screen
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the panel at the left of the arrow, as high as its lines.
            /// @details Original: ShowCtrl and BuildActionInfo.
            /// @param activity The event.
            /// @param location The map's name and cell, or "None".
            /// @param arrow The arrow's screen rectangle.
            void show(const protocol::ActivityRecord& activity, const std::string& location, core::Rect arrow);

            /// @brief Hides the window.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Paints the labels and their texts.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top-left of the text area.
            void paint(gfx::Canvas& canvas, core::Point at);

            ui::Screen& m_screen;
            ui::Window& m_window;
            ui::Panel& m_skin;
            ui::DrawingArea& m_text;
            const ui::Spot& m_lineSpot;
            const ui::Spot& m_textColorSpot;
            std::string m_start;
            std::string m_end;
            std::string m_location;
            std::string m_level;
            std::string m_introduction;
    };

}

#endif
