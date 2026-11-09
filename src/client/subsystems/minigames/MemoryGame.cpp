#include "client/subsystems/minigames/MemoryGame.h"

#include <algorithm>
#include <cstdlib>

namespace client {

    namespace {

        constexpr std::uint64_t kShowCardsMs = 1000; // how long the card numbers show at round start, ms
        constexpr std::uint64_t kRoundOkMs = 2500; // how long the right sign shows after a cleared round, ms
        constexpr std::uint64_t kMissSignMs = 1500; // how long the wrong sign shows after a miss, ms
        constexpr std::uint64_t kMissMs = 3000; // time after a miss until the round restarts or the game ends, ms
        constexpr std::uint64_t kResultMs = 5000; // how long the win/lose screen shows, ms
        constexpr std::uint64_t kSecondMs = 1000; // one second, ms (countdown and round timer step)
        constexpr int kBoardLeft = 200; // left edge of the card placement area, px
        constexpr int kBoardTop = 100; // top edge of the card placement area, px
        constexpr int kBoardWidth = 362; // width of the card placement area, px
        constexpr int kBoardHeight = 263; // height of the card placement area, px
        constexpr int kLevelRounds = 4; // cleared rounds per level step
        constexpr int kMostLevel = 7; // highest level (a round has level + 2 cards, so at most 9)
        constexpr int kCountdownStart = 4; // countdown start value (shows 3, 2, 1, Go)
        constexpr int kStartSeconds = 10; // seconds allowed to click the cards in a round
        constexpr int kTries = 500; // max random placement attempts per round before giving up
        const char* const kMusic = "BGM0019";

    }

    MemoryGame::MemoryGame(
        GameHooks hooks
    )
        : m_hooks(std::move(hooks)) {}

    void MemoryGame::open(std::uint64_t nowMs) {
        m_cards = {};
        m_phase = 2; // phase 2: counting down
        m_countdown = kCountdownStart;
        m_countdownMs = nowMs;
        m_timeLeft = kStartSeconds;
        m_nextCard = 1;
        m_expected = 1;
        m_nextPos = 1;
        m_showCards = true;
        m_level = 1;
        m_missed = false;
        m_roundCleared = false;
        m_missSoundPending = false;
        m_revealOnMiss = true;
        m_won = false;
        m_score = 0;
        m_lives = kStartLives;
        m_formShown = false;
        generate();
        m_hooks.stopMusic();
        m_musicPending = true;
    }

    void MemoryGame::reset(std::uint64_t nowMs) {
        m_nextCard = 1;
        m_expected = 1;
        m_nextPos = 1;
        m_showCards = true;
        m_revealOnMiss = true;
        m_phase = 2; // phase 2: counting down
        m_countdown = kCountdownStart;
        m_countdownMs = nowMs;
        m_timeLeft = kStartSeconds;
        generate();
    }

    void MemoryGame::generate() {
        // a round has level + 2 cards, numbered from 1 to 9
        while (m_level + 2 >= m_nextCard) {
            const int number = random(9) + 1;
            bool taken = false;
            for (int i = 1; i <= m_nextCard; ++i) {
                if (m_cards[static_cast<std::size_t>(i)].number == number) {
                    taken = true;
                    break;
                }
            }
            if (!taken) {
                m_cards[static_cast<std::size_t>(m_nextCard)].number = number;
                ++m_nextCard;
            }
        }
        sortCards();
        int tries = 0;
        while (m_nextPos <= m_level + 2) {
            if (++tries > kTries) {
                break;
            }
            const int x = random(kBoardWidth) + kBoardLeft + 1;
            const int y = random(kBoardHeight) + kBoardTop + 1;
            bool taken = false;
            for (int i = 1; i <= m_nextPos; ++i) {
                if (std::abs(x - m_cards[static_cast<std::size_t>(i)].x) < kCardWidth && std::abs(y - m_cards[static_cast<std::size_t>(i)].y) < kCardHeight) {
                    taken = true;
                    break;
                }
            }
            if (!taken) {
                m_cards[static_cast<std::size_t>(m_nextPos)].x = x;
                m_cards[static_cast<std::size_t>(m_nextPos)].y = y;
                ++m_nextPos;
            }
        }
    }

    void MemoryGame::sortCards() {
        for (int pass = m_nextCard - 2; pass >= 1; --pass) {
            bool swapped = false;
            for (int i = 1; i <= pass; ++i) {
                Card& first = m_cards[static_cast<std::size_t>(i)];
                Card& second = m_cards[static_cast<std::size_t>(i + 1)];
                if (first.number > second.number) {
                    std::swap(first.number, second.number);
                    swapped = true;
                }
            }
            if (!swapped) {
                break;
            }
        }
    }

