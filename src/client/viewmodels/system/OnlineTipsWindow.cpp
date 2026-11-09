#include "client/viewmodels/system/OnlineTipsWindow.h"

#include "core/FileSystem.h"
#include "core/StringUtil.h"
#include "ui/ImageButton.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr std::size_t kTextStart = 4; // text of a hotkey-help line starts at character 4 (after a 3-digit line number and a separator)

        /// @brief Loads OnlineTipsWindow.xml into the screen.
        /// @param services Shared game services (layout path and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The OnlineTips window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("OnlineTipsWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("OnlineTips");
        }

        /// @brief Reads a text file as a list of lines.
        /// @param file Path of the file.
        /// @return The lines, empty when the file cannot be read.
        std::vector<std::string> readLines(const std::filesystem::path& file) {
            const auto data = core::readFile(file);
            return data ? core::splitLines(std::string(data->begin(), data->end())) : std::vector<std::string>{};
        }

    }

    OnlineTipsWindow::OnlineTipsWindow(
        GameServices& services,
        ui::Screen& screen
    )
        : m_screen(screen),
          m_window(loadWindow(services, screen)),
          m_topics(screen.get<ui::ListBox>("OnlineTipsTopics")),
          m_text(screen.get<ui::Memo>("OnlineTipsText")),
          m_closeTopicsSpot(screen.get<ui::Spot>("OnlineTipsCloseTopics")),
          m_closeTextSpot(screen.get<ui::Spot>("OnlineTipsCloseText")) {
        for (const std::string& line : readLines(services.game.data("txt\\hotkeyt.txt"))) {
            m_topicLines.push_back(core::parseInt(line.substr(0, 3)).value_or(0)); // the line number is the first 3 characters
            if (!line.empty()) {
                m_topics.addItem(line.size() > kTextStart ? line.substr(kTextStart) : std::string());
            }
        }
        for (const std::string& line : readLines(services.game.data("txt\\hotkeye.txt"))) {
            m_lines.push_back(line.size() > kTextStart ? line.substr(kTextStart) : std::string());
        }
        if (!m_lines.empty()) {
            m_text.appendLine(m_lines.front());
        }
        screen.on("onOnlineTipsTopic", [this](const ui::EventArgs& args) { showTopic(args.index); });
        screen.on("onOnlineTipsBack", [this](const ui::EventArgs&) { showTopics(); });
        screen.on("onOnlineTipsPageUp", [this](const ui::EventArgs&) { turnTopics(-1); });
        screen.on("onOnlineTipsPageDown", [this](const ui::EventArgs&) { turnTopics(1); });
        screen.on("onOnlineTipsTextUp", [this](const ui::EventArgs&) { m_text.pageUp(); });
        screen.on("onOnlineTipsTextDown", [this](const ui::EventArgs&) { m_text.pageDown(); });
        screen.on("onOnlineTipsClose", [this](const ui::EventArgs&) { m_window.setVisible(false); });
    }

    void OnlineTipsWindow::toggle() {
        if (m_window.isVisible()) {
            m_window.setVisible(false);
            return;
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void OnlineTipsWindow::showTopic(int index) {
        if (index < 0 || index > m_topics.itemCount() - 1 || index >= static_cast<int>(m_topicLines.size())) {
            return;
        }
        m_text.clear();
        const int first = m_topicLines[static_cast<std::size_t>(index)] - 1;
        const int last = index == m_topics.itemCount() - 1 || index + 1 >= static_cast<int>(m_topicLines.size())
                             ? static_cast<int>(m_lines.size()) - 1
                             : m_topicLines[static_cast<std::size_t>(index) + 1] - 2; // next topic's 1-based first line minus 1, as a 0-based index
        for (int line = std::max(0, first); line <= last && line < static_cast<int>(m_lines.size()); ++line) {
            m_text.appendLine(m_lines[static_cast<std::size_t>(line)]);
        }
        showTextControls(true);
    }

    void OnlineTipsWindow::showTopics() { showTextControls(false); }

    void OnlineTipsWindow::showTextControls(bool text) {
        m_topics.setVisible(!text);
        m_screen.get<ui::ImageButton>("OnlineTipsPageUp").setVisible(!text);
        m_screen.get<ui::ImageButton>("OnlineTipsPageDown").setVisible(!text);
        m_text.setVisible(text);
        m_screen.get<ui::ImageButton>("OnlineTipsBack").setVisible(text);
        m_screen.get<ui::ImageButton>("OnlineTipsTextUp").setVisible(text);
        m_screen.get<ui::ImageButton>("OnlineTipsTextDown").setVisible(text);
        m_screen.get<ui::ImageButton>("OnlineTipsClose").setPosition(text ? m_closeTextSpot.at() : m_closeTopicsSpot.at());
    }

    void OnlineTipsWindow::turnTopics(int step) {
        const int page = m_topics.visibleRows();
        const int count = std::max(m_topics.itemCount(), page);
        if (count == page) {
            return;
        }
        if (step < 0) {
            m_topics.setTopIndex(std::max(0, m_topics.topIndex() - page));
        } else if (m_topics.topIndex() + page < count) {
            m_topics.setTopIndex(m_topics.topIndex() + page);
        }
    }

}
