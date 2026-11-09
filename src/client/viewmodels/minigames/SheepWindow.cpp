#include "client/viewmodels/minigames/SheepWindow.h"

#include "client/rendering/characters/FigureShadow.h"
#include "client/rendering/hud/GameNumbers.h"

namespace client {

    namespace {

        constexpr int kSleeperScale = 20; // sleeper scale numerator: 20 / 10 = drawn at 2x
        constexpr int kScaleDivisor = 10; // sleeper scale denominator
        constexpr int kEffectFrames = 5; // frames stacked vertically in the hit flash picture
        constexpr int kEffectLight = 25; // light-blend strength of the hit flash
        constexpr int kHammerFrames = 3; // frames stacked vertically in the hammer picture
        constexpr int kDreamFrames = 2; // frames stacked vertically in the dream and blink pictures (normal, hit)
        constexpr int kSheepNpc = 17371; // NPC 17371: Sheep (sleeper of game kind 1)
        constexpr int kPigNpc = 17400; // NPC 17400: Pig (sleeper of the other game kinds)

        /// @brief Loads SheepWindow.xml into the screen and returns its form.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The "SheepForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("SheepWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("SheepForm");
        }

    }

    SheepWindow::SheepWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks
    )
        : m_services(services),
          m_hooks(std::move(hooks)),
          m_form(loadForm(services, screen)),
          m_page(screen.get<ui::DrawingArea>("SheepPage")),
          m_exit(screen.get<ui::ImageButton>("SheepExit")),
          m_skySpot(screen.get<ui::Spot>("SheepSky")),
          m_hillsSpot(screen.get<ui::Spot>("SheepHills")),
          m_cloudsSpot(screen.get<ui::Spot>("SheepClouds")),
          m_boardSpot(screen.get<ui::Spot>("SheepBoard")),
          m_scoreSpot(screen.get<ui::Spot>("SheepScore")),
          m_lifeSpot(screen.get<ui::Spot>("SheepLife")),
          m_timeSpot(screen.get<ui::Spot>("SheepTime")),
          m_scoreDigitsSpot(screen.get<ui::Spot>("SheepScoreDigits")),
          m_timeDigitsSpot(screen.get<ui::Spot>("SheepTimeDigits")),
          m_blinkSheepSpot(screen.get<ui::Spot>("SheepBlinkSheep")),
          m_blinkPigSpot(screen.get<ui::Spot>("SheepBlinkPig")),
          m_sleeperSpot(screen.get<ui::Spot>("SheepSleeper")),
          m_dreamSpot(screen.get<ui::Spot>("SheepDream")),
          m_hammerSpot(screen.get<ui::Spot>("SheepHammer")),
          m_effectSpot(screen.get<ui::Spot>("SheepEffect")),
          m_messageSpot(screen.get<ui::Spot>("SheepMessage")),
          m_heartSpot(screen.get<ui::Spot>("SheepHeart")) {
        m_page.setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.on("onSheepDown", [this](const ui::EventArgs& args) { m_game->clicked(args.position); });
        screen.on("onSheepExit", [this](const ui::EventArgs&) { m_game->leave(); });
    }

    formats::NpcInfo SheepWindow::sleeper() const { return m_services.npcs.find(m_game->gameKind() == 1 ? kSheepNpc : kPigNpc).value_or(formats::NpcInfo{}); }

    SheepGame::Hooks SheepWindow::makeHooks() {
        SheepGame::Hooks hooks;
        static_cast<GameHooks&>(hooks) = m_hooks;
        hooks.sleeperFrames = [this](int slot) { return m_services.characters.npcFrameCount(sleeper().spriteId, slot); };
        return hooks;
    }

    void SheepWindow::open(int gameKind, std::uint64_t nowMs) {
        m_game = std::make_unique<SheepGame>(makeHooks(), gameKind);
        m_game->open(nowMs);
        m_exit.setVisible(false);
        m_page.setCursor(ui::CursorKind::Normal);
        m_form.setVisible(true);
    }

    void SheepWindow::begin(std::uint64_t nowMs) {
        m_page.setCursor(ui::CursorKind::Mallet);
        m_game->begin(nowMs);
    }

    void SheepWindow::close() {
        m_form.setVisible(false);
        m_page.setCursor(ui::CursorKind::Normal);
    }

    void SheepWindow::drawHud(gfx::Canvas& canvas) {
        gfx::PictureLibrary& pictures = m_services.pictures;
        if (const auto sky = pictures.find("59102")) {
            canvas.drawPicture(*sky, m_skySpot.at());
        }
        if (const auto hills = pictures.find("59087")) {
            canvas.drawPicture(*hills, m_hillsSpot.at());
        }
        if (const auto clouds = pictures.find("59090")) {
            canvas.drawPicture(*clouds, m_cloudsSpot.at());
        }
        if (const auto board = pictures.find("SheepGameBG_2")) {
            canvas.drawPicture(*board, m_boardSpot.at());
        }
        if (const auto heart = pictures.find(m_game->gameKind() == 1 ? "sheepDreamLife" : "sheepDreamLife_" + std::to_string(m_game->gameKind()))) {
            for (int i = 1; i <= m_game->lives(); ++i) {
                canvas.drawPictureScaled(*heart, {i * m_heartSpot.pitch().x, m_heartSpot.rect().y, m_heartSpot.rect().width, m_heartSpot.rect().height});
            }
        }
        if (const auto score = pictures.find("MiniGame_Score_1")) {
            canvas.drawPicture(*score, m_scoreSpot.at());
        }
        if (const auto life = pictures.find("MiniGame_Life_1")) {
            canvas.drawPicture(*life, m_lifeSpot.at());
        }
        if (const auto time = pictures.find("MiniGame_Time_1")) {
            canvas.drawPicture(*time, m_timeSpot.at());
        }
        drawNativeNumber(canvas, "Num_White5_1", m_scoreDigitsSpot.at(), m_game->score());
        drawNativeNumber(canvas, "Num_White5_1", m_timeDigitsSpot.at(), m_game->timeLeft());
        if (m_game->gameOver()) {
            drawGameLetters(canvas, m_messageSpot.at(), m_game->lives() <= 0 ? "You Lose" : "You Win", 0, 1.0F, m_messageSpot.pitch().x, 0, true);
        }
    }

    void SheepWindow::drawScene(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (!m_game->started()) {
            return;
        }
        gfx::PictureLibrary& pictures = m_services.pictures;
        const bool beaten = m_game->gameOver() && m_game->lives() <= 0;
        if (!beaten) {
            if (const auto blink = pictures.find("Sleepbabo")) {
                const gfx::Picture& data = pictures.picture(*blink);
                const int height = data.height() / kDreamFrames;
                canvas.drawPicture(
                    *blink,
                    m_game->gameKind() == 1 ? m_blinkSheepSpot.at() : m_blinkPigSpot.at(),
                    core::Rect{0, m_game->blinkFrame() * height, data.width(), height}
                );
            }
        }
        const formats::NpcInfo npc = sleeper();
        drawFigureShadow(canvas, m_sleeperSpot.at(), npc.shadow);
        SpriteHighlight highlight;
        highlight.scale = kSleeperScale;
        highlight.scaleDivisor = kScaleDivisor;
        m_services.characters.drawNpc(canvas, npc, m_sleeperSpot.at(), m_game->sleeperSlot(), m_game->sleeperFrame(), highlight);
        static constexpr int kDreamBase[4] = {2, 3, 4, 5};
        const int kind = m_game->dreamKind();
        const int slot =
            kind == 1 ? 3 : (m_game->gameKind() - 1) * 4 + kDreamBase[kind]; // dream picture number: 3 for the good dream, else 4 pictures per game kind
        const int rise = m_game->dreamRise(canvas.betweenBeats() ? m_game->sinceFrame(nowMs) : 1.0);
        if (const auto dream = pictures.find("SheepDream_" + std::to_string(slot))) {
            const gfx::Picture& data = pictures.picture(*dream);
            const int height = data.height() / kDreamFrames;
            canvas.drawPicture(
                *dream,
                {m_dreamSpot.at().x, m_dreamSpot.at().y - rise},
                core::Rect{0, (m_game->dreamHit() ? 1 : 0) * height, data.width(), height}
            );
        }
        if (const auto hammer = pictures.find("Hammer_1")) {
            const gfx::Picture& data = pictures.picture(*hammer);
            const int height = data.height() / kHammerFrames;
            canvas.drawPicture(*hammer, m_hammerSpot.at(), core::Rect{0, m_game->hammerFrame() * height, data.width(), height});
        }
        if (m_game->gameOver()) {
            drawGameLetters(canvas, m_messageSpot.at(), m_game->lives() <= 0 ? "You Lose" : "You Win", 0, 1.0F, m_messageSpot.pitch().x, 0, true);
        }
    }

    void SheepWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (!m_game) {
            return;
        }
        m_game->update(nowMs);
        m_exit.setVisible(m_game->started());
        drawHud(canvas);
        drawScene(canvas, nowMs);
        const int effect = m_game->effectFrame();
        if (effect >= 0) {
            if (const auto flash = m_services.pictures.find("S10151")) {
                const gfx::Picture& data = m_services.pictures.picture(*flash);
                const int height = data.height() / kEffectFrames;
                canvas.drawPictureLight(*flash, m_effectSpot.at(), kEffectLight, core::Rect{0, effect * height, data.width(), height});
            }
        }
    }

}