    void MemoryGame::countdownStep(std::uint64_t nowMs) {
        if (m_phase != 2) { // only while counting down
            return;
        }
        if (!elapsed(m_countdownMs, kSecondMs, nowMs) && m_countdown >= 0) {
            return;
        }
        if (m_countdown < 0) {
            m_phase = 3; // phase 3: playing
            m_showCardsMs = nowMs;
            if (m_musicPending) {
                m_hooks.playMusic(kMusic);
                m_musicPending = false;
            }
        }
        --m_countdown;
        if (m_countdown > 0) {
            m_hooks.playSound("sound/wav0145.wav"); // countdown tick sound
        }
        if (m_countdown == 0) {
            m_hooks.playSound("sound/wav1610.wav"); // "Go" sound
        }
        m_countdownMs = nowMs;
    }

    void MemoryGame::tick(std::uint64_t nowMs) {
        if (m_phase == 3 && elapsed(m_countdownMs, kSecondMs, nowMs)) { // phase 3: playing
            --m_timeLeft;
            m_countdownMs = nowMs;
        }
    }

    MemoryGame::Frame MemoryGame::step(std::uint64_t nowMs, bool instructionsOpen) {
        if (!instructionsOpen) {
            m_formShown = true;
            countdownStep(nowMs);
            if (m_phase == 3) { // phase 3: playing
                if (m_timeLeft == 0) {
                    --m_lives;
                    m_missed = true;
                    m_missSoundPending = true;
                }
                tick(nowMs);
            }
        }
        Frame frame;
        if (m_phase == 3 || m_phase == 1) { // phase 3: playing, 1: showing a result
            if (m_showCards) {
                if (!elapsed(m_showCardsMs, kShowCardsMs, nowMs)) {
                    frame.numbersOnly = true;
                } else {
                    m_showCards = false;
                    for (int i = 1; i < m_nextCard; ++i) {
                        m_cards[static_cast<std::size_t>(i)].revealed = false;
                    }
                }
            } else {
                frame.circles = true;
            }
        }
        if (m_phase == 2 && m_countdown >= 0 && m_countdown <= 3) { // phase 2: counting down; the countdown shows 3, 2, 1, Go (0)
            frame.countdown = m_countdown;
        }
        if (m_roundCleared) {
            m_phase = 1; // phase 1: showing a result
            if (!elapsed(m_roundOkMs, kRoundOkMs, nowMs)) {
                frame.rightSign = true;
            } else if (m_won) {
                m_phase = 0; // phase 0: over
                m_roundCleared = false;
            } else {
                m_roundCleared = false;
                reset(nowMs);
            }
        }
        if (m_missed) {
            m_phase = 1; // phase 1: showing a result
            if (m_missSoundPending) {
                m_hooks.playSound("sound/wav1607.wav"); // miss sound
                m_missSoundPending = false;
                m_missMs = nowMs;
            }
            if (!elapsed(m_missMs, kMissSignMs, nowMs)) {
                frame.wrongSign = true;
            } else if (m_revealOnMiss) {
                m_revealOnMiss = false;
                for (int i = 1; i < m_nextCard; ++i) {
                    m_cards[static_cast<std::size_t>(i)].revealed = true;
                }
            }
            if (elapsed(m_missMs, kMissMs, nowMs)) {
                if (m_lives == 0) {
                    m_lives = -1; // -1: out of lives, game over handled
                    m_phase = 0; // phase 0: over
                    m_resultMs = nowMs;
                    if (m_score >= kRoundsToWin) {
                        m_won = true;
                    }
                } else if (!m_won) {
                    reset(nowMs);
                }
                m_missed = false;
            }
        }
        if (m_phase == 0) { // phase 0: over
            frame.win = m_won;
            frame.lose = !m_won;
            if (elapsed(m_resultMs, kResultMs, nowMs)) {
                finish(m_won ? 1 : 0); // result 1: won, 0: lost
            }
        }
        return frame;
    }

    void MemoryGame::clicked(core::Point point, std::uint64_t nowMs) {
        int hit = 0;
        for (int i = 1; i < m_nextPos; ++i) {
            const Card& card = m_cards[static_cast<std::size_t>(i)];
            if (point.x >= card.x && point.x <= card.x + kCardWidth && point.y >= card.y && point.y <= card.y + kCardHeight) {
                hit = i;
                break;
            }
        }
        Card& card = m_cards[static_cast<std::size_t>(hit)];
        if (!card.revealed && hit > 0 && !m_missed) {
            if (hit == m_expected) {
                m_hooks.playSound("sound/wav1611.wav"); // right card sound
                card.revealed = true;
                ++m_expected;
            } else {
                --m_lives;
                m_missed = true;
                m_missSoundPending = true;
                card.revealed = true;
            }
        }
        if (m_expected == m_nextCard) {
            m_expected = 1;
            ++m_score;
            m_level = std::min(m_score / kLevelRounds + 1, kMostLevel);
            m_hooks.playSound("sound/wav0152.wav"); // round cleared sound
            m_roundCleared = true;
            m_roundOkMs = nowMs;
        }
        if (m_score >= kRoundsToWin) {
            m_resultMs = nowMs;
            m_won = true;
        }
    }

    void MemoryGame::leave() {
        m_hooks.stopMusic();
        m_hooks.endGame();
    }

    void MemoryGame::finish(int result) {
        m_hooks.stopMusic();
        m_hooks.finishGame(result);
    }

}
