#include "client/viewmodels/minigames/BoxingWindow.h"

#include "core/Between.h"

namespace client {

    namespace {

        // positions and sizes are screen px
        constexpr int kLitLight = 20; // extra light added to the boxer when he is lit (hit)
        constexpr int kDashFrame = 10; // BoxingNumber frame 10: dash (no score yet)
        constexpr int kClosingFrame = 11; // BoxingNumber frame 11: closing mark after the digits
        constexpr int kRewardOffsetX = 106; // x offset of the reward figure from the form
        constexpr int kLostRewardX = 175; // x of the "BoxerLose" figure before the offset
        constexpr int kKnockOutRewardX = 210; // x of the "BoxerKo" figure before the offset

        /// @brief Loads BoxingWindow.xml and returns its form.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "BoxingForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("BoxingWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("BoxingForm");
        }

    }

    BoxingWindow::BoxingWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks,
        std::function<bool()> instructionsOpen
    )
        : m_services(services),
          m_screen(screen),
          m_instructionsOpen(std::move(instructionsOpen)),
          m_game(std::move(hooks)),
          m_form(loadForm(services, screen)),
          m_quit(screen.get<ui::ImageButton>("BoxingQuit")),
          m_backgroundSpot(screen.get<ui::Spot>("BoxingBackground")),
          m_barFrameSpot(screen.get<ui::Spot>("BoxingBarFrame")),
          m_barSpot(screen.get<ui::Spot>("BoxingBar")),
          m_monsterSpot(screen.get<ui::Spot>("BoxingMonster")),
          m_boxerSpot(screen.get<ui::Spot>("BoxingBoxer")),
          m_placeSpot(screen.get<ui::Spot>("BoxingPlace")),
          m_leftSpot(screen.get<ui::Spot>("BoxingLeft")),
          m_digitSpot(screen.get<ui::Spot>("BoxingDigit")) {
        screen.get<ui::DrawingArea>("BoxingPage").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.on("onBoxingDown", [this](const ui::EventArgs&) { m_game.pressed(m_instructionsOpen()); });
        screen.on("onBoxingQuit", [this](const ui::EventArgs&) { m_game.quitClicked(); });
    }

    void BoxingWindow::open(std::uint64_t nowMs) {
        m_game.open(nowMs);
        m_quit.setVisible(false);
        m_form.setVisible(true);
    }

    void BoxingWindow::close() { m_form.setVisible(false); }

    void BoxingWindow::drawNumber(gfx::Canvas& canvas, core::Point at, int value) const {
        const auto picture = m_services.pictures.find("BoxingNumber");
        if (!picture) {
            return;
        }
        const auto digit = [&](int slot, int frame) {
            // 10: left margin of the first digit
            const int x = at.x + 10 + m_digitSpot.pitch().x * (slot + 1) + m_digitSpot.size().width * slot;
            canvas.drawPicture(*picture, {x, at.y}, core::Rect{0, frame * m_digitSpot.size().height, m_digitSpot.size().width, m_digitSpot.size().height});
        };
        if (value == -1) { // -1: no score yet, shown as dashes
            digit(0, kDashFrame);
            digit(1, kDashFrame);
            digit(2, kDashFrame);
        } else {
            // decimal digits: hundreds, tens, units
            digit(0, value / 100);
            digit(1, value % 100 / 10);
            digit(2, value % 10);
        }
        digit(3, kClosingFrame);
    }

    void BoxingWindow::drawScoreBoard(gfx::Canvas& canvas) const {
        const BoxingGame::Scores& scores = m_game.scores();
        // top row y: 30 + 3 pixels below the place picture height; row gaps are 10 and 20 pixels
        const int top = m_placeSpot.size().height + 30 + 3;
        const int rows[3] = {top, m_placeSpot.size().height + top + 10, m_placeSpot.size().height * 2 + top + 20};
        const char* const places[3] = {"1st", "2nd", "3rd"};
        for (int row = 0; row < 3; ++row) {
            if (const auto place = m_services.pictures.find(places[row])) {
                canvas.drawPicture(*place, {m_placeSpot.at().x, rows[row]}, core::Rect{0, 0, m_placeSpot.size().width, m_placeSpot.size().height});
            }
            drawNumber(canvas, {m_placeSpot.size().width + m_placeSpot.at().x, rows[row]}, scores.punch[row]);
        }
        if (const auto left = m_services.pictures.find("left")) {
            canvas.drawPicture(
                *left,
                {m_placeSpot.size().width * 4 + m_placeSpot.at().x, top},
                core::Rect{0, 0, m_leftSpot.size().width, m_leftSpot.size().height}
            );
        }
        // the "left" number sits 5 place widths right of the board plus 15 pixels; the "left" picture 4 widths
        drawNumber(canvas, {m_placeSpot.size().width * 5 + m_placeSpot.at().x + 15, top}, scores.left);
    }

    void BoxingWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        m_game.update(nowMs, m_instructionsOpen());
        m_quit.setVisible(m_game.formShown());
        if (const core::Point mouse = m_screen.mouse(); mouse != m_lastMouse) {
            m_lastMouse = mouse;
            const core::Point local{mouse.x - BoxingGame::kPunchButton.x, mouse.y - BoxingGame::kPunchButton.y};
            const auto boxer = m_services.pictures.find("BoxingGameBoxer");
            m_game.moved(boxer && BoxingGame::kPunchButton.contains(mouse) && m_services.pictures.picture(*boxer).isOpaque(local.x, local.y));
        }
        gfx::PictureLibrary& pictures = m_services.pictures;
        const double amount = canvas.betweenBeats() ? m_game.sinceFrame(nowMs) : 1.0;
        const int powerWidth = core::between(m_game.previousPowerWidth(), m_game.powerWidth(), amount);
        if (const auto ring = pictures.find("BoxingBackground")) {
            canvas.drawPicture(*ring, m_backgroundSpot.at());
        }
        if (const auto frame = pictures.find("StrengthBarFrame")) {
            canvas.drawPicture(*frame, m_barFrameSpot.at(), core::Rect{0, 0, m_barFrameSpot.size().width, m_barFrameSpot.size().height});
        }
        if (const auto bar = pictures.find("BoxingBar"); bar && powerWidth > 0) {
            canvas.drawPicture(*bar, m_barSpot.at(), core::Rect{0, 0, powerWidth, m_barSpot.size().height});
        }
        const BoxingGame::Pose& pose = m_game.pose();
        if (const auto monster = pictures.find("BoxingGameMonst")) {
            canvas.drawPicture(
                *monster,
                {m_monsterSpot.at().x + pose.shake, m_monsterSpot.at().y - m_monsterSpot.size().height},
                core::Rect{0, pose.monsterFrame * m_monsterSpot.size().height, m_monsterSpot.size().width, m_monsterSpot.size().height}
            );
        }
        if (const auto boxer = pictures.find("BoxingGameBoxer")) {
            canvas.drawPicture(
                *boxer,
                m_boxerSpot.at(),
                core::Rect{0, pose.boxerFrame * m_boxerSpot.size().height, m_boxerSpot.size().width, m_boxerSpot.size().height}
            );
            if (pose.lit) {
                canvas.drawPictureLight(*boxer, m_boxerSpot.at(), kLitLight, core::Rect{0, 0, m_boxerSpot.size().width, m_boxerSpot.size().height});
            }
        }
        drawScoreBoard(canvas);
        if (m_game.rewardFigure() >= 0) {
            const bool lost = m_game.rewardFigure() == 0;
            if (const auto figure = pictures.find(lost ? "BoxerLose" : "BoxerKo")) {
                const int y = core::between(m_game.previousRewardY(), m_game.rewardY(), amount);
                canvas.drawPicture(*figure, {(lost ? kLostRewardX : kKnockOutRewardX) + kRewardOffsetX, y});
            }
        }
    }

}
