#include "client/subsystems/minigames/QuizGame.h"

#include "core/ByteReader.h"

#include <algorithm>
#include <cmath>

namespace client {

    namespace {

        constexpr int kAnswerLeft = 250; // left edge of the answer rows, px
        constexpr int kAnswerTop = 184; // top of the first answer row, px
        constexpr int kAnswerPitch = 68; // vertical distance between answer rows, px
        constexpr int kAnswerWidth = 288; // width of an answer row hit area, px
        constexpr int kAnswerHeight = 24; // height of an answer row hit area, px
        constexpr std::uint64_t kResultMs = 3000; // how long the answer result shows, ms
        constexpr std::uint64_t kDotMs = 500; // step of the waiting dots animation, ms
        constexpr std::uint64_t kBlinkMs = 100; // host eyes-closed time and selection box flash step, ms
        const std::string kEmpty;

    }

    QuizGame::QuizGame(
        Hooks hooks
    )
        : m_hooks(std::move(hooks)) {}

    int QuizGame::randomSeconds() { return std::uniform_int_distribution<int>(7, 11)(m_random); } // host blink wait: 7 to 11 seconds

    const std::string& QuizGame::answerText(int answer) const {
        if (answer < 1 || answer > kAnswers) {
            return kEmpty;
        }
        const std::string& pending = m_answers[static_cast<std::size_t>(answer)];
        return pending.empty() ? m_answerTexts[static_cast<std::size_t>(answer)] : pending;
    }

    bool QuizGame::isOwnPlace(int place) const {
        const std::string& name = m_playerNames[static_cast<std::size_t>(place)];
        return !name.empty() && name == m_hooks.playerName(m_hooks.ownId());
    }

    void QuizGame::reset(std::uint64_t nowMs) {
        m_countdownMs = 0;
        m_resultMs = 0;
        m_flashMs = 0;
        m_hostWaitMs = 0;
        m_hostSinceMs = nowMs;
        m_dotsMs = nowMs;
        m_dots = 0;
        m_tickSound = 2; // first countdown tick sound plays at 2 seconds left
        m_playerCount = 0;
        m_lives = kStartLives;
        m_lost = false;
        m_resultSoundPending = false;
        m_resultOn = false;
        m_over = false;
        m_notListed = false;
        m_resultsListed = false;
        m_lifeGained = false;
        m_questionReceived = false;
        m_selected = 0;
        m_hover = 0;
        m_picked = 0;
        m_result = Result::None;
        m_question.clear();
        m_shownQuestion.clear();
        m_hostState = 0;
        for (std::string& answer : m_answers) {
            answer.clear();
        }
        for (std::string& answer : m_answerTexts) {
            answer.clear();
        }
        for (std::string& name : m_playerNames) {
            name.clear();
        }
        m_questionId = 0;
    }

    void QuizGame::keepQuestion() {
        if (m_shownQuestion.empty()) {
            m_shownQuestion = m_question;
            m_question.clear();
            for (int answer = 1; answer <= kAnswers; ++answer) {
                m_answerTexts[static_cast<std::size_t>(answer)] = m_answers[static_cast<std::size_t>(answer)];
                m_answers[static_cast<std::size_t>(answer)].clear();
            }
        }
    }

