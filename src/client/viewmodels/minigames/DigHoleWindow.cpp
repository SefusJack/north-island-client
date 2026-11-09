#include "client/viewmodels/minigames/DigHoleWindow.h"

namespace client {

    namespace {

        /// @brief Loads DigHoleWindow.xml and returns its form.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "DigHoleForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("DigHoleWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("DigHoleForm");
        }

    }

    DigHoleWindow::DigHoleWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks,
        std::function<bool()> instructionsOpen,
        std::function<bool()> requestBoxOpen
    )
        : m_services(services),
          m_screen(screen),
          m_hooks(std::move(hooks)),
          m_instructionsOpen(std::move(instructionsOpen)),
          m_requestBoxOpen(std::move(requestBoxOpen)),
          m_form(loadForm(services, screen)),
          m_page(screen.get<ui::DrawingArea>("DigHolePage")),
          m_quit(screen.get<ui::ImageButton>("DigHoleQuit")),
          m_dig(screen.get<ui::ImageButton>("DigHoleDig")),
          m_boardSpot(screen.get<ui::Spot>("DigHoleBoard")),
          m_holeSpot(screen.get<ui::Spot>("DigHoleHole")),
          m_prizeSpot(screen.get<ui::Spot>("DigHolePrize")),
          m_fingerSpot(screen.get<ui::Spot>("DigHoleFinger")),
          m_fingerUpSpot(screen.get<ui::Spot>("DigHoleFingerUp")),
          m_fingerDownSpot(screen.get<ui::Spot>("DigHoleFingerDown")) {
        m_page.setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.on("onDigHoleDown", [this](const ui::EventArgs& args) { m_game->mouseDown(args.position); });
        screen.on("onDigHoleUp", [this](const ui::EventArgs&) { m_game->mouseUp(); });
        screen.on("onDigHoleDig", [this](const ui::EventArgs&) { m_game->digClicked(); });
        screen.on("onDigHoleQuit", [this](const ui::EventArgs&) { m_game->quitClicked(); });
    }

    void DigHoleWindow::open(std::uint64_t nowMs) {
        DigHoleGame::Hooks hooks;
        static_cast<GameHooks&>(hooks) = m_hooks;
        hooks.requestBoxOpen = m_requestBoxOpen;
        hooks.instructionsOpen = m_instructionsOpen;
        m_game = std::make_unique<DigHoleGame>(std::move(hooks));
        m_game->open(nowMs);
        m_lastMouse = m_screen.mouse();
        m_quit.setVisible(false);
        m_dig.setVisible(false);
        m_form.setVisible(true);
    }

    void DigHoleWindow::close() { m_form.setVisible(false); }

    void DigHoleWindow::leave() { m_game->exitGame(); }

    void DigHoleWindow::begin(std::uint64_t) { m_game->begin(); }

    void DigHoleWindow::receive(const core::Bytes& payload) {
        if (m_game) {
            m_game->receive(payload);
        }
    }

    void DigHoleWindow::pointsArrived(std::int32_t points) {
        if (m_game) {
            m_game->pointsArrived(points);
        }
    }

    void DigHoleWindow::drawBoard(gfx::Canvas& canvas, double amount) {
        gfx::PictureLibrary& pictures = m_services.pictures;
        const int offset = m_game->boardOffset(amount);
        const int rise = m_game->revealHeight(amount);
        if (const auto board = pictures.find("DigHole")) {
            canvas.drawPicture(*board, {offset + m_boardSpot.at().x, m_boardSpot.at().y});
        }
        if (const auto icon = pictures.find("icon_dig_1")) {
            const core::Rect dig = m_dig.bounds();
            canvas.drawPicture(*icon, {offset + dig.x, dig.y}, core::Rect{0, 0, dig.width, dig.height});
        }
        if (m_instructionsOpen()) {
            return;
        }
        const auto hole = pictures.find("DigHole_Hole");
        for (int row = 1; row <= DigHoleGame::kRows; ++row) {
            for (int column = 1; column <= DigHoleGame::kColumns; ++column) {
                if (hole && m_game->dug(row, column) && !m_game->boardSwapping()) {
                    canvas.drawPicture(
                        *hole,
                        {(column - 1) * m_holeSpot.pitch().x + m_holeSpot.at().x + offset, (row - 1) * m_holeSpot.pitch().y + m_holeSpot.at().y}
                    );
                }
                if (row == m_game->risingRow() && column == m_game->risingColumn()) {
                    const auto record = m_services.items.find(m_game->itemId());
                    if (const auto icon = record ? pictures.find(std::to_string(record->iconId)) : std::nullopt) {
                        canvas.drawPicture(
                            *icon,
                            {(column - 1) * m_holeSpot.pitch().x + m_prizeSpot.at().x, (row - 1) * m_holeSpot.pitch().y + m_prizeSpot.at().y - rise}
                        );
                    }
                }
            }
        }
    }

    void DigHoleWindow::drawFinger(gfx::Canvas& canvas) {
        if (m_instructionsOpen()) {
            return;
        }
        if (const auto finger = m_services.pictures.find("DigHole_Finger")) {
            const core::Point at{m_game->mouse().x + m_fingerSpot.at().x, m_game->mouse().y + m_fingerSpot.at().y};
            if (m_game->fingerShown()) {
                canvas.drawPicture(*finger, at, m_fingerUpSpot.rect());
            }
            if (m_game->fingerPressed()) {
                canvas.drawPicture(*finger, at, m_fingerDownSpot.rect());
            }
        }
    }

    void DigHoleWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (!m_game) {
            return;
        }
        if (const core::Point mouse = m_screen.mouse(); mouse != m_lastMouse) {
            m_lastMouse = mouse;
            m_game->mouseMove(mouse);
        }
        m_game->update(nowMs);
        m_quit.setVisible(m_game->formShown());
        m_dig.setVisible(m_game->formShown() && m_game->digButtonShown());
        m_page.setCursor(m_game->fingerCursor() ? ui::CursorKind::Finger : ui::CursorKind::Normal);
        drawBoard(canvas, canvas.betweenBeats() ? m_game->sinceFrame(nowMs) : 1.0);
        drawFinger(canvas);
    }

}
