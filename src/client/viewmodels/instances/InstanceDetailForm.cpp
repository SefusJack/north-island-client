#include "client/viewmodels/instances/InstanceDetailForm.h"

#include "ui/DrawingArea.h"

namespace client {

    namespace {

        constexpr int kPlayersRight = 156; // player count sits this many px right of the info column
        constexpr int kDescriptionLeft = -82; // description starts this many px left of the info column
        constexpr int kDescriptionDown = 20; // extra px below the last info line for the description
        constexpr int kGlyphWidth = 8; // width of one character, px (used to centre the title)

        /// @brief Loads the form's layout and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen the form is loaded into.
        /// @return The form's window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("InstanceDetailForm.xml"), services.loadContext());
            return screen.get<ui::Window>("InstanceDetailForm");
        }

    }

    InstanceDetailForm::InstanceDetailForm(
        GameServices& services,
        ui::Screen& screen,
        Instances& instances
    )
        : m_instances(instances),
          m_screen(screen),
          m_window(loadWindow(services, screen)),
          m_members(screen.get<ui::ListBox>("InstanceDetailMembers")),
          m_infoSpot(screen.get<ui::Spot>("InstanceDetailInfo")),
          m_titleSpot(screen.get<ui::Spot>("InstanceDetailTitle")) {
        screen.get<ui::DrawingArea>("InstanceDetailTexts").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onInstanceDetailClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onInstanceDetailJoin", [this](const ui::EventArgs&) { m_instances.joinFromDetail(); });
        screen.on("onInstanceDetailUp", [this](const ui::EventArgs&) { m_members.scrollUp(); });
        screen.on("onInstanceDetailDown", [this](const ui::EventArgs&) { m_members.scrollDown(); });
    }

    void InstanceDetailForm::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void InstanceDetailForm::syncMembers() {
        std::vector<std::int32_t> ids;
        for (const InstanceMember& member : m_instances.detailMembers()) {
            ids.push_back(member.id);
        }
        if (ids == m_listed) {
            return;
        }
        m_listed = ids;
        m_members.clear();
        for (const InstanceMember& member : m_instances.detailMembers()) {
            m_members.addItem(member.name);
        }
    }

    void InstanceDetailForm::paint(gfx::Canvas& canvas, core::Point at) {
        syncMembers();
        const InstanceInfo& info = m_instances.detail();
        const gfx::TextStyle style{m_infoSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const auto text = [&](const std::string& line, int x, int y) {
            canvas.drawText({at.x + x, at.y + y}, line, style, m_infoSpot.size().width, m_infoSpot.size().height);
        };
        const int titleHalf = static_cast<int>(info.title.size()) / 2 * kGlyphWidth;
        text(info.title, m_titleSpot.at().x - titleHalf, m_titleSpot.at().y);
        text(info.minLevel, m_infoSpot.at().x, m_infoSpot.at().y - 1); // minimum level line, nudged 1 px up
        text(std::to_string(info.members) + "/" + info.maxPlayers, m_infoSpot.at().x + kPlayersRight, m_infoSpot.at().y);
        text(info.guildOnly, m_infoSpot.at().x, m_infoSpot.at().y + m_infoSpot.pitch().y);
        text(info.timeLimit, m_infoSpot.at().x, m_infoSpot.at().y + 2 * m_infoSpot.pitch().y - 3); // third info line (2 lines down), nudged 3 px up
        text(info.description, m_infoSpot.at().x + kDescriptionLeft, m_infoSpot.at().y + 3 * m_infoSpot.pitch().y + kDescriptionDown); // 3 lines down
    }

}
