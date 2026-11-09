#include "client/viewmodels/minigames/CompassWindow.h"

#include "client/rendering/hud/GameNumbers.h"

#include <string>

namespace client {

    namespace {

        constexpr int kLivesNumberStyle = 10; // game number size style 10: 14x19 px digits

        /// @brief Loads CompassWindow.xml and returns its drawing area.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "CompassPage" drawing area.
        ui::DrawingArea& loadPage(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("CompassWindow.xml"), services.loadContext());
            return screen.get<ui::DrawingArea>("CompassPage");
        }

    }

    CompassWindow::CompassWindow(
        GameServices& services,
        ui::Screen& screen,
        CompassGame::Hooks hooks,
        std::function<const CharacterLook&()> ownLook
    )
        : m_services(services),
          m_ownLook(std::move(ownLook)),
          m_game(std::move(hooks)),
          m_page(loadPage(services, screen)),
          m_avatarSpot(screen.get<ui::Spot>("CompassAvatar")),
          m_multiplySpot(screen.get<ui::Spot>("CompassMultiply")),
          m_livesSpot(screen.get<ui::Spot>("CompassLives")),
          m_participantsSpot(screen.get<ui::Spot>("CompassParticipants")),
          m_ownPlaceSpot(screen.get<ui::Spot>("CompassOwnPlace")) {
        m_page.setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
    }

    void CompassWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (canvas.isAnimationFrame()) {
            m_game.leaveWhenOffMap();
        }
        m_services.characters.drawLifeFace(canvas, m_ownLook(), m_avatarSpot.at());
        canvas.drawText(
            m_multiplySpot.at(),
            "x",
            {m_multiplySpot.color(), gfx::TextEffect::Outline, std::nullopt},
            16,
            16
        ); // width and height of the "x" text box
        drawGameNumber(canvas, "Num_White5_1", m_livesSpot.at(), m_game.lives(), kLivesNumberStyle, true);
        m_game.step(nowMs);
        for (int place = 1; place <= m_game.playerCount(); ++place) {
            const std::string& name = m_game.playerName(place);
            const core::Point at{m_participantsSpot.at().x, (place - 1) * m_participantsSpot.pitch().y + m_participantsSpot.at().y};
            canvas.drawText(
                at,
                name.empty() ? "(Participants left)" : name,
                {m_game.isOwnPlace(place) ? m_ownPlaceSpot.color() : m_participantsSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                144,
                16
            ); // width and height of a name text box
        }
    }

}
