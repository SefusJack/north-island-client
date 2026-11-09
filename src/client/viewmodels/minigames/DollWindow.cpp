#include "client/viewmodels/minigames/DollWindow.h"

#include "core/Between.h"
#include "ui/WrappedTooltip.h"

#include <algorithm>
#include <cstdlib>

namespace client {

    namespace {

        constexpr int kDollScale = 12; // dolls are drawn at 12/10 of their size
        constexpr int kScaleDivisor = 10; // divisor of the doll scale
        constexpr std::uint8_t kHoverLight = 140; // extra light on a hovered doll
        constexpr int kTooltipRise = 10; // tooltip sits 10 px above the doll's top
        constexpr int kTooltipLineHeight = 8; // tooltip text line height, px
        constexpr int kSnapPixels = 16; // a move of more than 16 px is a jump, drawn without smoothing

        /// @brief Loads DollWindow.xml and returns its form.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "DollForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("DollWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("DollForm");
        }

    }

    DollWindow::DollWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks,
        std::function<bool()> instructionsOpen
    )
        : m_services(services),
          m_screen(screen),
          m_hooks(std::move(hooks)),
          m_instructionsOpen(std::move(instructionsOpen)),
          m_form(loadForm(services, screen)),
          m_left(screen.get<ui::ImageButton>("DollLeft")),
          m_right(screen.get<ui::ImageButton>("DollRight")),
          m_grasp(screen.get<ui::ImageButton>("DollGrasp")),
          m_quit(screen.get<ui::ImageButton>("DollQuit")),
          m_groundSpot(screen.get<ui::Spot>("DollGround")),
          m_maskSpot(screen.get<ui::Spot>("DollMask")) {
        screen.get<ui::DrawingArea>("DollPage").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.on("onDollLeftDown", [this](const ui::EventArgs&) { m_game->buttonDown(ClawCatcher::State::Left); });
        screen.on("onDollRightDown", [this](const ui::EventArgs&) { m_game->buttonDown(ClawCatcher::State::Right); });
        screen.on("onDollGrasp", [this](const ui::EventArgs&) { m_game->buttonDown(ClawCatcher::State::Grasping); });
        screen.on("onDollUp", [this](const ui::EventArgs&) { m_game->buttonUp(); });
        screen.on("onDollQuit", [this](const ui::EventArgs&) { m_game->quitClicked(); });
    }

    int DollWindow::slotOf(const formats::NpcInfo& npc, int direction) const {
        if (m_services.characters.npcFrameCount(npc.spriteId, direction) > 0) {
            return direction;
        }
        return m_services.characters.npcSlot(npc.spriteId, npc.directionKind, direction);
    }

    DollGame::Hooks DollWindow::makeHooks() const {
        DollGame::Hooks hooks;
        static_cast<GameHooks&>(hooks) = m_hooks;
        hooks.npcInfo = [this](int npcId) { return m_services.npcs.find(npcId); };
        hooks.spriteSize = [this](const formats::NpcInfo& npc, int slot) { return m_services.characters.npcFrameSize(npc.spriteId, slotOf(npc, slot)); };
        hooks.frameCount = [this](const formats::NpcInfo& npc, int slot) { return m_services.characters.npcFrameCount(npc.spriteId, slotOf(npc, slot)); };
        return hooks;
    }

    void DollWindow::open(std::uint64_t nowMs) {
        m_game = std::make_unique<DollGame>(makeHooks());
        m_game->open(nowMs);
        m_left.setVisible(false);
        m_right.setVisible(false);
        m_grasp.setVisible(false);
        m_quit.setVisible(false);
        m_form.setVisible(true);
    }

    void DollWindow::close() {
        m_form.setVisible(false);
        m_game.reset();
    }

    int DollWindow::drawDolls(gfx::Canvas& canvas, double amount) {
        SpriteHighlight highlight;
        highlight.scale = kDollScale;
        highlight.scaleDivisor = kScaleDivisor;
        int hovered = -1;
        for (int i = 0; i < DollGame::kDolls; ++i) {
            const NpcDoll& doll = m_game->dolls()[static_cast<std::size_t>(i)];
            // dolls farther than 100 pixels outside the 900 x 700 area are skipped
            if (doll.screen.x < -100 || doll.screen.y < -100 || doll.screen.x > 900 || doll.screen.y > 700) {
                continue;
            }
            const int slot = slotOf(doll.npc, doll.direction);
            SpriteHighlight lit = highlight;
            if (m_services.characters.npcContains(doll.npc, doll.screen, slot, doll.anim.frame, m_screen.mouse(), highlight)) {
                lit.light = kHoverLight;
                if (hovered < 0) {
                    hovered = i;
                }
            }
            const ClawCatcher& claw = m_game->catcher();
            core::Point at = doll.screen;
            if (i == claw.doll() && claw.state() != ClawCatcher::State::Lifting) {
                at = core::between(doll.previousScreen, doll.screen, amount, kSnapPixels);
            }
            m_services.characters.drawNpc(canvas, doll.npc, at, slot, doll.anim.frame, lit);
        }
        return hovered;
    }

    void DollWindow::drawTooltip(gfx::Canvas& canvas, int hovered) {
        if (hovered < 0) {
            return;
        }
        const NpcDoll& doll = m_game->dolls()[static_cast<std::size_t>(hovered)];
        const core::Size size = m_game->spriteSizeOf(hovered);
        ui::drawWrappedTooltip(
            canvas,
            {doll.screen.x - size.width / 2, doll.screen.y - size.height - kTooltipRise},
            doll.npc.name,
            // colours (r, g, b, a): white text, blue-ish fill (alpha 200), blue border
            static_cast<int>(doll.npc.name.size()),
            kTooltipLineHeight,
            {255, 255, 255, 255},
            {61, 138, 249, 200},
            {0, 0, 255, 255}
        );
    }

    void DollWindow::drawClaw(gfx::Canvas& canvas, double amount) {
        const ClawCatcher& claw = m_game->catcher();
        const int before = m_game->previousClawX();
        const int x = std::abs(claw.x() - before) > kSnapPixels ? claw.x() : core::between(before, claw.x(), amount);
        for (const ClawCatcher::Blit& blit : claw.blits()) {
            const char* name = "CatcherBasic";
            int height = ClawCatcher::kBasicHeight;
            switch (blit.picture) {
                case ClawCatcher::Picture::Basic: // open claw, the default picture
                    break;
                case ClawCatcher::Picture::Moving: // claw moving sideways
                    name = "CatcherMoving";
                    height = ClawCatcher::kMovingHeight;
                    break;
                case ClawCatcher::Picture::Catching: // claw closing on a doll
                    name = "Catching";
                    height = ClawCatcher::kCatchingHeight;
                    break;
                case ClawCatcher::Picture::Dropping: // claw dropping the doll
                    name = "CatcherDroping";
                    height = ClawCatcher::kDroppingHeight;
                    break;
            }
            const auto picture = m_services.pictures.find(name);
            if (picture && (blit.frame + 1) * height <= m_services.pictures.picture(*picture).height()) {
                canvas.drawPicture(*picture, {x, claw.y()}, core::Rect{0, blit.frame * height, ClawCatcher::kClawWidth, height});
            }
        }
    }

    void DollWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (!m_game) {
            return;
        }
        m_game->update(nowMs, m_instructionsOpen());
        const bool shown = m_game->formShown();
        m_left.setVisible(shown);
        m_right.setVisible(shown);
        m_grasp.setVisible(shown);
        m_quit.setVisible(shown);
        gfx::PictureLibrary& pictures = m_services.pictures;
        canvas.fillRect(canvas.window(), {0, 0, 0, 255}); // opaque black (r, g, b, a)
        if (const auto ground = pictures.find("CD_BackGround")) {
            canvas.drawPicture(*ground, m_groundSpot.at());
        }
        const double amount = canvas.betweenBeats() ? m_game->sinceFrame(nowMs) : 1.0;
        const int hovered = drawDolls(canvas, amount);
        drawClaw(canvas, amount);
        if (const auto mask = pictures.find("BackGroundMask")) {
            canvas.drawPicture(*mask, m_maskSpot.at());
        }
        drawTooltip(canvas, hovered);
    }

}