    void QuizGame::receive(const core::Bytes& payload, std::uint64_t nowMs) {
        core::ByteReader reader(payload, 2);
        const int sub = reader.u8();
        switch (sub) {
            case 1: { // 57/8/1 (server to client): a question with id, text and four answers
                const int id = reader.u16();
                const int length = reader.u8();
                std::string question = reader.text(static_cast<std::size_t>(length));
                std::array<std::string, kAnswers + 1> answers;
                for (int answer = 1; answer <= kAnswers; ++answer) {
                    const int size = reader.u8();
                    answers[static_cast<std::size_t>(answer)] = reader.text(static_cast<std::size_t>(size));
                }
                if (reader.failed()) {
                    return;
                }
                m_questionId = id;
                m_question = std::move(question);
                m_shownQuestion.clear();
                m_answers = std::move(answers);
                for (std::string& text : m_answerTexts) {
                    text.clear();
                }
                m_questionReceived = true;
                m_result = Result::None;
                m_lifeGained = false;
                m_selected = 0;
                m_picked = 0;
                m_countdownMs = nowMs;
                m_tickSound = 2;
                m_resultOn = false;
                m_resultMs = 0;
                m_hostState = 0;
                return;
            }
            case 2: { // 57/8/2 (server to client): the TOP list, player ids best first
                const int count = std::min<int>(reader.u8(), kMostPlayers);
                for (std::string& name : m_playerNames) {
                    name.clear();
                }
                m_notListed = true;
                m_playerCount = count;
                const std::string own = m_hooks.playerName(m_hooks.ownId());
                for (int place = 1; place <= count; ++place) {
                    const std::int32_t id = reader.i32();
                    if (reader.failed()) {
                        return;
                    }
                    m_playerNames[static_cast<std::size_t>(place)] = m_hooks.playerName(id);
                    if (m_over) {
                        m_resultsListed = true;
                        if (m_notListed && m_playerNames[static_cast<std::size_t>(place)] == own) {
                            m_notListed = false;
                        }
                    }
                }
                return;
            }
            case 3: // 57/8/3 (server to client): enter the Q&A screen
                m_hooks.enterScreen();
                reset(nowMs);
                return;
            case 4: // 57/8/4 (server to client): leave the Q&A screen
                reset(nowMs);
                m_hooks.leaveScreen();
                return;
            case 5: { // 57/8/5 (server to client): answer result (right answer, lives left, result)
                const int picked = reader.u8();
                const int lives = reader.u8();
                const int result = reader.u8();
                if (reader.failed()) {
                    return;
                }
                m_resultSoundPending = true;
                m_resultOn = true;
                m_countdownMs = 0;
                m_picked = picked;
                const int before = m_lives;
                m_lives = lives;
                m_result = static_cast<Result>(result);
                if (!m_over) {
                    keepQuestion();
                }
                if (m_lives > before) {
                    m_lifeGained = true;
                }
                return;
            }
            case 6: // 57/8/6 (server to client): washed out notice in chat
                m_hooks.chat("You have been washed out! If you want to continue joining, please sign up again", false);
                return;
            case 7: { // 57/8/7 (server to client): the question text again, new countdown
                const int length = reader.u8();
                const std::string question = reader.text(static_cast<std::size_t>(length));
                if (reader.failed()) {
                    return;
                }
                m_question = question;
                m_countdownMs = nowMs;
                return;
            }
            case 8: { // 57/8/8 (server to client): notice, u8 which (1 activity time, 2 go to branch server 1)
                const int which = reader.u8();
                if (which == 1) {
                    m_hooks.message("The Q & A time is: \rDuring 7:00-7:30 PM on every Wednesday", 3000); // message display time in ms
                } else if (which == 2) {
                    m_hooks.message("Please go to branch server 1 to join in the Q & A activity", 3000); // message display time in ms
                }
                return;
            }
            case 9: // 57/8/9 (server to client): the game is over
                m_over = true;
                keepQuestion();
                return;
            case 10: { // 57/8/10 (server to client): final places as names, printed in chat
                const int count = reader.u8();
                for (std::string& name : m_playerNames) {
                    name.clear();
                }
                m_hooks.chat("The results of [Q & A ] activity:", true);
                for (int place = 1; place <= count; ++place) {
                    const int length = reader.u8();
                    const std::string name = reader.text(static_cast<std::size_t>(length));
                    if (reader.failed()) {
                        return;
                    }
                    if (place <= kMostPlayers) {
                        m_playerNames[static_cast<std::size_t>(place)] = name;
                    }
                    m_hooks.chat("The " + std::to_string(place) + " place in the competition : " + name, true);
                }
                return;
            }
            case 11: // 57/8/11 (server to client): answer was too late, select again
                m_selected = 0;
                m_questionReceived = true;
                m_hooks.message("please reselect the anwer to the question because you are so late.", 2200); // message display time in ms
                return;
            default: // unknown sub id: ignored
                return;
        }
    }

    void QuizGame::hover(core::Point mouse) {
        m_hover = 0;
        if (m_picked != 0 || m_selected != 0) {
            return;
        }
        for (int answer = 1; answer <= kAnswers; ++answer) {
            const int top = (answer - 1) * kAnswerPitch + kAnswerTop;
            if (!answerText(answer).empty() && mouse.x >= kAnswerLeft && mouse.x < kAnswerLeft + kAnswerWidth && mouse.y >= top &&
                mouse.y < top + kAnswerHeight) {
                m_hover = answer;
                return;
            }
        }
    }

