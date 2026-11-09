#include "client/viewmodels/minigames/ThrowWindow.h"

#include "client/rendering/characters/FigureShadow.h"
#include "core/Between.h"

#include <cstdlib>

namespace client {

    namespace {

        constexpr int kHeartFirstDx = -15; // x of the first heart from the thrower's feet, px
        constexpr int kHeartStepX = 15; // x step between hearts, px
        constexpr int kHeartDy = -90; // y of the heart row above the thrower's feet, px
        constexpr int kBarDx = 20; // x of the power bar right of the thrower, px
        constexpr int kBarFrameDy = -50; // y of the power bar frame from the feet, px
        constexpr int kBarFillDy = -20; // y of the bottom of the bar fill from the feet, px
        constexpr int kBarMostRows = 30; // full height of the Str_Bar fill area, px (rows)
        constexpr int kBarFillLeft = 6; // left edge of the fill part inside the Str_Bar picture, px
        constexpr int kBarFillRight = 12; // right edge of the fill part inside the Str_Bar picture, px
        constexpr int kBarWidth = 6; // width of the bar frame part of Str_Bar, px
        constexpr int kBarHeight = 30; // height of the bar frame part of Str_Bar, px
        constexpr std::size_t kWeaponSlot = 3; // index in equipmentLooks cleared so no weapon is drawn
        constexpr int kSnapPixels = 40; // moves longer than this many px jump instead of gliding

        /// @brief Gives a person's drawn position, part of the way from where it stood before the last frame.
        /// @param thrower The person.
        /// @param amount How far between the previous and current place (1 = current).
        /// @return Screen position to draw at.
        core::Point shownAt(const ThrowThrower& thrower, double amount) { return core::between(thrower.previousScreen, thrower.screen, amount, kSnapPixels); }

