#include "client/viewmodels/minigames/MemoryGameWindow.h"

#include "client/rendering/hud/GameNumbers.h"
#include "ui/DrawingArea.h"

namespace client {

    namespace {

        constexpr int kNumberStyle = 3; // game-number digit style for score and time
        constexpr int kCountdownStyle = 5; // game-number digit style for the start countdown
        constexpr int kCardNumberStyle = 10; // game-number digit style for the card numbers

        /// @brief Loads MemoryGameWindow.xml into the screen and returns its form.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The "MemoryGameForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MemoryGameWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("MemoryGameForm");
        }

    }

    MemoryGameWindow::MemoryGameWindow(
        GameServices& services,
        ui::Screen& screen,
        GameHooks hooks,
        std::function<bool()> instructionsOpen
    )
        : m_services(services),
          m_instructionsOpen(std::move(instructionsOpen)),
          m_game(std::move(hooks)),
          m_form(loadForm(services, screen)),
          m_leave(screen.get<ui::ImageButton>("MemoryGameLeave")),
          m_boardSpot(screen.get<ui::Spot>("MemoryGameBoard")),
          m_heartSpot(screen.get<ui::Spot>("MemoryGameHeart")),
          m_scoreSpot(screen.get<ui::Spot>("MemoryGameScore")),
          m_scoreNumberSpot(screen.get<ui::Spot>("MemoryGameScoreNumber")),
          m_timeSpot(screen.get<ui::Spot>("MemoryGameTime")),
          m_timeNumberSpot(screen.get<ui::Spot>("MemoryGameTimeNumber")),
          m_messageSpot(screen.get<ui::Spot>("MemoryGameMessage")),
          m_rightSignSpot(screen.get<ui::Spot>("MemoryGameRightSign")),
          m_wrongSignSpot(screen.get<ui::Spot>("MemoryGameWrongSign")),
          m_cardNumberSpot(screen.get<ui::Spot>("MemoryGameCardNumber")),
          m_heart(services.pictures.find("icon_heart").value_or(gfx::PictureId{})) {
        screen.get<ui::DrawingArea>("MemoryGamePage").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.on("onMemoryGameDown", [this](const ui::EventArgs& args) { m_game.clicked(args.position, m_frameMs); });
        screen.on("onMemoryGameLeave", [this](const ui::EventArgs&) { m_game.leave(); });
    }

    void MemoryGameWindow::open(std::uint64_t nowMs) {
        m_game.open(nowMs);
        m_leave.setVisible(false);
        m_form.setVisible(true);
    }

    void MemoryGameWindow::close() { m_form.setVisible(false); }

    void MemoryGameWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        gfx::PictureLibrary& pictures = m_services.pictures;
        m_frameMs = nowMs;
        const MemoryGame::Frame frame = m_game.step(nowMs, m_instructionsOpen());
        m_leave.setVisible(m_game.formShown());
        if (const auto board = pictures.find("CJ_BG1")) {
            canvas.drawPicture(*board, m_boardSpot.at());
        }
        if (m_heart.isValid()) {
            for (int life = 1; life <= m_game.lives(); ++life) {
                canvas.drawPictureScaled(
                    m_heart,
                    {life * m_heartSpot.pitch().x + m_heartSpot.rect().x, m_heartSpot.rect().y, m_heartSpot.rect().width, m_heartSpot.rect().height}
                );
            }
        }
        if (frame.circles) {
            if (const auto circle = pictures.find("little_circle")) {
                for (int card = 1; card <= m_game.positionCount(); ++card) {
                    canvas.drawPicture(*circle, {m_game.card(card).x, m_game.card(card).y});
                }
            }
        }
        if (frame.numbersOnly || frame.circles) {
            for (int card = 1; card <= m_game.cardCount(); ++card) {
                if (m_game.card(card).revealed) {
                    drawGameNumber(
                        canvas,
                        "Num_White5_1",
                        {m_game.card(card).x + m_cardNumberSpot.at().x, m_game.card(card).y + m_cardNumberSpot.at().y},
                        m_game.card(card).number,
                        kCardNumberStyle,
                        true
                    );
                }
            }
        }
        if (const auto score = pictures.find("MiniGame_Score_1")) {
            canvas.drawPicture(*score, m_scoreSpot.at());
        }
        drawGameNumber(canvas, "Num_White5_1", m_scoreNumberSpot.at(), m_game.score(), kNumberStyle, true);
        if (const auto time = pictures.find("MiniGame_Time_1")) {
            canvas.drawPicture(*time, m_timeSpot.at());
        }
        drawGameNumber(canvas, "Num_White5_1", m_timeNumberSpot.at(), m_game.timeLeft(), kNumberStyle, true);
        if (frame.countdown == 0) {
            drawGameLetters(canvas, m_messageSpot.at(), "Go", 0, 1.0F, m_messageSpot.pitch().x, 0, true);
        } else if (frame.countdown > 0) {
            drawGameNumber(canvas, "Num_YellowRed10_2", m_messageSpot.at(), frame.countdown, kCountdownStyle, true);
        }
        if (frame.rightSign) {
            if (const auto sign = pictures.find("Icon_O_1")) {
                canvas.drawPicture(*sign, m_rightSignSpot.at());
            }
        }
        if (frame.wrongSign) {
            if (const auto sign = pictures.find("Icon_X_1")) {
                canvas.drawPicture(*sign, m_wrongSignSpot.at());
            }
        }
        if (frame.win || frame.lose) {
            drawGameLetters(canvas, m_messageSpot.at(), frame.win ? "You Win" : "You Lose", 0, 1.0F, m_messageSpot.pitch().x, 0, true);
        }
    }

}
