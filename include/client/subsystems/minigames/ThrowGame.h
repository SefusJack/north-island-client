#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_THROW_GAME_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_THROW_GAME_H

#include "client/subsystems/minigames/GameHooks.h"
#include "client/subsystems/minigames/ThrowProjectile.h"
#include "client/subsystems/minigames/ThrowTeam.h"
#include "core/Point.h"
#include "gfx/PictureId.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <random>
#include <string>

namespace client {

    /// @brief The ball-throwing minigame (game kind 1): two teams of four with three hearts each, played frame by frame.
    /// @details Original: CH_MiniGame.pas CH_TThrowGame; only the result is reported to the server.
    class ThrowGame {
        public:
            static constexpr std::uint64_t kFrameMs = 30; // one game frame, milliseconds
            static constexpr int kProjectiles = 50; // most balls thrown in one game
            static constexpr int kPlayers = ThrowTeam::kMost;
            static constexpr int kDrawItems = 58; // entries depth-sorted for drawing (players first, balls from 9)

            /// @brief Callbacks the game needs from the host (NPC sprites, frame counts, pictures) on top of the common game hooks.
            struct Hooks : GameHooks {
                std::function<int(int npcId)> npcSprite;
                std::function<int(int spriteId, int slot)> spriteFrames;
                std::function<int(int slot)> playerFrames;
                std::function<gfx::PictureId(const std::string& name)> picture;
            };

            /// @brief Creates the game with its host callbacks.
            /// @param hooks Host callbacks.
            explicit ThrowGame(
                Hooks hooks
            );

            /// @brief Sets up both sides against the given NPC kind.
            /// @details Original: CH_TThrowGame.Create and StartThrowGame.
            /// @param npcId NPC kind that supplies the fighters.
            /// @param nowMs Current clock in ms.
            void open(int npcId, std::uint64_t nowMs);

            /// @brief Leaves the instructions page and starts playing.
            /// @details Original: FStateB.V[4].
            void begin() { m_begun = true; }

            /// @brief Leaves the game (the Exit button).
            /// @details Original: FinishThrowGame 0.
            void leave();

            /// @brief Handles a mouse button going down.
            /// @details Original: ThrowGameMouseDown.
            /// @param at Mouse position.
            /// @param left True for the left button.
            void pressed(core::Point at, bool left);

            /// @brief Handles a mouse button coming up.
            /// @details Original: ThrowGameMouseUp.
            /// @param at Mouse position.
            /// @param left True for the left button.
            void released(core::Point at, bool left);

            /// @brief Runs the frames that have passed since the last update.
            /// @details Original: UpdateThrowGame and DrawThrowGame.
            /// @param nowMs Current clock in ms.
            void update(std::uint64_t nowMs);

            /// @brief Frees a ball whose hit or landing effect is over.
            /// @details Original: DrawThrowGame's freeing of the ball.
            /// @param projectile Ball index.
            void finishLight(int projectile);

            /// @brief Tells whether the player has left the instructions.
            /// @return True once begun.
            bool begun() const { return m_begun; }

            /// @brief Gives the slot of the figure the player moves.
            /// @return Slot in the player's team.
            int playerSlot() const { return m_playerSlot; }

            /// @brief Gives the player's side.
            /// @return The team.
            const ThrowTeam& own() const { return m_own; }

            /// @brief Gives the rival side.
            /// @return The team.
            const ThrowTeam& rival() const { return m_rival; }

            /// @brief Gives how far the clock is from the last frame towards the next, for drawing between frames.
            /// @param nowMs Current clock in ms.
            /// @return Fraction from 0 to 1.
            double sinceFrame(std::uint64_t nowMs) const {
                return nowMs > m_lastFrameMs ? std::min(1.0, static_cast<double>(nowMs - m_lastFrameMs) / static_cast<double>(kFrameMs)) : 0.0;
            }

