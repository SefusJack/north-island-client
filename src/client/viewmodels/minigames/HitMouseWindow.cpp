#include "client/viewmodels/minigames/HitMouseWindow.h"

#include "client/rendering/hud/GameNumbers.h"

namespace client {

    namespace {

        constexpr int kNumberStyle = 3; // drawGameNumber size style of the time and score numbers
        constexpr int kCountdownStyle = 5; // drawGameNumber size style of the countdown digits
        constexpr int kFrames = 5; // frames stacked in a target or hit picture
        constexpr int kBombFrames = 4; // frames of the Bomb_2 explosion light

        /// @brief The pictures of a look: what comes up, what it does when hit and the bomb.
        struct Looks {
            const char* target;
            const char* hit;
            const char* bomb;
        };

        constexpr Looks kLooks[] = {{"Mouse_1", "Mouse_2", "Bomb_1"}, {"Rabit_1", "Rabit_2", "RabitBomb"}, {"HiTurtle_1", "HiTurtle_2", "TurtleBomb"}};
        // per look (mouse, rabbit, turtle): x, y offsets of the target, hit and bomb pictures from the hole position, px
        constexpr core::Point kTargetOffsets[] = {{16, 42}, {30, 42}, {14, 49}};
        constexpr core::Point kHitOffsets[] = {{16, 45}, {25, 27}, {14, 46}};
        constexpr core::Point kBombOffsets[] = {{30, 51}, {30, 51}, {30, 53}};

