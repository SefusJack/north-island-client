#include "client/viewmodels/events/ActivitiesWindow.h"

#include "core/DateTime.h"
#include "gfx/TextStyle.h"
#include "protocol/ActivityRequests.h"

#include <algorithm>
#include <cmath>

namespace client {

    namespace {

        constexpr int kMinutesPerDay = 1440; // 24 * 60 minutes in a day
        const std::array<const char*, 8> kWeekDays =
            {"", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"}; // index 1..7; index 0 unused

        /// @brief Loads ActivitiesWindow.xml into the screen and returns its window.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ActivitiesWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("ActivitiesWindow");
        }

        /// @brief Builds the element name of a row's query button.
        /// @param row Row 1 to 5.
        /// @return The element name.
        std::string queryName(int row) { return "ActivitiesQuery" + std::to_string(row); }

        /// @brief Builds a plain text style of one colour.
        /// @param color Text colour.
        /// @return The text style.
        gfx::TextStyle plain(core::Color color) { return {color, gfx::TextEffect::Plain, std::nullopt}; }

    }

    ActivitiesWindow::ActivitiesWindow(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        ActivityInfoWindow& info,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_messages(messages),
          m_info(info),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_queries{
              screen.get<ui::ImageButton>(queryName(1)),
              screen.get<ui::ImageButton>(queryName(2)),
              screen.get<ui::ImageButton>(queryName(3)),
              screen.get<ui::ImageButton>(queryName(4)),
              screen.get<ui::ImageButton>(queryName(5))
          },
          m_controls{
              screen.get<ui::ImageButton>("ActivitiesPrevious"),
              screen.get<ui::ImageButton>("ActivitiesNext"),
              screen.get<ui::ImageButton>("ActivitiesTransmit"),
              screen.get<ui::ImageButton>("ActivitiesLeave")
          },
          m_textSpot(screen.get<ui::Spot>("ActivitiesText")),
          m_textColorSpot(screen.get<ui::Spot>("ActivitiesTextColor")),
          m_processingColorSpot(screen.get<ui::Spot>("ActivitiesProcessingColor")),
          m_highlightSpot(screen.get<ui::Spot>("ActivitiesHighlight")) {
        screen.get<ui::DrawingArea>("ActivitiesPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onActivitiesClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onActivitiesPrevious", [this](const ui::EventArgs&) { turnPage(-1); });
        screen.on("onActivitiesNext", [this](const ui::EventArgs&) { turnPage(1); });
        screen.on("onActivitiesTransmit", [this](const ui::EventArgs&) { transmit(); });
        screen.on("onActivitiesQuery", [this](const ui::EventArgs& args) { query(args.source.back() - '0'); });
        screen.on("onActivitiesRowUp", [this](const ui::EventArgs&) {
            if (const int row = rowAt(m_screen.mouse())) {
                m_selected = row;
            }
        });
        screen.on("onActivitiesRowDouble", [this](const ui::EventArgs&) {
            if (m_hovered != 0) {
                transmit();
            }
        });
    }

    std::filesystem::path ActivitiesWindow::cacheFile() const { return m_services.game.user("ActionInfo.dat"); }

    void ActivitiesWindow::toggle() {
        if (isOpen()) {
            close();
        } else {
            open();
        }
    }

    void ActivitiesWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_hooks.blink(false);
        m_pageNumber = 1;
        refreshPage();
        m_hovered = 0;
        m_selected = 0;
    }

    void ActivitiesWindow::close() {
        m_window.setVisible(false);
        m_info.close();
    }

    void ActivitiesWindow::requestSchedule() {
        if (m_schedule.version() == 0) {
            m_schedule.load(cacheFile());
        }
        m_schedule.resetStates();
        m_hooks.send(protocol::activityListRequest(m_schedule.version()));
    }

    void ActivitiesWindow::scheduleReceived(const protocol::ActivityList& list) {
        m_hooks.blink(false);
        m_schedule.replace(list);
        bool running = false;
        for (int number = 1; number <= m_schedule.count(); ++number) {
            running = running || m_schedule.at(number).state == 2; // event state 2: processing (on)
        }
        if (running && !isOpen()) {
            m_hooks.blink(true);
        }
        m_schedule.save(cacheFile());
        refreshPage();
    }