            /// @brief Gives the sprite id of the rival fighters.
            /// @return Sprite id.
            int fighterSprite() const { return m_fighterSprite; }

            /// @brief Gives a ball for change.
            /// @param index Ball index.
            /// @return The ball.
            ThrowProjectile& projectile(int index) { return m_projectiles[static_cast<std::size_t>(index)]; }

            /// @brief Gives a ball for reading.
            /// @param index Ball index.
            /// @return The ball.
            const ThrowProjectile& projectile(int index) const { return m_projectiles[static_cast<std::size_t>(index)]; }

            /// @brief Gives the item at a place of the draw order.
            /// @param place Place in the order.
            /// @return Item number (0: nothing yet).
            int drawItem(int place) const { return m_drawOrder[static_cast<std::size_t>(place)]; }

            /// @brief Gives the throw strength being charged.
            /// @return Strength from 0 to 15.
            int power() const { return m_power; }

            /// @brief Tells whether the power bar shows (the player's state 1).
            /// @return True while charging.
            bool charging() const { return m_own.states[static_cast<std::size_t>(m_playerSlot)] == 1; }

        private:
            /// @brief Runs one 30 ms game frame.
            /// @param nowMs Current clock in ms.
            void frame(std::uint64_t nowMs);

            /// @brief Lays the dead down and advances each person's animation frame (DrawThrowGame's part).
            /// @param nowMs Current clock in ms.
            void drawStep(std::uint64_t nowMs);

            /// @brief Creates both teams from the NPC kind.
            /// @param npcId NPC kind.
            /// @param nowMs Current clock in ms.
            void createTeams(int npcId, std::uint64_t nowMs);

            /// @brief Raises the throw strength while the button is held.
            void chargePower();

            /// @brief Applies the player's click or release as a walk or throw.
            void handleAction();

            /// @brief Plays the arm back frame by frame after the player's throw.
            void playRecovery();

            /// @brief Walks the player's figure toward the clicked place, then back to standing or on to the charge.
            void moveOwnThrower();

            /// @brief Works out the distances the player's figure has to walk.
            void calcOwnMoveDistance();

            /// @brief Tells whether the player's figure has reached its destination.
            /// @return True when arrived.
            bool ownThrowerArrived() const;

            /// @brief Throws a ball from a figure.
            /// @param index Figure slot.
            /// @param side 1 the player's side, 2 the rival's.
            /// @param power Throw strength.
            void launch(int index, int side, int power);

            /// @brief Sets a ball's target and arc from the thrower and strength.
            /// @param projectile Ball index.
            /// @param power Throw strength.
            /// @param side Throwing side.
            /// @param index Thrower slot.
            void aim(int projectile, int power, int side, int index);

            /// @brief Moves every flying ball one step.
            void moveProjectiles();

            /// @brief Lands balls that have reached their target.
            void landProjectiles();

            /// @brief Checks the flying balls against the people.
            void checkHits();

            /// @brief Tells whether a ball hits a person.
            /// @param projectile Ball index.
            /// @param index Person slot.
            /// @param side Side of the person.
            /// @return True on a hit.
            bool hits(int projectile, int index, int side) const;

            /// @brief Stands the player's hit people up again a second after the hit.
            void recoverHitThrowers();

            /// @brief Runs the machine's play for one side.
            /// @param side Side to play.
            void updateSideAi(int side);

            /// @brief Runs the AI's throwing step for a person.
            /// @param index Person slot.
            /// @param side Side of the person.
            void aiThrowStep(int index, int side);

            /// @brief Runs the AI's walking step for a person.
            /// @param index Person slot.
            /// @param side Side of the person.
            void aiWalkStep(int index, int side);

            /// @brief Runs the AI's recovery step for a person.
            /// @param index Person slot.
            /// @param side Side of the person.
            void aiRecover(int index, int side);

            /// @brief Works out the distances an AI person has to walk.
            /// @param index Person slot.
            /// @param side Side of the person.
            void calcAiMoveDistance(int index, int side);

