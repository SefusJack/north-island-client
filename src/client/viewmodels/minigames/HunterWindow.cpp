#include "client/viewmodels/minigames/HunterWindow.h"

#include "client/rendering/hud/GameNumbers.h"
#include "core/Between.h"

namespace client {

    namespace {

        constexpr int kBarMarginPixels = 80; // px cut off the Color_R picture width to get the full life bar width
        constexpr int kBarHeight = 5; // height of an animal life bar, px
        constexpr int kBarRise = 30; // px the life bar sits above an animal drawn top
        constexpr int kSightOffset = 14; // px from the aim point to the top-left of the sight picture (half of it)
        constexpr int kShotFrameHeight = 100; // height and width of one frame of the S10151 shot flash, px
        constexpr int kShotLight = 25; // light strength of the shot flash
        constexpr std::uint8_t kHoverLight = 140; // light strength added to the animal under the mouse
        constexpr int kSnapPixels = 40; // an animal that moved more than this many px between frames is snapped, not interpolated

        /// @brief Loads HunterWindow.xml and returns its form.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "HunterForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("HunterWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("HunterForm");
        }

    }

    HunterWindow::HunterWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks,
        std::function<int()> playerBodyType
    )
        : m_services(services),
          m_screen(screen),
          m_hooks(std::move(hooks)),
          m_playerBodyType(std::move(playerBodyType)),
          m_form(loadForm(services, screen)),
          m_exit(screen.get<ui::ImageButton>("HunterExit")),
          m_forestSpot(screen.get<ui::Spot>("HunterForest")),
          m_frontSpot(screen.get<ui::Spot>("HunterFront")),
          m_scoreSpot(screen.get<ui::Spot>("HunterScore")),
          m_timeSpot(screen.get<ui::Spot>("HunterTime")),
          m_lifeSpot(screen.get<ui::Spot>("HunterLife")),
          m_scoreDigitsSpot(screen.get<ui::Spot>("HunterScoreDigits")),
          m_timeDigitsSpot(screen.get<ui::Spot>("HunterTimeDigits")),
          m_attackSpot(screen.get<ui::Spot>("HunterAttack")),
          m_messageSpot(screen.get<ui::Spot>("HunterMessage")),
          m_heartSpot(screen.get<ui::Spot>("HunterHeart")) {
        screen.get<ui::DrawingArea>("HunterPage").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.on("onHunterDown", [this](const ui::EventArgs& args) { m_game->shoot(args.position); });
        screen.on("onHunterExit", [this](const ui::EventArgs&) { m_game->leave(); });
    }

    HunterGame::Hooks HunterWindow::makeHooks() {
        HunterGame::Hooks hooks;
        static_cast<GameHooks&>(hooks) = m_hooks;
        hooks.npcInfo = [this](int npcId) { return m_services.npcs.find(npcId); };
        hooks.spriteSize = [this](const formats::NpcInfo& npc, int slot) { return m_services.characters.npcFrameSize(npc.spriteId, slot); };
        hooks.frameCount = [this](const formats::NpcInfo& npc, int slot) { return m_services.characters.npcFrameCount(npc.spriteId, slot); };
        hooks.hit = [this](const formats::NpcInfo& npc, core::Point feet, int slot, int frame, int scale, core::Point point) {
            SpriteHighlight highlight;
            highlight.scale = scale;
            return m_services.characters.npcContains(npc, feet, slot, frame, point, highlight);
        };
        hooks.playerBodyType = m_playerBodyType;
        return hooks;
    }

    void HunterWindow::open(std::uint64_t nowMs) {
        m_game = std::make_unique<HunterGame>(makeHooks());
        m_game->open(nowMs);
        m_lastMouse = m_screen.mouse();
        m_exit.setVisible(false);
        m_form.setVisible(true);
    }

    void HunterWindow::close() { m_form.setVisible(false); }

    void HunterWindow::drawField(gfx::Canvas& canvas) {
        const core::Point shake = m_game->shake();
        gfx::PictureLibrary& pictures = m_services.pictures;
        // forest layers are pictures 59092 (forest), 59091 (trees), 59093 (front)
        if (const auto forest = pictures.find("59092")) {
            canvas.drawPicture(*forest, {m_forestSpot.at().x + shake.x, m_forestSpot.at().y + shake.y});
        }
        if (const auto trees = pictures.find("59091")) {
            canvas.drawPicture(*trees, {m_forestSpot.at().x + shake.x, m_forestSpot.at().y + shake.y});
        }
        if (const auto front = pictures.find("59093")) {
            canvas.drawPicture(*front, {m_frontSpot.at().x + shake.x, m_frontSpot.at().y + shake.y});
        }
    }

    void HunterWindow::drawHud(gfx::Canvas& canvas) {
        gfx::PictureLibrary& pictures = m_services.pictures;
        drawField(canvas);
        if (const auto heart = pictures.find("icon_heart")) {
            for (int i = 1; i <= m_game->lives(); ++i) {
                canvas.drawPictureScaled(
                    *heart,
                    {i * m_heartSpot.pitch().x + m_heartSpot.at().x, m_heartSpot.at().y, m_heartSpot.rect().width, m_heartSpot.rect().height}
                );
            }
        }
        if (const auto score = pictures.find("MiniGame_Score_1")) {
            canvas.drawPicture(*score, m_scoreSpot.at());
        }
        if (const auto time = pictures.find("MiniGame_Time_1")) {
            canvas.drawPicture(*time, m_timeSpot.at());
        }
        if (const auto life = pictures.find("MiniGame_Life_1")) {
            canvas.drawPicture(*life, m_lifeSpot.at());
        }
        drawNativeNumber(canvas, "Num_White5_1", m_scoreDigitsSpot.at(), m_game->score());
        drawNativeNumber(canvas, "Num_White5_1", m_timeDigitsSpot.at(), m_game->timeLeft());
    }

    void HunterWindow::drawAnimals(gfx::Canvas& canvas, double amount) {
        gfx::PictureLibrary& pictures = m_services.pictures;
        const auto bar = pictures.find("Color_R");
        for (int i = 1; i <= m_game->roleCount(); ++i) {
            const std::optional<HunterRole>& slot = m_game->roles()[static_cast<std::size_t>(i)];
            if (!slot) {
                continue;
            }
            const HunterRole& role = *slot;
            const core::Point moved = core::between(role.previousScreen, role.screen, amount, kSnapPixels);
            const core::Point at{role.drawnAt.x + moved.x - role.screen.x, role.drawnAt.y + moved.y - role.screen.y};
            if (bar && role.maxLife > 0) {
                const int full = pictures.picture(*bar).width() - kBarMarginPixels;
                const int width = role.life * full / role.maxLife;
                if (width > 0) {
                    canvas.drawPicture(*bar, {at.x - width / 2, at.y - role.drawnHeight - kBarRise}, core::Rect{0, 0, width, kBarHeight});
                }
            }
            SpriteHighlight highlight;
            highlight.scale = role.drawnScale;
            highlight.light = role.mouseOver ? kHoverLight : 0;
            m_services.characters.drawNpc(canvas, role.npc, at, role.drawnSlot, role.drawnFrame, highlight);
            if (role.attackShown) {
                if (const auto attack = pictures.find("animAtt")) {
                    canvas.drawPicture(*attack, m_attackSpot.at());
                }
            }
        }
    }

    void HunterWindow::drawCrosshair(gfx::Canvas& canvas) {
        gfx::PictureLibrary& pictures = m_services.pictures;
        if (const auto sight = pictures.find("sight")) {
            canvas.drawPicture(*sight, {m_game->aim().x - kSightOffset, m_game->aim().y - kSightOffset});
        }
        const int frame = m_game->shotFrame();
        if (frame > 0) {
            // S10151 is the shot flash strip; the flash is centred on x and sits a tenth of its height above y
            if (const auto flash = pictures.find("S10151")) {
                const gfx::Picture& data = pictures.picture(*flash);
                const core::Point shake = m_game->shake();
                canvas.drawPictureLight(
                    *flash,
                    {m_game->shotAt().x - data.width() / 2 + shake.x, m_game->shotAt().y - data.height() / 10 + shake.y},
                    kShotLight,
                    core::Rect{0, (frame - 1) * kShotFrameHeight, kShotFrameHeight, kShotFrameHeight}
                );
            }
        }
    }

    void HunterWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (!m_game) {
            return;
        }
        if (const core::Point mouse = m_screen.mouse(); mouse != m_lastMouse) {
            m_lastMouse = mouse;
            m_game->moveAim(mouse);
        }
        m_game->update(nowMs, m_screen.mouse());
        m_exit.setVisible(m_game->started());
        drawHud(canvas);
        drawAnimals(canvas, canvas.betweenBeats() ? m_game->sinceFrame(nowMs) : 1.0);
        drawCrosshair(canvas);
        if (m_game->timeLeft() <= 0) {
            drawGameLetters(canvas, m_messageSpot.at(), "You Win", 0, 1.0F, m_messageSpot.pitch().x, 0, true); // 1.0F is the letter scale
        }
        if (m_game->lives() <= 0) {
            drawGameLetters(canvas, m_messageSpot.at(), "You Lose", 0, 1.0F, m_messageSpot.pitch().x, 0, true); // 1.0F is the letter scale
        }
    }

}
