#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_BATTLE_BATTLE_MAIN_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_BATTLE_BATTLE_MAIN_FORM_H

#include "client/services/GameServices.h"
#include "client/subsystems/battle/Battle.h"
#include "client/rendering/hud/Gauge.h"
#include "client/rendering/painters/PetInfoPainter.h"
#include "client/models/character/PlayerStatus.h"
#include "core/Point.h"
#include "ui/DigitText.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/ImageView.h"
#include "ui/Label.h"
#include "ui/ProgressBar.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <array>
#include <cstdint>
#include <functional>

namespace client {

    /// @brief Battle panel and command ring shown while the own player fights.
    /// @details Original: BattleMainForm.xml, TRE_BattleMainForm; the panel slides in (AnimateBattlePanelSlide) and the ring flies out (AnimateCommandWheel).
    class BattleMainForm {
        public:
            /// @brief Builds the panel and wires its member painters and command buttons.
            /// @param services Game services.
            /// @param screen Screen the panel lives on.
            /// @param battle The running battle.
            /// @param status Own player status.
            BattleMainForm(
                GameServices& services,
                ui::Screen& screen,
                Battle& battle,
                const PlayerStatus& status
            );

            /// @brief Sets the remote-control hook that presses the ring's auto button for the commander.
            /// @details Original: RenderWithChildren, HandleBattleCommandClick 5.
            /// @param autoFight Returns true when the fight should be played automatically.
            void setAutoFight(std::function<bool()> autoFight) { m_autoFight = std::move(autoFight); }

            /// @brief Sets the mapping from a battle field point to a window point.
            /// @param toWindow The mapping function.
            void setToWindow(std::function<core::Point(core::Point)> toWindow) { m_toWindow = std::move(toWindow); }

            /// @brief Tells the ring whether a game pad is played with: it then turns, and the button at its top is the chosen one.
            /// @param on True in controller mode; the ring turns back to its own places otherwise.
            /// @param hand The hand that points at the chosen button.
            void setPad(bool on, const ui::PadPointer& hand) {
                m_padOn = on;
                m_padHand = hand;
            }

            /// @brief Whether the ring is up and turned by the pad now.
            /// @return True while the pad's left and right turn it.
            bool padTurns() const { return m_padOn && m_battle.choosing(); }

            /// @brief Turns the ring by one button.
            /// @param right True to bring the button to the right of the top one up, false the one to its left.
            void turn(bool right) { m_top = (m_top + (right ? 1 : kButtons - 1)) % kButtons; }

            /// @brief Takes the button at the ring's top, as a click on it would.
            void takeTop() { m_battle.command(m_top + 1); }

            /// @brief Starts the panel coming down from above the screen.
            /// @details Original: SetBattlePanelSlide 1.
            /// @param nowMs Current time in milliseconds.
            void slideIn(std::uint64_t nowMs);

            /// @brief Hides the panel.
            /// @details Original: HideCtrl.
            void hide();

            /// @brief Animates the panel and the ring and refreshes the member panels.
            /// @param nowMs Current time in milliseconds.
            void update(std::uint64_t nowMs);

        private:
            /// @brief The controls and painters of one member panel.
            struct Member {
                std::reference_wrapper<ui::Element> panel;
                std::reference_wrapper<ui::Label> name;
                std::reference_wrapper<ui::ImageView> element;
                std::reference_wrapper<ui::DigitText> level;
                std::reference_wrapper<ui::ImageView> job;
                Gauge hp;
                Gauge sp;
                PetInfoPainter pet;
            };

            /// @brief Shows a panel for each fighter on the own side and fills the own HP, SP, experience and gold.
            /// @details Original: RenderWithChildren / DrawBattleMembers.
            void showMembers();

            /// @brief Places the ring's buttons on their way out of the commander or at their places.
            /// @details Original: AnimateCommandWheel.
            /// @param nowMs Current time in milliseconds.
            void placeCommands(std::uint64_t nowMs);

            /// @brief Paints the HP gauge, SP gauge or portrait of a member's fighter.
            /// @param canvas Target canvas.
            /// @param member Member index.
            /// @param part 0 HP, 1 SP, 2 portrait.
            /// @param at Origin of the drawing area.
            void paintMember(gfx::Canvas& canvas, int member, int part, core::Point at);

            /// @brief Paints the member's pet fighter (the kind-4 fighter it owns).
            /// @details Original: TRe_MiniPetInfo in battle.
            /// @param canvas Target canvas.
            /// @param member Member index.
            /// @param at Origin of the drawing area.
            /// @param nowMs Current time in milliseconds.
            void paintMemberPet(gfx::Canvas& canvas, int member, core::Point at, std::uint64_t nowMs);

            /// @brief Paints a fighter's status icons from a point, closer together when there are more than two (pet: at most four).
            /// @details Original: DrawCharStatusIcons / DrawPetStatusIcons.
            /// @param canvas Target canvas.
            /// @param fighter The fighter.
            /// @param at First icon position.
            /// @param pet True for a pet fighter.
            void paintStatusIcons(gfx::Canvas& canvas, const Fighter& fighter, core::Point at, bool pet);

            static constexpr int kButtons = 7;
            static constexpr int kMembers = 4;

            GameServices& m_services;
            Battle& m_battle;
            const PlayerStatus& m_status;
            ui::Element& m_panel;
            ui::Element& m_ownInfo;
            ui::DigitText& m_hp;
            ui::DigitText& m_sp;
            ui::ProgressBar& m_expBar;
            ui::DigitText& m_expPercent;
            ui::DigitText& m_gold;
            std::array<std::reference_wrapper<ui::ImageButton>, kButtons> m_commands;
            ui::ImageButton& m_lastCommand;
            ui::ImageButton& m_autoCancel;
            const ui::Spot& m_memberPanelSpot;
            const ui::Spot& m_wheelCentreSpot;
            std::vector<Member> m_members;
            int m_slideY = 0;
            std::uint64_t m_slideMs = 0;
            std::function<bool()> m_autoFight;
            std::function<core::Point(core::Point)> m_toWindow;
            bool m_slid = false;
            int m_wheelActor = 0;
            std::uint64_t m_wheelMs = 0;
            ui::Screen& m_screen;
            bool m_padOn = false; // a game pad is played with: the ring turns
            ui::PadPointer m_padHand; // the hand at the ring's top button
            static constexpr int kPadFirst = 6; // the button a turn starts with at the top for the pad, counted from 0: the skills
            int m_top = kPadFirst; // the button at the ring's top for the pad, counted from 0
            float m_turn = 0.0f; // how far the ring is turned now, in buttons (it follows m_top, or goes back to 0 without a pad)
            std::uint64_t m_turnMs = 0; // when the ring was last placed
            bool m_padFocused = false; // the screen's pad highlight is on a ring button
    };

}

#endif
