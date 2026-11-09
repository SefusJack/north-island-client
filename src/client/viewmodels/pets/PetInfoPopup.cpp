#include "client/viewmodels/pets/PetInfoPopup.h"

#include "client/rules/PetInfoLines.h"
#include "gfx/TextStyle.h"
#include "ui/DrawingArea.h"

namespace client {

    namespace {

        constexpr int kScreenWidth = 800; // game screen width, px
        constexpr int kScreenHeight = 600; // game screen height, px

        /// @brief Loads PetInfoPopup.xml into the screen and returns the popup panel.
        /// @param services Shared game services (layout and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The PetInfo panel element.
        ui::Element& loadPopup(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PetInfoPopup.xml"), services.loadContext());
            return screen.get<ui::Element>("PetInfo");
        }

    }

    PetInfoPopup::PetInfoPopup(
        GameServices& services,
        ui::Screen& screen
    )
        : m_services(services),
          m_panel(loadPopup(services, screen)),
          m_widthSpot(screen.get<ui::Spot>("PetInfoWidth")),
          m_textSpot(screen.get<ui::Spot>("PetInfoText")) {
        screen.get<ui::DrawingArea>("PetInfoLines").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
    }

    void PetInfoPopup::show(const protocol::PetInfoBlock& pet, int maxHp, int maxSp, core::Point mouse) {
        m_lines = petInfoLines(pet, maxHp, maxSp, m_services.npcs.find(pet.npcId), [this](int skillId) {
            const auto skill = m_services.skills.find(skillId);
            return skill ? skill->name : std::string();
        });
        core::Rect box{
            mouse.x,
            mouse.y,
            m_widthSpot.size().width,
            static_cast<int>(m_lines.size()) * m_textSpot.rect().height + 3 * m_textSpot.rect().x
        }; // two margins plus one extra margin of bottom padding
        if (box.x + box.width > kScreenWidth) {
            box.x = kScreenWidth - box.width;
        }
        if (box.y + box.height > kScreenHeight) {
            box.y = kScreenHeight - box.height;
        }
        m_panel.setBounds(box);
        m_panel.children().front()->setBounds({0, 0, box.width, box.height});
        m_panel.setVisible(true);
    }

    void PetInfoPopup::hide() { m_panel.setVisible(false); }

    void PetInfoPopup::paint(gfx::Canvas& canvas, core::Point at) {
        const gfx::TextStyle style{m_textSpot.color(), gfx::TextEffect::Shadow, std::nullopt};
        for (std::size_t i = 0; i < m_lines.size(); ++i) {
            canvas.drawText(
                {at.x + m_textSpot.rect().x, at.y + m_textSpot.rect().x + static_cast<int>(i) * m_textSpot.rect().height},
                m_lines[i],
                style,
                m_textSpot.rect().width,
                m_textSpot.rect().height
            );
        }
    }

}
