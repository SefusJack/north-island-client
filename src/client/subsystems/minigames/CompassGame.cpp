#include "client/subsystems/minigames/CompassGame.h"

#include "core/ByteReader.h"

#include <algorithm>
#include <cmath>

namespace client {

    namespace {

        constexpr int kQuestionColumns = 62; // question text is padded with spaces to this many characters
        constexpr int kFallingSlot = 27; // character action 27: falling (a life lost)
        constexpr int kCheeringSlot = 58; // character action 58: cheering (right answer)
        constexpr int kAnswerShownMs = 3000; // how long the right answer shows, ms
        constexpr int kCompassMap = 10095; // map 10095: Quiz Zone (the Golden Compass map)

    }

    CompassGame::CompassGame(
        Hooks hooks
    )
        : m_hooks(std::move(hooks)) {}

    bool CompassGame::isOwnPlace(int place) const {
        const std::string& name = m_playerNames[static_cast<std::size_t>(place)];
        return !name.empty() && name == m_hooks.playerName(m_hooks.ownId());
    }

    void CompassGame::reset() {
        m_question.clear();
        for (std::string& answer : m_answers) {
            answer.clear();
        }
        m_tickSound = 2; // first countdown tick at 2 seconds left
        m_lives = kStartLives;
        m_countdownMs = 0;
        m_shownNumber = -1;
        m_playerCount = 0;
        for (std::string& name : m_playerNames) {
            name.clear();
        }
        m_hooks.clearQuestion();
    }

    void CompassGame::receive(const core::Bytes& payload, std::uint64_t nowMs) {
        core::ByteReader reader(payload, 2); // skip the 2 byte packet header (main/sub ids)
        const int sub = reader.u8();
        switch (sub) {
            case 1: // 57/10 sub 1: joined, enter the compass screen
                m_hooks.chat("Right click to move in case you can't see your character in a crowd", false);
                m_hooks.enterScreen();
                reset();
                return;
            case 2: // 57/10 sub 2: left, reset and leave the screen
                reset();
                m_hooks.leaveScreen();
                return;
            case 3: { // 57/10 sub 3: question (u8 len, text) and 4 answers (u8 len, text)
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
                m_question = std::move(question);
                m_answers = std::move(answers);
                std::vector<std::string> choices;
                for (int answer = 1; answer <= kAnswers; ++answer) {
                    choices.push_back("(" + std::to_string(answer) + ") " + m_answers[static_cast<std::size_t>(answer)]);
                }
                std::string text = m_question;
                if (text.size() < kQuestionColumns) {
                    text.append(kQuestionColumns - text.size(), ' ');
                }
                m_shownNumber = -1;
                m_hooks.showQuestion(text, choices, [this] { return m_shownNumber; });
                m_countdownMs = nowMs;
                return;
            }
            case 4: { // 57/10 sub 4: right answer (u8) and lives left (u8)
                const int answer = reader.u8();
                const int lives = reader.u8();
                if (reader.failed() || answer < 1 || answer > kAnswers) {
                    return;
                }
                if (m_lives > lives) {
                    m_hooks.playSound("sound/wav1609.wav"); // wrong answer, a life lost
                    m_hooks.playerActs(kFallingSlot);
                } else if (m_lives == lives) {
                    m_hooks.playSound("sound/wav1610.wav"); // right answer, no life change
                    m_hooks.playerActs(kCheeringSlot);
                } else {
                    m_hooks.playSound("sound/wav0150.wav");
                    m_hooks.playerActs(kCheeringSlot);
                }
                m_lives = lives;
                m_hooks.clearQuestion();
                m_hooks.showAnswer("The correct answer is...(" + std::to_string(answer) + ")" + m_answers[static_cast<std::size_t>(answer)], kAnswerShownMs);
                return;
            }
            case 5: { // 57/10 sub 5: ranking list (u8 count, then u8 len + name each)
                const int count = reader.u8();
                m_hooks.chat("Place in [Golden Compass]:", true);
                for (int place = 1; place <= count; ++place) {
                    const int length = reader.u8();
                    const std::string name = reader.text(static_cast<std::size_t>(length));
                    if (reader.failed()) {
                        return;
                    }
                    m_hooks.chat("The " + std::to_string(place) + " place in the competition : " + name, true);
                }
                return;
            }
            case 6: // 57/10 sub 6: washed out of the game
                m_hooks.chat("You have been washed out! If you want to continue joining, please sign up again", false);
                return;
            case 7: { // 57/10 sub 7: players taking part (u8 count, i32 id each)
                const int count = std::min<int>(reader.u8(), kMostPlayers);
                for (std::string& name : m_playerNames) {
                    name.clear();
                }
                m_playerCount = count;
                for (int place = 1; place <= count; ++place) {
                    const std::int32_t id = reader.i32();
                    if (reader.failed()) {
                        return;
                    }
                    m_playerNames[static_cast<std::size_t>(place)] = m_hooks.playerName(id);
                }
                return;
            }
            case 8: { // 57/10 sub 8: schedule notice (u8 which message)
                const int which = reader.u8();
                if (which == 1) {
                    m_hooks.message("The Golden Compass is during 7:00pm-7:30pm every Tuesday.", 3000); // message display time in ms
                } else if (which == 2) {
                    m_hooks.message("time to participate in the Golden Compass activity.", 3000); // message display time in ms
                }
                return;
            }
            default: // unknown sub id: ignored
                return;
        }
    }

    void CompassGame::step(std::uint64_t nowMs) {
        if (m_countdownMs == 0) {
            return;
        }
        const double elapsed = static_cast<double>(nowMs - m_countdownMs) / 1000.0; // ms per second
        const int seconds = static_cast<int>(std::ceil(kQuestionSeconds - elapsed));
        if (seconds <= 0) {
            m_countdownMs = 0;
        }
        if (seconds == m_tickSound) {
            m_hooks.playSound("sound/wav0145.wav");
            if (m_tickSound == 0) {
                m_tickSound = 3; // wraps back to 2 after the decrement below
            }
            --m_tickSound;
        }
        m_shownNumber = seconds;
    }

    void CompassGame::leaveRequested() { m_hooks.send({57, 10, 2}); } // packet 57/10, sub 2: leave request

    void CompassGame::leaveWhenOffMap() {
        if (m_hooks.mapId() != kCompassMap) {
            leaveRequested();
        }
    }

}