            /// @brief Tells whether an AI person has reached its destination.
            /// @param index Person slot.
            /// @param side Side of the person.
            /// @return True when arrived.
            bool aiArrived(int index, int side) const;

            /// @brief Sorts people and balls from the top of the screen down (a ball by its target).
            void sortDrawOrder();

            /// @brief Ends the game when the player or a whole side is out of hearts, and reports the result three seconds later.
            void checkGameOver();

            /// @brief Finishes the game and reports the result through the host.
            /// @param result 1 won, 0 lost.
            void finish(int result);

            /// @brief Tells whether a timer has run for an interval, and restarts it if so.
            /// @param timer Timer start in ms.
            /// @param interval Interval in ms.
            /// @return True when elapsed.
            bool timerElapsed(std::uint64_t& timer, int interval) const;

            /// @brief Moves a point toward a target using per-axis tick timers.
            /// @param current Current position.
            /// @param target Target position.
            /// @param distX Distance to cover on x.
            /// @param distY Distance to cover on y.
            /// @param tickX Timer of the x axis.
            /// @param tickY Timer of the y axis.
            /// @param speed Speed factor.
            /// @return The new position.
            core::Point moveToward(
                core::Point current,
                core::Point target,
                int distX,
                int distY,
                std::uint64_t& tickX,
                std::uint64_t& tickY,
                double speed
            ) const;

            /// @brief Gives a side's team for change.
            /// @param side 1 the player's side, else the rival's.
            /// @return The team.
            ThrowTeam& team(int side) { return side == 1 ? m_own : m_rival; }

            /// @brief Gives a side's team for reading.
            /// @param side 1 the player's side, else the rival's.
            /// @return The team.
            const ThrowTeam& team(int side) const { return side == 1 ? m_own : m_rival; }

            /// @brief Draws a random number below a count.
            /// @param count Exclusive upper bound.
            /// @return Value from 0 to count - 1.
            int random(int count) { return std::uniform_int_distribution<int>(0, count - 1)(m_random); }

            /// @brief Gives the frame count of a person's action slot.
            /// @param side Side of the person.
            /// @param index Person slot.
            /// @param slot Action slot.
            /// @return Frame count.
            int frameCountOf(int side, int index, int slot) const;

            /// @brief Starts a hit or landing effect on a ball.
            /// @param projectile Ball to carry the effect.
            /// @param picture Name of the effect picture.
            /// @param frames Frame count of the effect.
            /// @param at Where the effect plays.
            void startLight(ThrowProjectile& projectile, const char* picture, int frames, core::Point at);

            Hooks m_hooks;
            std::mt19937 m_random{std::random_device{}()};
            std::uint64_t m_lastFrameMs = 0;
            std::uint64_t m_nowMs = 0;
            ThrowTeam m_own;
            ThrowTeam m_rival;
            std::array<ThrowProjectile, kProjectiles + 1> m_projectiles;
            std::array<int, kDrawItems + 1> m_drawOrder{};
            int m_fighterSprite = 0;
            int m_playerSlot = 1;
            bool m_creatingTeam = false;
            bool m_begun = false;
            bool m_musicStarted = false;
            bool m_playing = true;
            int m_gameOver = 0;
            std::uint64_t m_gameOverMs = 0;
            bool m_finished = false;
            bool m_mouseHeld = false;
            bool m_mouseReleased = false;
            bool m_throwing = false;
            int m_power = 0;
            core::Point m_ownDest;
            int m_ownDistX = 0;
            int m_ownDistY = 0;
            std::uint64_t m_chargeMs = 0;
            std::uint64_t m_throwMs = 0;
            std::uint64_t m_moveTickX = 0;
            std::uint64_t m_moveTickY = 0;
            int m_aimIndex = 0;
            int m_projectileCount = 0;
    };

}

#endif