        /// @brief Loads HitMouseWindow.xml and returns its form.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "HitMouseForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("HitMouseWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("HitMouseForm");
        }

    }

    HitMouseWindow::HitMouseWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks
    )
        : m_services(services),
          m_game(std::move(hooks)),
          m_form(loadForm(services, screen)),
          m_page(screen.get<ui::DrawingArea>("HitMousePage")),
          m_exit(screen.get<ui::ImageButton>("HitMouseExit")),
          m_timeSpot(screen.get<ui::Spot>("HitMouseTime")),
          m_timeNumberSpot(screen.get<ui::Spot>("HitMouseTimeNumber")),
          m_scoreSpot(screen.get<ui::Spot>("HitMouseScore")),
          m_scoreNumberSpot(screen.get<ui::Spot>("HitMouseScoreNumber")),
          m_messageSpot(screen.get<ui::Spot>("HitMouseMessage")) {
        m_page.setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.on("onHitMouseUp", [this](const ui::EventArgs& args) { released(args); });
        screen.on("onHitMouseLeave", [this](const ui::EventArgs&) { m_game.leave(); });
    }

    void HitMouseWindow::open(int npcId, std::uint64_t nowMs) {
        m_game.open(npcId, nowMs);
        m_light.stop();
        m_exit.setVisible(false);
        m_page.setCursor(ui::CursorKind::Mallet);
        m_form.setVisible(true);
    }

    void HitMouseWindow::begin() {
        m_game.begin();
        m_exit.setVisible(true);
    }

    void HitMouseWindow::close() {
        m_form.setVisible(false);
        m_light.stop();
    }

    void HitMouseWindow::released(const ui::EventArgs& args) {
        const HitMouseGame::Strike strike = m_game.released(args.position, m_frameMs);
        if (strike.kind == HitMouseGame::Strike::Kind::None) {
            return;
        }
        LightEffect::Setup setup;
        setup.motion = LightEffect::Motion::Once;
        setup.loop = false;
        if (strike.kind == HitMouseGame::Strike::Kind::Target) {
            setup.picture = m_services.pictures.find("S10416").value_or(gfx::PictureId{});
            // hit light of picture S10416: 70 ms per frame, frames 4 to 9 of 9, alpha 30 (of 32)
            setup.frameMs = 70;
            setup.firstFrame = 4;
            setup.lastFrame = 9;
            setup.frameCount = 9;
            setup.alpha = 30;
        } else {
            setup.picture = m_services.pictures.find("Bomb_2").value_or(gfx::PictureId{});
            // bomb light: 100 ms per frame, frames 1 to 4
            setup.frameMs = 100;
            setup.firstFrame = 1;
            setup.lastFrame = 4;
            setup.frameCount = kBombFrames;
        }
        m_light.start(setup, strike.at, m_frameMs);
    }

    void HitMouseWindow::drawHole(gfx::Canvas& canvas, const HitMouseGame::Hole& hole) const {
        const Looks& names = kLooks[static_cast<int>(m_game.look())];
        const auto draw = [&](const char* name, int frame, core::Point offset) {
            const auto picture = m_services.pictures.find(name);
            if (!picture || frame < 1) {
                return;
            }
            const gfx::Picture& data = m_services.pictures.picture(*picture);
            const int height = data.height() / kFrames;
            canvas.drawPicture(*picture, {hole.position.x + offset.x, hole.position.y - offset.y}, core::Rect{0, (frame - 1) * height, data.width(), height});
        };
        const int look = static_cast<int>(m_game.look());
        // hole state 1..3: a target or bomb is coming up; state 4: the target was hit
        if (hole.state >= 1 && hole.state <= 3 && hole.animFrame != 0) {
            if (hole.bomb) {
                draw(names.bomb, hole.animFrame, kBombOffsets[look]);
            } else {
                draw(names.target, hole.animFrame, kTargetOffsets[look]);
            }
        } else if (hole.state == 4 && hole.hitFrame != 0) {
            draw(names.hit, hole.hitFrame, kHitOffsets[look]);
        }
    }

    void HitMouseWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        m_frameMs = nowMs;
        if (canvas.isAnimationFrame()) {
            m_game.step(nowMs);
        }
        m_page.setCursor(
            m_game.hammer() == HitMouseGame::Hammer::Down ? ui::CursorKind::MalletDown
            : m_game.hammer() == HitMouseGame::Hammer::Up ? ui::CursorKind::MalletUp
                                                          : ui::CursorKind::Mallet
        );
        if (m_game.state() == 3) { // game state 3: nothing is drawn
            return;
        }
        gfx::PictureLibrary& pictures = m_services.pictures;
        if (const auto field = pictures.find("HitMouse")) {
            canvas.drawPicture(*field, {0, 0});
        }
        if (const auto hole = pictures.find("Hole")) {
            for (int index = 1; index <= HitMouseGame::kHoles; ++index) {
                canvas.drawPicture(*hole, m_game.hole(index).position);
            }
        }
        for (int index = 1; index <= HitMouseGame::kHoles; ++index) {
            drawHole(canvas, m_game.hole(index));
        }
        m_light.draw(canvas, nowMs);
        if (const auto time = pictures.find("MiniGame_Time_1")) {
            canvas.drawPicture(*time, m_timeSpot.at());
        }
        drawGameNumber(canvas, "Num_White5_1", m_timeNumberSpot.at(), m_game.timeLeft(), kNumberStyle, true);
        if (const auto score = pictures.find("MiniGame_Score_1")) {
            canvas.drawPicture(*score, m_scoreSpot.at());
        }
        drawGameNumber(canvas, "Num_White5_1", m_scoreNumberSpot.at(), m_game.score(), kNumberStyle, true);
        if (m_game.state() == 0) { // game state 0: the countdown
            if (m_game.countdown() == 0) {
                drawGameLetters(canvas, m_messageSpot.at(), "Go", 0, 1.0F, m_messageSpot.pitch().x, 0, true); // 1.0F is the letter scale
            } else if (m_game.countdown() >= 1 && m_game.countdown() <= 3) { // countdown numbers 3, 2, 1
                drawGameNumber(canvas, "Num_YellowRed10_2", m_messageSpot.at(), m_game.countdown(), kCountdownStyle, true);
            }
        }
    }

}
