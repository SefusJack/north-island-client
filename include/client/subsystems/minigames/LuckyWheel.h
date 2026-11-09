#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_LUCKY_WHEEL_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_MINIGAMES_LUCKY_WHEEL_H

#include "client/subsystems/minigames/GameHooks.h"
#include "core/Bytes.h"
#include "core/Point.h"
#include "formats/ItemTable.h"

#include <array>
#include <cstdint>
#include <functional>
#include <random>
#include <string>
#include <vector>

namespace client {

    /// @brief The Lucky Wheel: twenty prizes round a wheel, payment, Refresh, Double and Start, and a spin that stops on the server's prize.
    /// @details Original: CJ_MiniGame.pas TTurnGameForm without its drawing; the form sits at FormLeft, FormTop; payment by IM points, Carnie or Fun Tokens.
    class LuckyWheel {
        public:
            static constexpr int kPrizes = 20; // prize cells round the wheel
            static constexpr int kPanelSlots = 6; // top-prize panel slots, 1-based
            static constexpr int kPointsPerSpin = 65; // IM Points one spin costs
            static constexpr int kTokensPerSpin = 13; // tokens one spin costs

            /// @brief Callbacks the wheel uses to reach the host.
            struct Hooks : GameHooks {
                std::function<void(int slot, core::Point prizePosition, int tier)> frameEffect;
            };

            /// @brief One prize of the wheel.
            /// @details Original: FPrizes.
            struct Prize {
                int itemId = 0;
                int count = 0;
                std::string name;
                core::Point position;
            };

            /// @brief A prize shown on the panel, with the frame that runs round it on the wheel.
            /// @details Original: FPrizeSlots, FTopPrizes.
            struct TopPrize {
                int prizeIndex = 0;
                int itemId = 0;
                int count = 0;
                int tier = 0;
                std::string text;
            };

            /// @brief The light of a win.
            /// @details Original: TLight of S40003.
            struct WinLight {
                core::Point position;
                int frame = 0;
                std::uint64_t frameMs = 0;
                bool finished = false;
            };

            /// @brief Creates the wheel.
            /// @param items Item table, for prize names.
            /// @param hooks Callbacks to the host.
            LuckyWheel(
                const formats::ItemTable& items,
                Hooks hooks
            );

            /// @brief Sets where the form's picture goes (the window tells, as the picture's size is not known here).
            /// @param origin Top left of the form.
            void setFormOrigin(core::Point origin);

            /// @brief Opens the form with its buttons away and no prizes known.
            /// @param nowMs Current time in ms.
            /// @details Original: StartTurnGame.
            void open(std::uint64_t nowMs);

            /// @brief Shows the buttons and asks for the points (Begin on the instructions).
            /// @param nowMs Current time in ms.
            /// @details Original: ShowCtrl.
            void begin(std::uint64_t nowMs);

            /// @brief Handles the prize list packet.
            /// @param payload Packet body.
            /// @param nowMs Current time in ms.
            /// @details Original: 57/12 (RecvWheelPrizeList).
            void receiveList(const core::Bytes& payload, std::uint64_t nowMs);

            /// @brief Handles a play error packet.
            /// @param payload Packet body.
            /// @details Original: 71/21 sub 2 (RecvWheelPlayError).
            void receiveError(const core::Bytes& payload);

            /// @brief Handles the spin result packet.
            /// @param payload Packet body.
            /// @param nowMs Current time in ms.
            /// @details Original: 71/21 sub 3 (RecvWheelResult).
            void receiveResult(const core::Bytes& payload, std::uint64_t nowMs);

            /// @brief Whether the Refresh button shows.
            /// @return True when shown.
            bool refreshShown() const { return m_refreshShown; }

            /// @brief Whether the Double button shows.
            /// @return True when shown.
            bool doubleShown() const { return m_doubleShown; }

            /// @brief Whether the Start button shows.
            /// @return True when shown.
            bool startShown() const { return m_startShown; }

            /// @brief Whether the Exit button shows.
            /// @return True when shown.
            bool exitShown() const { return m_exitShown; }

            /// @brief Handles the Refresh button (another group of rewards).
            void refreshClicked();

            /// @brief Handles the Double button.
            void doubleClicked();

            /// @brief Handles the Start button.
            void startClicked();

            /// @brief Handles the Exit button.
            void exitClicked();

            /// @brief Runs the spin's clock work; the drawing asks between this and the blink.
            /// @param nowMs Current time in ms.
            /// @details Original: AnimateWheelResult's clock work.
            void spinStep(std::uint64_t nowMs);

            /// @brief Runs the blink's clock work.
            /// @param nowMs Current time in ms.
            void blinkStep(std::uint64_t nowMs);

            /// @brief Decides which win lights show.
            /// @param nowMs Current time in ms.
            /// @details Original: UpdateWheelEffects' clock work.
            void lightsStep(std::uint64_t nowMs);