    void QuizGame::clicked(core::Point mouse) {
        if (!m_questionReceived || m_picked != 0 || m_selected != 0) {
            return;
        }
        hover(mouse);
        if (m_hover == 0) {
            return;
        }
        m_selected = m_hover;
        m_questionReceived = false;
        // packet 57/8/3 then answer, question id low byte (0xFF mask), high byte (>> 8)
        m_hooks.send(
            {57, 8, 3, static_cast<std::uint8_t>(m_selected), static_cast<std::uint8_t>(m_questionId & 0xFF), static_cast<std::uint8_t>(m_questionId >> 8)}
        );
        m_hooks.playSound("sound/wav0152.wav"); // answer chosen sound
        if (!m_over) {
            keepQuestion();
        }
    }

    void QuizGame::leaveRequested() { m_hooks.send({57, 8, 2}); } // packet 57/8/2: request to leave

    void QuizGame::leaveWhenOffMap() {
        constexpr int kQuizMap = 10079; // map 10079: Quiz Zone
        if (m_hooks.mapId() != kQuizMap) {
            leaveRequested();
        }
    }

    int QuizGame::secondsLeft(std::uint64_t nowMs) {
        if (m_countdownMs == 0) {
            return 0;
        }
        const double elapsed = static_cast<double>(nowMs - m_countdownMs) / 1000.0; // ms per second
        const int seconds = static_cast<int>(std::ceil(kQuestionSeconds - elapsed));
        if (seconds == 0) {
            m_countdownMs = 0;
        }
        if (m_selected == 0 && seconds == m_tickSound) {
            m_hooks.playSound("sound/wav0145.wav"); // countdown tick sound
            if (m_tickSound == 0) {
                m_tickSound = 3; // wraps to -1 after the decrement: no more tick sounds
            }
            --m_tickSound;
        }
        return std::max(seconds, 0);
    }

    void QuizGame::stepHost(std::uint64_t nowMs) {
        if (m_hostState != 0 && m_hostState != 1) {
            return;
        }
        if (nowMs - m_hostSinceMs <= m_hostWaitMs) {
            return;
        }
        if (m_hostState == 0) {
            m_hostWaitMs = kBlinkMs;
            m_hostSinceMs = nowMs;
            m_hostState = 1;
        } else {
            m_hostWaitMs = static_cast<std::uint64_t>(randomSeconds()) * 1000; // ms per second
            m_hostSinceMs = nowMs;
            m_hostState = 0;
        }
    }

    void QuizGame::stepResult(std::uint64_t nowMs) {
        if (!m_resultOn) {
            return;
        }
        if (m_resultMs == 0) {
            m_resultMs = nowMs;
        }
        m_hostState = m_result == Result::Right ? 3 : 4; // host frame: 3 pleased, 4 sorry
        if (nowMs - m_resultMs > kResultMs) {
            m_resultMs = 0;
            m_hostState = 0;
            m_resultOn = false;
            m_result = Result::None;
            m_shownQuestion.clear();
            for (int answer = 1; answer <= kAnswers; ++answer) {
                m_answerTexts[static_cast<std::size_t>(answer)].clear();
            }
            m_flashMs = 0;
            m_hostWaitMs = static_cast<std::uint64_t>(randomSeconds()) * 1000; // ms per second
            m_hostSinceMs = nowMs;
            if (m_lives == 0) {
                m_lost = true;
            }
        }
    }

    int QuizGame::dots(std::uint64_t nowMs) {
        if (nowMs - m_dotsMs > kDotMs) {
            m_dotsMs = nowMs;
            m_dots = (m_dots + 1) % 3; // dots cycle 0..2
        }
        return m_dots;
    }

    bool QuizGame::selectionBoxDue(std::uint64_t nowMs) {
        if (m_flashMs == 0) {
            m_flashMs = nowMs;
        }
        if (nowMs - m_flashMs <= kBlinkMs) {
            return false;
        }
        m_flashMs = nowMs;
        return true;
    }

    void QuizGame::soundResult() {
        if (m_picked == 0 || !m_resultSoundPending) {
            return;
        }
        if (m_result == Result::Right) {
            const bool anyAnswer = !answerText(1).empty() || !answerText(2).empty() || !answerText(3).empty() || !answerText(4).empty();
            if (!anyAnswer) {
                return;
            }
            if (m_lifeGained) {
                m_hooks.playSound("sound/wav0150.wav"); // right answer and life gained sound
                m_lifeGained = false;
            } else {
                m_hooks.playSound("sound/wav1610.wav"); // right answer sound
            }
            m_resultSoundPending = false;
        } else if (m_result == Result::Wrong || m_result == Result::NoAnswer) {
            m_hooks.playSound("sound/wav1609.wav"); // wrong or no answer sound
            m_resultSoundPending = false;
        }
    }

}