        /// @brief Loads ThrowWindow.xml into the screen and returns its form.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The "ThrowForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ThrowWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("ThrowForm");
        }

    }

    ThrowWindow::ThrowWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks,
        std::function<const CharacterLook&()> playerLook
    )
        : m_services(services),
          m_hooks(std::move(hooks)),
          m_playerLook(std::move(playerLook)),
          m_form(loadForm(services, screen)),
          m_exit(screen.get<ui::ImageButton>("ThrowExit")) {
        screen.get<ui::DrawingArea>("ThrowPage").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.on("onThrowDown", [this](const ui::EventArgs& args) { m_game->pressed(args.position, true); });
        screen.on("onThrowUp", [this](const ui::EventArgs& args) { m_game->released(args.position, true); });
        screen.on("onThrowRightUp", [this](const ui::EventArgs& args) { m_game->released(args.position, false); });
        screen.on("onThrowExit", [this](const ui::EventArgs&) { m_game->leave(); });
    }

    ThrowGame::Hooks ThrowWindow::makeHooks() {
        ThrowGame::Hooks hooks;
        static_cast<GameHooks&>(hooks) = m_hooks;
        hooks.npcSprite = [this](int npcId) {
            const auto info = m_services.npcs.find(npcId);
            return info ? info->spriteId : 0;
        };
        hooks.spriteFrames = [this](int spriteId, int slot) { return m_services.characters.npcFrameCount(spriteId, slot); };
        hooks.playerFrames = [this](int slot) { return m_services.characters.figureFrameCount(m_playerLook(), slot); };
        hooks.picture = [this](const std::string& name) { return m_services.pictures.find(name).value_or(gfx::PictureId{}); };
        return hooks;
    }

    void ThrowWindow::open(int npcId, std::uint64_t nowMs) {
        m_game = std::make_unique<ThrowGame>(makeHooks());
        m_game->open(npcId, nowMs);
        m_exit.setVisible(false);
        m_form.setVisible(true);
    }

    void ThrowWindow::begin() { m_game->begin(); }

    void ThrowWindow::close() { m_form.setVisible(false); }

    void ThrowWindow::drawHearts(gfx::Canvas& canvas, int side, int index, core::Point at) {
        const ThrowTeam& team = side == 1 ? m_game->own() : m_game->rival();
        if (const auto heart = m_services.pictures.find("Icon_Heart")) {
            for (int j = 1; j <= team.lives[static_cast<std::size_t>(index)]; ++j) {
                canvas.drawPicture(*heart, {at.x + kHeartFirstDx + (j - 1) * kHeartStepX, at.y + kHeartDy});
            }
        }
    }

    void ThrowWindow::drawThrower(gfx::Canvas& canvas, int side, int index, double amount) {
        const ThrowTeam& team = side == 1 ? m_game->own() : m_game->rival();
        const ThrowThrower& thrower = team.throwers[static_cast<std::size_t>(index)];
        const core::Point at = shownAt(thrower, amount);
        drawFigureShadow(canvas, at, 0);
        if (side == 1 && index == m_game->playerSlot()) {
            CharacterLook look = m_playerLook();
            look.equipmentLooks[kWeaponSlot] = 0;
            m_services.characters.drawFigure(canvas, look, at, thrower.direction, thrower.anim.frame);
        } else {
            formats::NpcInfo fighter;
            fighter.spriteId = m_game->fighterSprite();
            fighter.colors.fill(444444444); // default colour code of every fighter palette slot
            m_services.characters.drawNpc(canvas, fighter, at, thrower.direction, thrower.anim.frame);
        }
        drawHearts(canvas, side, index, at);
    }

    void ThrowWindow::drawPowerBar(gfx::Canvas& canvas, double amount) {
        if (!m_game->charging()) {
            return;
        }
        const auto bar = m_services.pictures.find("Str_Bar");
        if (!bar) {
            return;
        }
        const core::Point at = shownAt(m_game->own().throwers[static_cast<std::size_t>(m_game->playerSlot())], amount);
        const int rows = m_game->power() * 2; // 2 px of bar fill per power point
        if (rows > 0) {
            canvas.drawPicture(
                *bar,
                {at.x + kBarDx, at.y + kBarFillDy - rows},
                core::Rect{kBarFillLeft, kBarMostRows - rows, kBarFillRight - kBarFillLeft, rows}
            );
        }
        canvas.drawPicture(*bar, {at.x + kBarDx, at.y + kBarFrameDy}, core::Rect{0, 0, kBarWidth, kBarHeight});
    }

    void ThrowWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (!m_game) {
            return;
        }
        m_game->update(nowMs);
        m_exit.setVisible(m_game->begun());
        canvas.fillRect(canvas.window(), {0, 0, 0, 255}); // opaque black RGBA
        const double amount = canvas.betweenBeats() ? m_game->sinceFrame(nowMs) : 1.0;
        const auto ball = m_services.pictures.find("Icon_Element_3_1");
        for (int place = 1; place <= ThrowGame::kDrawItems; ++place) {
            const int item = m_game->drawItem(place);
            if (item >= 1 && item <= 4) { // draw items 1-4: the own team's four places
                drawThrower(canvas, 1, item, amount);
            } else if (item >= 5 && item <= 8) { // draw items 5-8: the rival team's four places
                drawThrower(canvas, 2, item - 4, amount);
            } else if (item >= 9) { // draw items 9 and up: projectiles
                const int index = item - 8; // projectile index (items start at 9)
                ThrowProjectile& projectile = m_game->projectile(index);
                if (projectile.active == 1 && ball) { // projectile state 1: ball in flight
                    const bool jumped =
                        std::abs(projectile.at.x - projectile.previousAt.x) > kSnapPixels || std::abs(projectile.at.y - projectile.previousAt.y) > kSnapPixels;
                    const double part = jumped ? 1.0 : amount;
                    const int arc = core::between(projectile.previousArcHeight, projectile.arcHeight, part);
                    const core::Point at{
                        core::between(projectile.previousAt.x, projectile.at.x, part),
                        core::between(projectile.previousAt.y, projectile.at.y, part)
                    };
                    canvas.drawPicture(*ball, {at.x, at.y - arc});
                }
                if (projectile.active == 2 || projectile.active == 3) { // projectile states 2 and 3: landing / hit effect playing
                    projectile.light.draw(canvas, nowMs);
                    m_game->finishLight(index);
                }
            }
        }
        drawPowerBar(canvas, amount);
    }

}
