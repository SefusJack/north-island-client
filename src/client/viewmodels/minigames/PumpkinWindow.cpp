#include "client/viewmodels/minigames/PumpkinWindow.h"

#include "client/rendering/characters/FigureShadow.h"
#include "client/rendering/hud/GameNumbers.h"
#include "core/Between.h"

namespace client {

    namespace {

        constexpr int kGhostNpc = 15544; // NPC 15544: Barbarian Tribe (the sprite used for the ghosts)
        constexpr int kNumberStyle = 3; // game-number digit style for score and time
        constexpr int kCountdownStyle = 5; // game-number digit style for the start countdown
        constexpr int kAimWidth = 10; // aim line width, px
        constexpr int kSnapPixels = 40; // moves longer than this many px are drawn without interpolation
        constexpr std::uint64_t kHitLightFrameMs = 70; // hit light frame duration, ms
        constexpr int kHitLightFirst = 4; // first frame of the hit light (S10416)
        constexpr int kHitLightFrames = 9; // last frame / frame count of the hit light
        constexpr int kHitLightAlpha = 30; // light-blend strength of the hit light

        /// @brief Loads PumpkinWindow.xml into the screen and returns its form.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The "PumpkinForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PumpkinWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("PumpkinForm");
        }

    }

    PumpkinWindow::PumpkinWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks,
        std::function<bool()> instructionsOpen,
        std::function<const CharacterLook&()> playerLook
    )
        : m_services(services),
          m_hooks(std::move(hooks)),
          m_instructionsOpen(std::move(instructionsOpen)),
          m_playerLook(std::move(playerLook)),
          m_form(loadForm(services, screen)),
          m_quit(screen.get<ui::ImageButton>("PumpkinQuit")),
          m_fieldSpot(screen.get<ui::Spot>("PumpkinField")),
          m_frontSpot(screen.get<ui::Spot>("PumpkinFront")),
          m_heartSpot(screen.get<ui::Spot>("PumpkinHeart")),
          m_lifeSpot(screen.get<ui::Spot>("PumpkinLife")),
          m_scoreSpot(screen.get<ui::Spot>("PumpkinScore")),
          m_scoreNumberSpot(screen.get<ui::Spot>("PumpkinScoreNumber")),
          m_timeSpot(screen.get<ui::Spot>("PumpkinTime")),
          m_timeNumberSpot(screen.get<ui::Spot>("PumpkinTimeNumber")),
          m_messageSpot(screen.get<ui::Spot>("PumpkinMessage")),
          m_aimSpot(screen.get<ui::Spot>("PumpkinAim")),
          m_coinSpot(screen.get<ui::Spot>("PumpkinCoin")) {
        screen.get<ui::DrawingArea>("PumpkinPage").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.on("onPumpkinDown", [this](const ui::EventArgs&) { m_game->mouseDown(); });
        screen.on("onPumpkinQuit", [this](const ui::EventArgs&) { m_game->exitGame(); });
    }

    PumpkinGame::Hooks PumpkinWindow::makeHooks() {
        PumpkinGame::Hooks hooks;
        static_cast<GameHooks&>(hooks) = m_hooks;
        hooks.instructionsOpen = m_instructionsOpen;
        hooks.pictureSize = [this](const std::string& name) {
            const auto picture = m_services.pictures.find(name);
            return picture ? core::Size{m_services.pictures.picture(*picture).width(), m_services.pictures.picture(*picture).height()} : core::Size{};
        };
        hooks.hitLight = [this](core::Point at) {
            LightEffect::Setup setup;
            setup.picture = m_services.pictures.find("S10416").value_or(gfx::PictureId{});
            setup.frameMs = kHitLightFrameMs;
            setup.firstFrame = kHitLightFirst;
            setup.lastFrame = kHitLightFrames;
            setup.frameCount = kHitLightFrames;
            setup.motion = LightEffect::Motion::Once;
            setup.loop = false;
            setup.alpha = kHitLightAlpha;
            m_light.start(setup, at, m_frameMs);
        };
        hooks.playerFrames = [this](int slot) { return m_services.characters.figureFrameCount(m_playerLook(), slot); };
        hooks.ghostFrames = [this](int slot) { return m_ghost ? m_services.characters.npcFrameCount(m_ghost->spriteId, slot) : 0; };
        hooks.random = [this](int count) { return std::uniform_int_distribution<int>(0, count - 1)(m_random); };
        return hooks;
    }

    void PumpkinWindow::open(std::uint64_t nowMs) {
        m_ghost = m_services.npcs.find(kGhostNpc);
        m_game = std::make_unique<PumpkinGame>(makeHooks());
        m_game->open(nowMs);
        m_light.stop();
        m_frameMs = nowMs;
        m_quit.setVisible(false);
        m_form.setVisible(true);
    }

    void PumpkinWindow::close() {
        m_form.setVisible(false);
        m_light.stop();
    }

    void PumpkinWindow::leave() { m_game->exitGame(); }

    void PumpkinWindow::begin(std::uint64_t) { m_game->begin(); }

    void PumpkinWindow::drawField(gfx::Canvas& canvas) {
        gfx::PictureLibrary& pictures = m_services.pictures;
        if (const auto field = pictures.find("59028")) {
            canvas.drawPicture(*field, m_fieldSpot.at());
        }
        if (const auto front = pictures.find("59027")) {
            canvas.drawPicture(*front, m_frontSpot.at());
        }
        drawFigureShadow(canvas, PumpkinGame::kPlayerAt, 0);
        m_services.characters.drawFigure(canvas, m_playerLook(), PumpkinGame::kPlayerAt, 12, m_game->playerFrame()); // figure slot 12 for the player's pose
    }

    void PumpkinWindow::drawGhosts(gfx::Canvas& canvas, double amount) {
        gfx::PictureLibrary& pictures = m_services.pictures;
        if (m_game->bombShown()) {
            if (const auto shot = pictures.find("1268")) {
                const gfx::Picture& data = pictures.picture(*shot);
                const core::Point at =
                    m_game->previousBombShown() ? core::between(m_game->previousBombAt(), m_game->bombAt(), amount, kSnapPixels) : m_game->bombAt();
                canvas.drawPicture(*shot, {at.x - data.width() / 2, at.y - data.height() / 2});
            }
        }
        const auto coin = pictures.find("1290");
        for (int index = 1; index <= PumpkinGame::kGhosts; ++index) {
            const PumpkinGhost& ghost = m_game->ghosts()[static_cast<std::size_t>(index)];
            if (ghost.shown != PumpkinGhost::Shown::Walking || !m_ghost) {
                continue;
            }
            const core::Point at =
                ghost.previousShown == PumpkinGhost::Shown::Walking ? core::between(ghost.previousShownAt, ghost.shownAt, amount, kSnapPixels) : ghost.shownAt;
            drawFigureShadow(canvas, at, m_ghost->shadow);
            m_services.characters.drawNpc(canvas, *m_ghost, at, ghost.shownSlot, ghost.shownFrame);
            if (ghost.shownCoin && coin) {
                canvas.drawPicture(*coin, {at.x - m_coinSpot.at().x, at.y - m_coinSpot.at().y});
            }
        }
    }

    void PumpkinWindow::drawStatus(gfx::Canvas& canvas) {
        gfx::PictureLibrary& pictures = m_services.pictures;
        if (const auto heart = pictures.find("icon_Heart")) {
            for (int j = 1; j <= m_game->hearts(); ++j) {
                canvas.drawPictureScaled(
                    *heart,
                    {j * m_heartSpot.pitch().x + m_heartSpot.rect().x, m_heartSpot.rect().y, m_heartSpot.rect().width, m_heartSpot.rect().height}
                );
            }
        }
        if (const auto score = pictures.find("MiniGame_Score_1")) {
            canvas.drawPicture(*score, m_scoreSpot.at());
        }
        drawGameNumber(canvas, "Num_White5_1", m_scoreNumberSpot.at(), m_game->score(), kNumberStyle, true);
        if (const auto time = pictures.find("MiniGame_Time_1")) {
            canvas.drawPicture(*time, m_timeSpot.at());
        }
        drawGameNumber(canvas, "Num_White5_1", m_timeNumberSpot.at(), m_game->timeLeft(), kNumberStyle, true);
        if (const auto life = pictures.find("MiniGame_Life_1")) {
            canvas.drawPicture(*life, m_lifeSpot.at());
        }
        if (m_game->phase() == PumpkinGame::Phase::Countdown) {
            if (m_game->startCountdown() == 0) {
                drawGameLetters(canvas, m_messageSpot.at(), "Go", 0, 1.0F, m_messageSpot.pitch().x, 0, true);
            } else if (m_game->startCountdown() >= 1 && m_game->startCountdown() <= 3) { // countdown numbers 3, 2, 1 are shown as digits
                drawGameNumber(canvas, "Num_YellowRed10_2", m_messageSpot.at(), m_game->startCountdown(), kCountdownStyle, true);
            }
        }
        if (m_game->phase() == PumpkinGame::Phase::Over) {
            drawGameLetters(canvas, m_messageSpot.at(), m_game->overText(), 0, 1.0F, m_messageSpot.pitch().x, 0, true);
        }
    }

    void PumpkinWindow::drawAimLine(gfx::Canvas& canvas, double amount) {
        if (m_game->phase() != PumpkinGame::Phase::Playing) {
            return;
        }
        const core::Point tip = core::between(m_game->previousAimTip(), m_game->aimTip(), amount, kSnapPixels);
        const core::Point barbA = core::between(m_game->previousAimBarbA(), m_game->aimBarbA(), amount, kSnapPixels);
        const core::Point barbB = core::between(m_game->previousAimBarbB(), m_game->aimBarbB(), amount, kSnapPixels);
        canvas.drawLine(PumpkinGame::kPlayerAt, tip, kAimWidth, m_aimSpot.color());
        canvas.drawLine(tip, barbA, kAimWidth, m_aimSpot.color());
        canvas.drawLine(tip, barbB, kAimWidth, m_aimSpot.color());
    }

    void PumpkinWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (!m_game) {
            return;
        }
        m_frameMs = nowMs;
        m_game->update(nowMs);
        if (!m_game || !m_form.isVisible()) {
            return;
        }
        m_quit.setVisible(m_game->formShown());
        const double amount = canvas.betweenBeats() ? m_game->sinceFrame(nowMs) : 1.0;
        drawField(canvas);
        drawGhosts(canvas, amount);
        m_light.draw(canvas, nowMs);
        drawStatus(canvas);
        drawAimLine(canvas, amount);
    }

}