    void ActivitiesWindow::statesReceived(const protocol::ActivityStates& states) {
        m_hooks.blink(false);
        if (m_schedule.applyStates(states) && !isOpen()) {
            m_hooks.blink(true);
        }
        refreshPage();
    }

    void ActivitiesWindow::noticeReceived(int kind) {
        if (kind == 0) { // notice 0: server busy
            m_messages.show("The server is busy now. Try later.", 2000); // message display time in ms
        } else if (kind == 1) { // notice 1: schedule updates
            m_messages.show("Event schedule updates", 2000); // message display time in ms
        }
    }

    void ActivitiesWindow::refreshPage() {
        m_page = {};
        for (const auto& button : m_queries) {
            button.get().setVisible(false);
        }
        const std::vector<int> listed = m_schedule.listed();
        m_pageCount = std::max(1, (static_cast<int>(listed.size()) + kRows - 1) / kRows);
        m_pageNumber = std::min(m_pageNumber, m_pageCount);
        const std::size_t first = static_cast<std::size_t>(m_pageNumber - 1) * kRows;
        for (std::size_t i = first; i < listed.size() && i < first + kRows; ++i) {
            const protocol::ActivityRecord& activity = m_schedule.at(listed[i]);
            Row& row = m_page[i - first];
            row.number = listed[i];
            row.name = activity.name;
            if (activity.time - std::floor(activity.time) + activity.duration / static_cast<double>(kMinutesPerDay) <=
                1.0) { // 1.0: the event must end by the end of its day
                // substr(11, 5): the "HH:MM" part of the date-time text
                row.start = core::formatDateTime(activity.time).substr(11, 5);
                row.end = core::formatDateTime(activity.duration / static_cast<double>(kMinutesPerDay) + activity.time).substr(11, 5);
                if (row.end == "00:00") {
                    row.end = "24:00"; // midnight end shown as 24:00
                }
            }
            row.state = activity.state;
            m_queries[i - first].get().setVisible(true);
        }
    }

    int ActivitiesWindow::rowAt(core::Point screenPoint) const {
        const core::Point local = screenPoint - core::Point{m_window.bounds().x, m_window.bounds().y};
        for (int row = 0; row < kRows; ++row) {
            const core::Rect area{24, 85 + row * 39, 485, 36}; // row hit rectangle: x 24, y 85, row pitch 39, width 485, height 36
            if (area.contains(local) && m_page[static_cast<std::size_t>(row)].number != 0) {
                return row + 1;
            }
        }
        return 0;
    }

    void ActivitiesWindow::turnPage(int step) {
        const int page = m_pageNumber + step;
        if (page < 1 || page > m_pageCount) {
            return;
        }
        m_selected = 0;
        m_pageNumber = page;
        refreshPage();
        m_info.close();
    }

    void ActivitiesWindow::query(int row) {
        if (m_queried == row && m_info.isOpen()) {
            m_info.close();
            return;
        }
        m_queried = row;
        const Row& chosen = m_page[static_cast<std::size_t>(row - 1)];
        if (chosen.number == 0) {
            return;
        }
        const protocol::ActivityRecord& activity = m_schedule.at(chosen.number);
        std::string location = "None";
        if (activity.mapId > 0) {
            location = m_hooks.sceneName(activity.mapId) + "(" + std::to_string(activity.mapX) + "," + std::to_string(activity.mapY) + ")";
        }
        const core::Rect button = m_queries[static_cast<std::size_t>(row - 1)].get().bounds();
        m_info.show(activity, location, {m_window.bounds().x + button.x, m_window.bounds().y + button.y, button.width, button.height});
    }

    void ActivitiesWindow::transmit() {
        if (m_selected == 0) {
            m_messages.show("Playing", 2000); // message display time in ms
            return;
        }
        const protocol::ActivityRecord& activity = m_schedule.at(m_page[static_cast<std::size_t>(m_selected - 1)].number);
        if (activity.state != 2) { // event state 2: processing (on)
            m_messages.show("Teleport failed. The event isn't available yet", 2000); // message display time in ms
            return;
        }
        if (activity.mapId == 0 || activity.mapX == 0 || activity.mapY == 0) { // a missing map or coordinate (0) means no teleport target
            m_messages.show("This event prohibits teleportation.", 2000); // message display time in ms
            return;
        }
        m_hooks.ask("Go to the event zone?", [this] { teleport(); });
    }

