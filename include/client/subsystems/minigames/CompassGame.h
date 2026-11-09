#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_COMPASS_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_COMPASS_GAME_H

#include "client/subsystems/minigames/GameHooks.h"
#include "core/Bytes.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief The Golden Compass quiz minigame logic (original HL_Minigame.pas TWaterMelonGame without drawing; messages 57/10).
    class CompassGame {
        public:
            static constexpr int kStartLives = 3; // lives at the start
            static constexpr int kQuestionSeconds = 6; // seconds to answer a question
            static constexpr int kMostPlayers = 20; // most players in a round, 1-based
            static constexpr int kAnswers = 4; // answer choices per question, 1-based

            /// @brief Callbacks the game uses to reach the screen, chat and player state.
            struct Hooks : GameHooks {
                std::function<void(const std::string&, bool byPlayer)> chat;
                std::function<std::string(std::int32_t)> playerName;
                std::function<std::int32_t()> ownId;
                std::function<int()> mapId;
                std::function<void()> enterScreen;
                std::function<void()> leaveScreen;
                std::function<void(const std::string& question, const std::vector<std::string>& choices, std::function<int()> shownNumber)> showQuestion;
                std::function<void()> clearQuestion;
                std::function<void(const std::string&, int ms)> showAnswer;
                std::function<void(int slot)> playerActs;
            };

            /// @brief Creates the game with its callbacks.
            /// @param hooks Callbacks into the client.
            explicit CompassGame(
                Hooks hooks
            );

            /// @brief Clears the question, answers, lives and players (original ResetGoldenCompass).
            void reset();

            /// @brief Handles a 57/10 server packet by its sub id (original RecvGoldenCompassPacket).
            /// @param payload Packet bytes (sub id after a 2 byte header).
            /// @param nowMs Current time in ms, starts the countdown for a question.
            void receive(const core::Bytes& payload, std::uint64_t nowMs);

            /// @brief Per-frame countdown: updates the number shown and plays the tick sounds at 2, 1 and 0 (original DrawCompassCountdown).
            /// @param nowMs Current time in ms.
            void step(std::uint64_t nowMs);

            /// @brief Asks to leave when the player is not on the game's map (original TWaterMelonGame.DrawControl).
            void leaveWhenOffMap();

            /// @brief Sends the leave request, 57/10 sub 2 (original RequestLeaveGoldenCompass).
            void leaveRequested();

            /// @brief Lives left.
            /// @return Lives (original FCompassState.V[6]).
            int lives() const { return m_lives; }

            /// @brief Number of players taking part.
            /// @return Player count.
            int playerCount() const { return m_playerCount; }

            /// @brief Name of the player at a place.
            /// @param place Place number, 1-based.
            /// @return The name, empty when none.
            const std::string& playerName(int place) const { return m_playerNames[static_cast<std::size_t>(place)]; }

            /// @brief Tells whether a place is the own player.
            /// @param place Place number, 1-based.
            /// @return True when the name at the place is the own name.
            bool isOwnPlace(int place) const;

        private:
            Hooks m_hooks;
            std::string m_question;
            std::array<std::string, kAnswers + 1> m_answers;
            std::array<std::string, kMostPlayers + 1> m_playerNames;
            int m_playerCount = 0;
            int m_lives = kStartLives;
            int m_tickSound = 2; // seconds left at which the next countdown tick sounds
            std::uint64_t m_countdownMs = 0;
            int m_shownNumber = -1;
    };

}

#endif