            /// @brief After a light is drawn, moves it to its next frame when one is due and plays the sound of its second frame.
            /// @param slot Light number.
            /// @param nowMs Current time in ms.
            /// @details Original: TLight.DrawLight / IsFrameDue.
            void advanceLight(int slot, std::uint64_t nowMs);

            /// @brief Ends the lights once all three are finished.
            void endLightsWhenFinished();

            /// @brief The wheel state.
            /// @return 0 waiting for the list, 1 resting, 2 spinning.
            int state() const { return m_state; }

            /// @brief The prize the spin is on.
            /// @return The position.
            int spinPosition() const { return m_spinPos; }

            /// @brief The blink phase.
            /// @return The phase.
            int blinkPhase() const { return m_blinkPhase; }

            /// @brief The highlighted prize row.
            /// @return The row.
            int highlightRow() const { return m_highlightRow; }

            /// @brief Whether a double is pending.
            /// @return True when pending.
            bool doublePending() const { return m_doublePending; }

            /// @brief Whether the win lights are active.
            /// @return True when active.
            bool lightsActive() const { return m_lightsActive; }

            /// @brief Which lights show.
            /// @return A bit mask.
            int lightMask() const { return m_lightMask; }

            /// @brief The win lights.
            /// @return The three lights.
            const std::array<WinLight, 3>& lights() const { return m_lights; }

            /// @brief The win lights, changeable.
            /// @return The three lights.
            std::array<WinLight, 3>& lights() { return m_lights; }

            /// @brief The wheel's prizes (from 1).
            /// @return The prizes.
            const std::array<Prize, kPrizes + 1>& prizes() const { return m_prizes; }

            /// @brief The prizes on the panel (from 1).
            /// @return The panel prizes.
            const std::array<TopPrize, kPanelSlots + 1>& panel() const { return m_panel; }

            static constexpr int kLightFrames = 7; // frames of the light animation
            static constexpr int kLightIntervalMs = 90; // milliseconds per light frame

        private:
            /// @brief Resets the game.
            /// @param nowMs Current time in ms.
            /// @details Original: ResetTurnGame.
            void reset(std::uint64_t nowMs);

            /// @brief Reads a prize list packet into the prizes and the panel.
            /// @param payload Packet body.
            /// @param explicitItems True when the packet names the items themselves, false when it sends codes the tables resolve.
            /// @param nowMs Current time in ms.
            /// @details Original: SetupWheelPrizes (codes) and SetupWheelPrizeItems (items).
            void setup(const core::Bytes& payload, bool explicitItems, std::uint64_t nowMs);

            /// @brief Asks which way to pay.
            /// @details Original: ChooseWheelPayment.
            void choosePayment();

            /// @brief Asks to confirm the chosen payment.
            /// @param kind The chosen way.
            /// @details Original: AskPlayWheel.
            void askToPay(int kind);

            /// @brief Pays and plays.
            /// @param kind 1 points, 2 Carnie Tokens, 3 Fun Tokens.
            /// @details Original: PlayWheelWithPoints / PlayWheelWithCarnieToken / PlayWheelWithFunToken.
            void pay(int kind);

            /// @brief Whether a spin can be played now.
            /// @return True when it can.
            /// @details Original: CanPlayWheel.
            bool canPlay();

            /// @brief Asks to confirm a double.
            /// @details Original: ConfirmDoubleWheel.
            void confirmDouble();

            /// @brief Whether the bag has an empty place.
            /// @return True when it has.
            bool hasEmptyPlace() const;

            /// @brief Starts a win light.
            /// @param slot Light number.
            /// @param position Where it shows.
            /// @param nowMs Current time in ms.
            void startLight(int slot, core::Point position, std::uint64_t nowMs);

            const formats::ItemTable& m_items;
            Hooks m_hooks;
            std::array<Prize, kPrizes + 1> m_prizes{};
            std::array<TopPrize, kPanelSlots + 1> m_panel{};
            std::array<WinLight, 3> m_lights{};
            int m_state = 0;
            int m_spinPos = 1;
            int m_spinStepsLeft = 0;
            int m_spinAccelSteps = 0;
            int m_spinIntervalMs = 500; // milliseconds between spin steps (start value, slower at first)
            int m_blinkIntervalMs = 300; // milliseconds between blinks
            int m_blinkPhase = 0;
            std::uint64_t m_spinMs = 0;
            std::uint64_t m_blinkMs = 0;
            int m_payKind = 0;
            int m_payMode = 0;
            bool m_paid = false;
            bool m_doublePending = false;
            bool m_doubleAsked = false;
            int m_resultAnimStep = 1;
            int m_highlightRow = 0;
            int m_resultRow = 0;
            std::string m_wonName;
            int m_wonCount = 0;
            std::mt19937 m_random{std::random_device{}()};
            std::string m_winSound;
            bool m_lightsPending = false;
            bool m_lightsActive = false;
            int m_lightMask = 0;
            int m_lightIntervalMs = 0;
            std::uint64_t m_lightMs = 0;
            bool m_refreshShown = true;
            bool m_doubleShown = true;
            bool m_startShown = true;
            bool m_exitShown = true;
    };

}

#endif
