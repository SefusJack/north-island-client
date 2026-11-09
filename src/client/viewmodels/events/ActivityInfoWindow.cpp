#include "client/viewmodels/events/ActivityInfoWindow.h"

#include "core/DateTime.h"
#include "gfx/TextStyle.h"

#include <cmath>

namespace client {

    namespace {

        constexpr int kLineChars = 20; // characters that fit on one wrapped line
        constexpr int kMinutesPerDay = 1440; // 24 * 60 minutes in a day

        /// @brief Loads ActivityInfoWindow.xml into the screen and returns its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ActivityInfoWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ActivityInfoWindow");
        }

        /// @brief Counts the lines a text takes, Ceil(Length(text) / 20).
        /// @param text The text.
        /// @return Line count.
        int linesOf(const std::string& text) { return (static_cast<int>(text.size()) + kLineChars - 1) / kLineChars; }

    }

    ActivityInfoWindow::ActivityInfoWindow(
        GameServices& services,
        ui::Screen& screen
    )
        : m_screen(screen),
          m_window(loadWindow(services, screen)),
          m_skin(screen.get<ui::Panel>("ActivityInfoSkin")),
          m_text(screen.get<ui::DrawingArea>("ActivityInfoText")),
          m_lineSpot(screen.get<ui::Spot>("ActivityInfoLine")),
          m_textColorSpot(screen.get<ui::Spot>("ActivityInfoTextColor")) {
        m_text.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onActivityInfoClose", [this](const ui::EventArgs&) { close(); });
    }

    void ActivityInfoWindow::show(const protocol::ActivityRecord& activity, const std::string& location, core::Rect arrow) {
        const std::string start = core::formatDateTime(activity.time);
        const std::string end = core::formatDateTime(activity.duration / static_cast<double>(kMinutesPerDay) + activity.time);
        if (activity.time - std::floor(activity.time) + activity.duration / static_cast<double>(kMinutesPerDay) >
            1.0) { // 1.0: the event runs past the end of its day, so show dates too
            // substr(0, 16): the "date HH:MM" part of the date-time text
            m_start = start.substr(0, 16);
            m_end = end.substr(0, 16);
        } else {
            // substr(11, 5): the "HH:MM" part of the date-time text
            m_start = start.substr(11, 5);
            m_end = end.substr(11, 5);
            if (m_end == "00:00") {
                m_end = "24:00"; // midnight end shown as 24:00
            }
        }
        m_location = location;
        m_level = activity.minLevel > 0 ? std::to_string(activity.minLevel) : "No limits";
        m_introduction = activity.description;
        // 5 label rows plus the text lines; 20 pixels of vertical padding
        const int rows = 5 + linesOf(m_start) + linesOf(m_end) + linesOf(m_location) + linesOf(m_level) + linesOf(m_introduction);
        const int height = rows * m_lineSpot.size().height + 20;
        m_window.setBounds({arrow.x - m_lineSpot.size().width, arrow.y, m_lineSpot.size().width, height});
        m_skin.setBounds({0, 0, m_lineSpot.size().width, height});
        m_text.setBounds({0, 0, m_lineSpot.size().width, height});
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void ActivityInfoWindow::paint(gfx::Canvas& canvas, core::Point at) {
        constexpr int kInset = 10; // text inset from the window edge, px
        constexpr int kTextWidth = 160; // text clip width, px
        constexpr int kCharHeight = 16; // text line height, px
        const gfx::TextStyle style{m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        int y = at.y + kInset;
        const auto line = [&](const std::string& text) {
            canvas.drawText({at.x + kInset, y}, text, style, kTextWidth, kCharHeight);
            y += linesOf(text) * m_lineSpot.size().height;
        };
        line("Start:");
        line(m_start);
        line("End:");
        line(m_end);
        line("Location:");
        line(m_location);
        line("Level restricitons:");
        line(m_level);
        line("Event Introduction:");
        line(m_introduction);
    }

}
