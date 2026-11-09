#include "client/viewmodels/minigames/ScotdWindow.h"

#include "client/rendering/characters/FigureShadow.h"
#include "client/rendering/hud/GameNumbers.h"
#include "core/Between.h"
#include "ui/DrawingArea.h"

namespace client {

    namespace {

        constexpr int kEffectSize = 100; // catch flash (S10151) frame size, px (square)
        constexpr int kEffectLight = 25; // light-blend strength of the catch flash
        constexpr int kHearts = 5; // hearts at level 0; the game starts with (5 - level) hearts
        constexpr int kSnapPixels = 40; // moves longer than this many px are drawn without interpolation

        /// @brief Loads ScotdWindow.xml into the screen and returns its form.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The "ScotdForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ScotdWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("ScotdForm");
        }

    }

    ScotdWindow::ScotdWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks,
        std::function<const CharacterLook&()> playerLook
    )
        : m_services(services),
          m_hooks(std::move(hooks)),
          m_playerLook(std::move(playerLook)),
          m_form(loadForm(services, screen)),
          m_exit(screen.get<ui::ImageButton>("ScotdExit")),
          m_fieldSpot(screen.get<ui::Spot>("ScotdField")),
          m_frontSpot(screen.get<ui::Spot>("ScotdFront")),
          m_heartSpot(screen.get<ui::Spot>("ScotdHeart")),
          m_messageSpot(screen.get<ui::Spot>("ScotdMessage")) {
        screen.get<ui::DrawingArea>("ScotdPage").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.on("onScotdDown", [this](const ui::EventArgs& args) {
            if (m_game) {
                m_game->mouseDown(0, args.position);
            }
        });
        screen.on("onScotdRightDown", [this](const ui::EventArgs& args) {
            if (m_game) {
                m_game->mouseDown(1, args.position);
            }
        });
        screen.on("onScotdExit", [this](const ui::EventArgs&) { leave(); });
    }

    ScotdGame::Hooks ScotdWindow::makeHooks() {
        ScotdGame::Hooks hooks;
        static_cast<GameHooks&>(hooks) = m_hooks;
        hooks.bodySize = [this](bool player, int slot) {
            if (player) {
                return m_services.characters.figureFrameSize(m_playerLook(), slot);
            }
            return m_npc ? m_services.characters.npcFrameSize(m_npc->spriteId, npcSlot(slot)) : core::Size{};
        };
        hooks.frameCount = [this](bool player, int slot) {
            if (player) {
                return m_services.characters.figureFrameCount(m_playerLook(), slot);
            }
            return m_npc ? m_services.characters.npcFrameCount(m_npc->spriteId, npcSlot(slot)) : 0;
        };
        hooks.playerBodyType = [this] { return m_playerLook().bodyType; };
        hooks.random = [this](int count) { return std::uniform_int_distribution<int>(0, count - 1)(m_random); };
        return hooks;
    }

    int ScotdWindow::npcSlot(int slot) { return m_services.characters.npcSlot(m_npc->spriteId, m_npc->directionKind, slot); }

    void ScotdWindow::startedWith(int npcId, int level) {
        m_npcId = npcId;
        m_level = level;
    }

    void ScotdWindow::open(std::uint64_t nowMs) {
        m_npc = m_services.npcs.find(m_npcId);
        m_game = std::make_unique<ScotdGame>(makeHooks());
        m_game->open(nowMs, kHearts - m_level);
        m_exit.setVisible(false);
        m_form.setVisible(true);
    }

    void ScotdWindow::close() { m_form.setVisible(false); }

    void ScotdWindow::leave() {
        if (m_game) {
            m_game->leave();
        }
    }

    void ScotdWindow::begin(std::uint64_t) { m_game->begin(); }

    void ScotdWindow::keyDown(ScotdGame::Key key) {
        if (m_game && m_form.isVisible()) {
            m_game->keyDown(key);
        }
    }

    void ScotdWindow::drawField(gfx::Canvas& canvas) {
        gfx::PictureLibrary& pictures = m_services.pictures;
        if (const auto field = pictures.find("59082")) {
            canvas.drawPicture(*field, m_fieldSpot.at());
        }
        if (const auto front = pictures.find("59087")) {
            canvas.drawPicture(*front, m_frontSpot.at());
        }
        if (const auto top = pictures.find("59090")) {
            canvas.drawPicture(*top, m_fieldSpot.at());
        }
        if (const auto heart = pictures.find("icon_heart")) {
            for (int j = 1; j <= m_game->remaining(); ++j) {
                canvas.drawPictureScaled(
                    *heart,
                    {j * m_heartSpot.pitch().x + m_heartSpot.at().x, m_heartSpot.at().y, m_heartSpot.rect().width, m_heartSpot.rect().height}
                );
            }
        }
    }

    void ScotdWindow::drawShadow(gfx::Canvas& canvas, const ScotdRole& role, double amount) {
        drawFigureShadow(canvas, core::between(role.previousShadowAt, role.shadowAt, amount, kSnapPixels), 0);
    }

    void ScotdWindow::drawFigure(gfx::Canvas& canvas, const ScotdRole& role, bool player, double amount) {
        const core::Point at = core::between(role.previousDrawnAt, role.drawnAt, amount, kSnapPixels);
        if (player) {
            m_services.characters.drawFigure(canvas, m_playerLook(), at, role.drawnSlot, role.drawnFrame);
        } else if (m_npc) {
            m_services.characters.drawNpc(canvas, *m_npc, at, npcSlot(role.drawnSlot), role.drawnFrame);
        }
    }

    void ScotdWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (!m_game) {
            return;
        }
        m_game->update(nowMs);
        if (!m_form.isVisible()) {
            return;
        }
        m_exit.setVisible(m_game->started());
        drawField(canvas);
        const double amount = canvas.betweenBeats() ? m_game->sinceFrame(nowMs) : 1.0;
        drawFigure(canvas, m_game->player(), true, amount);
        drawShadow(canvas, m_game->player(), amount);
        for (int j = 1; j <= m_game->roleCount(); ++j) {
            if (const std::optional<ScotdRole>& role = m_game->roles()[static_cast<std::size_t>(j)]) {
                drawShadow(canvas, *role, amount);
                drawFigure(canvas, *role, false, amount);
            }
        }
        if (m_game->effectShown()) {
            if (const auto flash = m_services.pictures.find("S10151")) {
                canvas.drawPictureLight(
                    *flash,
                    m_game->effectAt(),
                    kEffectLight,
                    core::Rect{0, (m_game->effectFrame() - 1) * kEffectSize, kEffectSize, kEffectSize}
                );
            }
        }
        if (m_game->catchCount() >= ScotdGame::kCatchesToWin) {
            drawGameLetters(canvas, m_messageSpot.at(), "You Win", 0, 1.0F, m_messageSpot.pitch().x, 0, true);
        }
        if (m_game->remaining() <= 0) {
            drawGameLetters(canvas, m_messageSpot.at(), "You Lose", 0, 1.0F, m_messageSpot.pitch().x, 0, true);
        }
    }

}
