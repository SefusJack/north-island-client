#include "client/viewmodels/minigames/QuizWindow.h"

#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"

#include <string>

namespace client {

    namespace {

        constexpr int kStandingRight = 11; // player figure slot (pose) after a right answer / while waiting
        constexpr int kStandingSad = 30; // player figure slot (pose) after a wrong or missing answer
        constexpr int kLargeLine = 24; // line height of the large text, px
        constexpr int kBoxInset = 10; // selection box sits this many px above its answer text
        constexpr int kBoxPen = 3; // selection box line thickness, px

        /// @brief Loads QuizWindow.xml into the screen and returns its form.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The "QuizForm" element.
        ui::Element& loadForm(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("QuizWindow.xml"), services.loadContext());
            return screen.get<ui::Element>("QuizForm");
        }

    }

    QuizWindow::QuizWindow(
        GameServices& services,
        ui::Screen& screen,
        QuizGame::Hooks hooks,
        std::function<const CharacterLook&()> ownLook
    )
        : m_services(services),
          m_screen(screen),
          m_ownLook(std::move(ownLook)),
          m_game(std::move(hooks)),
          m_form(loadForm(services, screen)),
          m_background(services.pictures.loadFile(services.game.data("Menu/Skins/White/Form_Conundrum_1.jpg")).value_or(gfx::PictureId{})),
          m_timerBarSpot(screen.get<ui::Spot>("QuizTimerBar")),
          m_timerSecondsSpot(screen.get<ui::Spot>("QuizTimerSeconds")),
          m_playerSpot(screen.get<ui::Spot>("QuizPlayer")),
          m_hostSpot(screen.get<ui::Spot>("QuizHost")),
          m_livesSpot(screen.get<ui::Spot>("QuizLives")),
          m_participantsSpot(screen.get<ui::Spot>("QuizParticipants")),
          m_answersSpot(screen.get<ui::Spot>("QuizAnswers")),
          m_boxSpot(screen.get<ui::Spot>("QuizBox")),
          m_yellowSpot(screen.get<ui::Spot>("QuizYellow")),
          m_whiteSpot(screen.get<ui::Spot>("QuizWhite")) {
        screen.get<ui::DrawingArea>("QuizPage").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.on("onQuizUp", [this](const ui::EventArgs& args) { m_game.clicked(args.position); });
        screen.on("onQuizLeave", [this](const ui::EventArgs&) { m_game.leaveRequested(); });
    }

    void QuizWindow::open() { m_form.setVisible(true); }

    void QuizWindow::close() { m_form.setVisible(false); }

    void QuizWindow::drawPicture(gfx::Canvas& canvas, const char* name, core::Point at) {
        if (const auto picture = m_services.pictures.find(name)) {
            canvas.drawPicture(*picture, at);
        }
    }

    void QuizWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (canvas.isAnimationFrame()) {
            m_game.leaveWhenOffMap();
        }
        m_game.hover(m_screen.mouse());
        if (m_background.isValid()) {
            canvas.drawPicture(m_background, {0, 0});
        }
        drawCountdown(canvas, nowMs);
        const CharacterLook& look = m_ownLook();
        const int slot = m_game.result() == QuizGame::Result::Wrong || m_game.result() == QuizGame::Result::NoAnswer ? kStandingSad : kStandingRight;
        const int count = m_services.characters.figureFrameCount(look, slot);
        m_services.characters.drawFigure(canvas, look, m_playerSpot.at(), slot, m_animation.frame(nowMs, count));
        drawQuestion(canvas, nowMs);
        m_services.characters.drawPortrait(canvas, look, m_hostSpot.at(), m_game.hostState());
        m_game.stepHost(nowMs);
        drawLives(canvas);
        drawParticipants(canvas);
        drawResult(canvas, nowMs);
    }

    void QuizWindow::drawCountdown(gfx::Canvas& canvas, std::uint64_t nowMs) {
        drawPicture(canvas, "Icon_TimerBar", m_timerBarSpot.at());
        const int seconds = m_game.secondsLeft(nowMs);
        if (const auto segment = m_services.pictures.find("Icon_SecondBar_1")) {
            for (int second = 1; second <= seconds; ++second) {
                canvas.drawPicture(*segment, {m_timerSecondsSpot.at().x + (second - 1) * m_timerSecondsSpot.pitch().x, m_timerSecondsSpot.at().y});
            }
        }
    }

    void QuizWindow::drawQuestion(gfx::Canvas& canvas, std::uint64_t nowMs) {
        const gfx::TextStyle yellow{m_yellowSpot.color(), gfx::TextEffect::Outline, std::nullopt};
        const gfx::TextStyle white{m_whiteSpot.color(), gfx::TextEffect::Outline, std::nullopt};
        const std::string& question = m_game.questionText();
        if (!question.empty()) {
            canvas.drawLargeText({210, 60}, "Question: ", yellow, 144, kLargeLine); // label position and max width in pixels
            canvas.drawLargeText({334, 60}, question, yellow, 408, kLargeLine); // question text position and max width in pixels
        } else if (m_game.over()) {
            const std::string dots(static_cast<std::size_t>(3 + m_game.dots(nowMs)), '.'); // at least 3 dots, plus the animated ones
            canvas.drawLargeText({300, 86}, "Place in the competition" + dots, yellow, 384, kLargeLine); // text position and max width in pixels
            if (m_game.resultsListed()) {
                drawPicture(canvas, m_game.notListed() ? "Icon_Lost_1" : "Icon_GotIt_1", {290, 200}); // position of the result icon
                return;
            }
        } else {
            const std::string dots(static_cast<std::size_t>(1 + m_game.dots(nowMs)), '.');
            canvas.drawLargeText({300, 86}, "Waiting for questions" + dots, yellow, 432, kLargeLine); // text position and max width in pixels
        }
        if (m_game.lost()) {
            drawPicture(canvas, "Icon_Lost_1", {290, 200}); // position of the lost icon
            return;
        }
        bool boxAsked = false;
        for (int answer = 1; answer <= QuizGame::kAnswers; ++answer) {
            const std::string& text = m_game.answerText(answer);
            if (text.empty()) {
                continue;
            }
            const int top = (answer - 1) * m_answersSpot.pitch().y + m_answersSpot.at().y;
            const bool picking = m_game.selected() != 0 && m_game.picked() == 0;
            bool boxDue = false;
            if (picking && !boxAsked) {
                boxAsked = true;
                if (canvas.isAnimationFrame()) {
                    m_selectionBox = m_game.selectionBoxDue(nowMs);
                }
                boxDue = m_selectionBox;
            }
            if (boxDue || (m_game.selected() == 0 && m_game.hoverAnswer() == answer)) {
                const int boxTop = (picking ? m_game.selected() - 1 : answer - 1) * m_answersSpot.pitch().y + m_answersSpot.at().y - kBoxInset;
                canvas.drawRectOutline({m_boxSpot.at().x, boxTop, m_boxSpot.rect().width, m_boxSpot.rect().height}, m_yellowSpot.color(), kBoxPen);
            }
            canvas.drawLargeText({m_answersSpot.at().x, top}, "(" + std::to_string(answer) + ") " + text, white, 288, kLargeLine); // max text width in pixels
            if (m_game.picked() != 0) {
                if (m_game.picked() == answer) {
                    drawPicture(
                        canvas,
                        "Icon_Currect_1",
                        {245, (m_game.picked() - 1) * m_answersSpot.pitch().y + 171}
                    ); // x and y offset of the right-answer mark
                } else if (m_game.selected() == answer && m_game.result() == QuizGame::Result::Wrong) {
                    drawPicture(
                        canvas,
                        "Icon_Miss_1",
                        {245, (m_game.selected() - 1) * m_answersSpot.pitch().y + 174}
                    ); // x and y offset of the wrong-answer mark
                }
            }
        }
        if (!boxAsked && canvas.isAnimationFrame()) {
            m_selectionBox = false;
        }
        m_game.soundResult();
    }

    void QuizWindow::drawLives(gfx::Canvas& canvas) {
        const CharacterLook& look = m_ownLook();
        for (int life = 1; life <= m_game.lives(); ++life) {
            m_services.characters.drawLifeFace(canvas, look, {(life - 1) * m_livesSpot.pitch().x + m_livesSpot.at().x, m_livesSpot.at().y});
        }
    }

    void QuizWindow::drawParticipants(gfx::Canvas& canvas) {
        for (int place = 1; place <= m_game.playerCount(); ++place) {
            const std::string& name = m_game.playerName(place);
            const bool mine = m_game.isOwnPlace(place);
            const core::Point at{m_participantsSpot.at().x, (place - 1) * m_participantsSpot.pitch().y + m_participantsSpot.at().y};
            canvas.drawText(
                at,
                name.empty() ? "(Participants left)" : name,
                {mine ? m_yellowSpot.color() : m_whiteSpot.color(), gfx::TextEffect::Plain, std::nullopt},
                144,
                16
            ); // max name width and line height in pixels
        }
    }

    void QuizWindow::drawResult(gfx::Canvas& canvas, std::uint64_t nowMs) {
        if (!m_game.resultOn()) {
            return;
        }
        switch (m_game.result()) {
            case QuizGame::Result::Right: drawPicture(canvas, "Icon_O_1", {615, 185}); break; // right answer: O sign at {615,185}
            case QuizGame::Result::NoAnswer: drawPicture(canvas, "Icon_Miss", {608, 164}); break; // no answer given: miss sign at {608,164}
            default: drawPicture(canvas, "Icon_X_1", {629, 199}); break; // wrong answer: X sign at {629,199}
        }
        m_game.stepResult(nowMs);
    }

}