    void ActivitiesWindow::teleport() {
        if (m_hooks.teleportRefused()) {
            return;
        }
        if (m_selected == 0) {
            m_messages.show("Playing", 2000); // message display time in ms
            return;
        }
        const int number = m_page[static_cast<std::size_t>(m_selected - 1)].number;
        const protocol::ActivityRecord& activity = m_schedule.at(number);
        const char* const notYet = "Teleport failed. The event isn't available yet";
        if (activity.weekDay != 0 && activity.weekDay != m_schedule.weekDay()) {
            m_messages.show(notYet, 2000); // message display time in ms
            return;
        }
        if (activity.state != 2) { // event state 2: processing (on)
            m_messages.show(notYet, 2000); // message display time in ms
            return;
        }
        if (m_hooks.ownLevel() < activity.minLevel) {
            m_messages.show("Teleport failed. Your level is too low. ", 2000); // message display time in ms
            return;
        }
        m_hooks.send(protocol::activityTeleportRequest(number));
        close();
    }

    void ActivitiesWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const core::Rect form{m_window.bounds().x, m_window.bounds().y, m_window.bounds().width, m_window.bounds().height};
        if (form.contains(m_screen.mouse())) {
            m_hovered = rowAt(m_screen.mouse());
        }
        const bool dayKnown = m_schedule.weekDay() >= 1 && m_schedule.weekDay() <= 7; // weekdays 1 (Monday) to 7 (Sunday)
        for (const auto& control : m_controls) {
            control.get().setVisible(dayKnown);
        }
        if (!dayKnown) {
            return;
        }
        // highlight rectangle: x 21, y 45 + row * 41 (row pitch 41), width 481, height 36
        const auto highlight = [&](int row) { canvas.fillRectAlpha({at.x + 21, at.y + row * 41 + 45, 481, 36}, m_highlightSpot.color()); };
        if (m_selected != 0) {
            highlight(m_selected);
        }
        if (m_hovered != 0 && m_hovered != m_selected) {
            highlight(m_hovered);
        }
        // weekday name at x 20, y 49
        canvas.drawText(
            {at.x + 20, at.y + 49},
            kWeekDays[static_cast<std::size_t>(m_schedule.weekDay())],
            plain(m_textColorSpot.color()),
            m_textSpot.size().width,
            m_textSpot.size().height
        );
        for (int row = 0; row < kRows; ++row) {
            const Row& shown = m_page[static_cast<std::size_t>(row)];
            if (shown.name.empty()) {
                continue;
            }
            // row text y: 97 + row * 41 (row pitch 41); columns x: name 25, start 265, end 359, state 419
            const int y = at.y + 97 + row * 41;
            canvas.drawText({at.x + 25, y}, shown.name, plain(m_textColorSpot.color()), m_textSpot.size().width, m_textSpot.size().height);
            canvas.drawText({at.x + 265, y}, shown.start, plain(m_textColorSpot.color()), m_textSpot.size().width, m_textSpot.size().height);
            canvas.drawText({at.x + 359, y}, shown.end, plain(m_textColorSpot.color()), m_textSpot.size().width, m_textSpot.size().height);
            if (shown.state == 1) { // event state 1: preparing
                canvas.drawText({at.x + 419, y}, "Preparing", plain(m_textColorSpot.color()), m_textSpot.size().width, m_textSpot.size().height);
            } else if (shown.state == 2) { // event state 2: processing (on)
                canvas.drawText({at.x + 419, y}, "Processing", plain(m_processingColorSpot.color()), m_textSpot.size().width, m_textSpot.size().height);
            }
        }
        const std::string page = std::to_string(m_pageNumber);
        // page number: right-aligned at x 262 (8 pixels per extra digit), y 303; the page count at x 270
        canvas.drawText(
            {at.x + 262 - (static_cast<int>(page.size()) - 1) * 8, at.y + 303},
            page,
            plain(m_textColorSpot.color()),
            m_textSpot.size().width,
            m_textSpot.size().height
        );
        canvas.drawText(
            {at.x + 270, at.y + 303},
            "/" + std::to_string(m_pageCount),
            plain(m_textColorSpot.color()),
            m_textSpot.size().width,
            m_textSpot.size().height
        );
    }

}
