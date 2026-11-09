#include "client/viewmodels/minigames/LuckyWindow.h"

#include "client/rendering/characters/FigureShadow.h"
#include "client/rendering/hud/GameNumbers.h"
#include "core/Between.h"

#include <cstdlib>

namespace client {

    namespace {

        constexpr int kNumberStyle = 3; // game-number digit style for score and time
        constexpr int kCountdownStyle = 5; // game-number digit style for the start countdown
        constexpr std::uint64_t kBombLightFrameMs = 10; // bomb light frame duration, ms
        constexpr int kBombLightFrames = 12; // number of bomb light frames
        constexpr int kBombLightAlpha = 30; // light-blend strength of the bomb light
        constexpr int kSnapPixels = 40; // snap distance when catching an item, px

        /// @brief Loads LuckyWindow.xml into the screen and returns its form.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The "LuckyForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("LuckyWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("LuckyForm");
        }

    }

    LuckyWindow::LuckyWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks,
        std::function<bool()> instructionsOpen,
        std::function<const CharacterLook&()> playerLook
    )
        : m_services(services),
          m_screen(screen),
          m_hooks(std::move(hooks)),
          m_instructionsOpen(std::move(instructionsOpen)),
          m_playerLook(std::move(playerLook)),
          m_form(loadForm(services, screen)),
          m_quit(screen.get<ui::ImageButton>("LuckyQuit")),
          m_backgroundSpot(screen.get<ui::Spot>("LuckyBackground")),
          m_topSpot(screen.get<ui::Spot>("LuckyTop")),
          m_bottomSpot(screen.get<ui::Spot>("LuckyBottom")),
          m_lifeSpot(screen.get<ui::Spot>("LuckyLife")),
          m_scoreSpot(screen.get<ui::Spot>("LuckyScore")),
          m_scoreNumberSpot(screen.get<ui::Spot>("LuckyScoreNumber")),
          m_timeSpot(screen.get<ui::Spot>("LuckyTime")),
          m_timeNumberSpot(screen.get<ui::Spot>("LuckyTimeNumber")),
          m_messageSpot(screen.get<ui::Spot>("LuckyMessage")),
          m_heartSpot(screen.get<ui::Spot>("LuckyHeart")) {
        screen.get<ui::DrawingArea>("LuckyPage").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.on("onLuckyQuit", [this](const ui::EventArgs&) { m_game->quitClicked(); });
    }

    LuckyGame::Hooks LuckyWindow::makeHooks() {
        LuckyGame::Hooks hooks;
        static_cast<GameHooks&>(hooks) = m_hooks;
        hooks.pictureSize = [this](const std::string& name) {
            const auto picture = m_services.pictures.find(name);
            return picture ? core::Size{m_services.pictures.picture(*picture).width(), m_services.pictures.picture(*picture).height()} : core::Size{};
        };
        hooks.bombLight = [this](core::Point at) {
            LightEffect::Setup setup;
            setup.picture = m_services.pictures.find("S10152").value_or(gfx::PictureId{});
            setup.frameMs = kBombLightFrameMs;
            setup.firstFrame = 1;
            setup.lastFrame = kBombLightFrames;
            setup.frameCount = kBombLightFrames;
            setup.motion = LightEffect::Motion::Once;
            setup.loop = false;
            setup.alpha = kBombLightAlpha;
            m_light.start(setup, at, m_game->lastFrameMs());
        };
        hooks.playerFrames = [this](int slot) { return m_services.characters.figureFrameCount(m_playerLook(), slot); };
        hooks.throwerFrames = [this](int slot) { return m_services.characters.npcFrameCount(m_game->throwerSprite(), throwerSlot(slot)); };
        return hooks;
    }

    int LuckyWindow::throwerSlot(int slot) const {
        if (m_services.characters.npcFrameCount(m_game->throwerSprite(), slot) > 0) {
            return slot;
        }
        return m_services.characters.npcSlot(m_game->throwerSprite(), 0, slot);
    }

    void LuckyWindow::open(int npcId, std::uint64_t nowMs) {
        m_game = std::make_unique<LuckyGame>(makeHooks(), npcId, m_lastDropX);
        m_game->open(nowMs);
        m_light.stop();
        m_quit.setVisible(false);
        m_form.setVisible(true);
    }

    void LuckyWindow::close() {
        m_form.setVisible(false);
        m_light.stop();
    }

    void LuckyWindow::drawFigures(gfx::Canvas& canvas, double amount) {
        const CharacterLook& look = m_playerLook();
        const core::Point player = core::between(m_game->previousPlayerAt(), m_game->playerAt(), amount, kSnapPixels);
        drawFigureShadow(canvas, player, 0);
        m_services.characters.drawFigure(canvas, look, player, m_game->playerSlot(), m_game->playerFrame());
        formats::NpcInfo thrower;
        thrower.spriteId = m_game->throwerSprite();
        thrower.colors.fill(444444444); // default colour code of every thrower palette slot
        const core::Point at = core::between(m_game->previousThrowerAt(), m_game->throwerAt(), amount, kSnapPixels);
        m_services.characters.drawNpc(canvas, thrower, at, throwerSlot(m_game->throwerSlot()), m_game->throwerFrame());
    }

    void LuckyWindow::drawObjects(gfx::Canvas& canvas, std::uint64_t nowMs, double amount) {
        for (int i = 0; i < LuckyGame::kObjects; ++i) {
            const FallingObject& object = m_game->objects()[static_cast<std::size_t>(i)];
            if (!object.inUse) {
                continue;
            }
            if (const auto picture =
                    m_services.pictures.find(object.kind == 1 ? "2302" : "S10816L")) { // object kind 1 (coin) uses picture 2302, otherwise the bomb picture
                const int y = std::abs(object.y - object.previousY) > kSnapPixels ? object.y : core::between(object.previousY, object.y, amount);
                canvas.drawPicture(
                    *picture,
                    {object.x - object.halfWidth, y - object.anchorY},
                    core::Rect{0, m_game->shownFrame(i) * object.frameHeight, object.frameWidth, object.frameHeight}
                );
            }
        }
        m_light.draw(canvas, nowMs);
    }

    void LuckyWindow::drawHearts(gfx::Canvas& canvas) {
        gfx::PictureLibrary& pictures = m_services.pictures;
        if (const auto heart = pictures.find("icon_heart")) {
            for (int j = 1; j <= m_game->hearts(); ++j) {
                canvas.drawPictureScaled(
                    *heart,
                    {j * m_heartSpot.pitch().x + m_heartSpot.at().x, m_heartSpot.at().y, m_heartSpot.rect().width, m_heartSpot.rect().height}
                );
            }
        }
    }

    void LuckyWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (!m_game) {
            return;
        }
        m_game->update(nowMs, m_instructionsOpen(), m_screen.mouse().x);
        m_lastDropX = m_game->lastDropX();
        m_quit.setVisible(m_game->formShown());
        gfx::PictureLibrary& pictures = m_services.pictures;
        const double amount = canvas.betweenBeats() ? m_game->sinceFrame(nowMs) : 1.0;
        if (const auto background = pictures.find("59001")) {
            canvas.drawPicture(*background, m_backgroundSpot.at());
        }
        if (const auto top = pictures.find("59016")) {
            canvas.drawPicture(*top, m_topSpot.at());
        }
        if (const auto bottom = pictures.find("59083")) {
            canvas.drawPicture(*bottom, m_bottomSpot.at());
        }
        drawFigures(canvas, amount);
        drawHearts(canvas);
        if (m_game->phase() == LuckyGame::Phase::Playing) {
            drawObjects(canvas, nowMs, amount);
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
        if (m_game->phase() == LuckyGame::Phase::Countdown) {
            if (m_game->countdown() == 0) {
                drawGameLetters(canvas, m_messageSpot.at(), "Go", 0, 1.0F, m_messageSpot.pitch().x, 0, true);
            } else if (m_game->countdown() >= 1 && m_game->countdown() <= 3) { // countdown numbers 3, 2, 1 are shown as digits
                drawGameNumber(canvas, "Num_YellowRed10_2", m_messageSpot.at(), m_game->countdown(), kCountdownStyle, true);
            }
        }
        if (m_game->phase() == LuckyGame::Phase::Over) {
            drawGameLetters(canvas, m_messageSpot.at(), m_game->overText(), 0, 1.0F, m_messageSpot.pitch().x, 0, true);
        }
    }

}
