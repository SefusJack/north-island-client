#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_GROUP_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SOCIAL_GROUP_WINDOW_H

#include "client/services/GameServices.h"
#include "client/rendering/painters/PetInfoPainter.h"
#include "client/models/pets/PetTeam.h"
#include "client/models/social/TeamMate.h"
#include "client/models/social/Teams.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/viewmodels/chat/TalkDialog.h"
#include "client/viewmodels/social/PartyLoginWindow.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The group window (original GroupWindow.xml, TSe_TeamForm) with its party tab.
    /// @details Shows the pets in the order they joined (figure, level, HP and SP, name, element, what each does, Exile) and the pet command form that sets what one does (CH_PetStatusForm: rest, fight, ride). A pet with tradable 1 is renamed by clicking its name; Enter or a click elsewhere sends the new name. Under them are the teammates (DrawTeamPage) with Leave / Disband and, for the leader, sending a member away or making it the leader. The qol client's multi-login adds Log in (PartyLoginWindow) and, on a party member's row, Log out in Drive away's place.
    class GroupWindow {
        public:
            /// @brief A refusal of the leader's offer to lead and how long it shows.
            struct LeaderRefusal {
                std::string text;
                int ms = 0;
            };

            /// @brief Who the own player is and what is known of another player.
            struct Players {
                std::function<std::int32_t()> ownId;
                std::function<std::optional<TeamMate>(std::int32_t)> mate;
                std::function<bool()> fighting;
                std::function<bool()> keepsStall;
                std::function<std::optional<LeaderRefusal>(bool pending)> leaderRefusal;
                std::function<bool()> usesVehicle;
            };

            /// @brief The guild tab, a window of its own that takes this one's place.
            struct GuildTab {
                std::function<bool()> exists;
                std::function<bool()> isOpen;
                std::function<core::Point()> position;
                std::function<void(core::Point)> open;
                std::function<void()> close;
                std::function<void()> shown;
            };

            /// @brief The instance tab, a window of its own like the guild tab.
            struct InstanceTab {
                std::function<bool()> isOpen;
                std::function<core::Point()> position;
                std::function<void(core::Point)> open;
                std::function<void()> close;
            };

            /// @brief Builds the window and its forms.
            /// @param services Shared game services.
            /// @param screen Screen the windows are added to.
            /// @param pets The player's pet team.
            /// @param teams The teams known to the client.
            /// @param players Accessors for the own player and other players.
            /// @param messages Sink for player messages.
            /// @param talk Dialog used for questions.
            /// @param guildTab Accessors of the guild tab.
            /// @param instanceTab Accessors of the instance tab.
            GroupWindow(
                GameServices& services,
                ui::Screen& screen,
                PetTeam& pets,
                const Teams& teams,
                Players players,
                PlayerMessages& messages,
                TalkDialog& talk,
                GuildTab guildTab,
                InstanceTab instanceTab
            );

            /// @brief Tells whether the group window or one of its tabs is open.
            /// @return True while any of the three is visible.
            bool isOpen() const { return m_window.isVisible() || m_guildTab.isOpen() || m_instanceTab.isOpen(); }

            /// @brief Opens the window on the tab it was left on.
            /// @details Original ShowCtrl.
            void open();

            /// @brief Closes the window and its tabs.
            void close();

            /// @brief Opens the window when closed, closes it when open.
            void toggle();

            /// @brief Refreshes after the pets or the team changed.
            /// @details Original RefreshTeamPage.
            void refresh();

            /// @brief Forgets the pending offer to lead.
            /// @details Original FNewLeaderId := 0: the offer was answered or the team broke up.
            void leaderOfferEnded() { m_newLeaderId = 0; }

            /// @brief Forgets the pending offer to lead when the player who left is the one who left or the member offered.
            /// @param playerId Id of the player who left.
            /// @param own True when the own player left.
            void playerLeft(std::int32_t playerId, bool own) {
                if (own || playerId == m_newLeaderId) {
                    m_newLeaderId = 0;
                }
            }

            /// @brief Shows the party tab at a place, from the guild or instance tab.
            /// @details Original ShowTeamTab.
            /// @param at Window position.
            void showParty(core::Point at);

            /// @brief Shows the guild tab at a place, from the instance tab.
            /// @details Original ShowArmyGroupTab.
            /// @param at Window position.
            void showGuildAt(core::Point at);

            /// @brief Shows the instance tab at a place (where the window or the guild tab was).
            /// @details Original OpenDupMisForm.
            /// @param at Window position.
            void showInstances(core::Point at);

            /// @brief Handles the guild being gone: the party tab takes the guild tab's place.
            void guildGone();

        private:
            /// @brief One pet row of the form.
            struct PetRow {
                std::reference_wrapper<ui::TextBox> name;
                std::reference_wrapper<ui::Element> nameBack;
                std::reference_wrapper<ui::DrawingArea> element;
                std::reference_wrapper<ui::ImageButton> assign;
                std::reference_wrapper<ui::ImageButton> exile;
            };

            /// @brief One teammate row of the form.
            struct MateRow {
                std::reference_wrapper<ui::ImageButton> kick;
                std::reference_wrapper<ui::ImageButton> lead;
                std::reference_wrapper<ui::DrawingArea> job;
                std::reference_wrapper<ui::DrawingArea> element;
                std::reference_wrapper<ui::ImageButton> logout;
            };

            /// @brief Switches to the guild tab: this window gives way to the guild window.
            void showGuild();

            /// @brief Opens the pet command form above a row's button.
            /// @details Original PetAssignButtonClick.
            /// @param order Pet order (1 to 4).
            void openCommands(int order);

            /// @brief Closes the pet command form.
            void closeCommands();

            /// @brief Applies a choice of the pet command form.
            /// @details Original AssignPetStatus / ChangePetAssignState: 1 rest, 2 fight, 3 ride.
            /// @param choice The chosen command (1 rest, 2 fight, 3 ride).
            void command(int choice);

            /// @brief Rides the pet, or gets off it when it is the one ridden.
            /// @details Original ToggleFightPet.
            /// @param place Pet place in the team.
            void ride(int place);

            /// @brief Gets off the pet being ridden.
            /// @param place Pet place in the team.
            void dismount(int place);

            /// @brief Asks whether to exile a pet.
            /// @details Original ConfirmExilePet.
            /// @param order Pet order (1 to 4).
            void askExile(int order);

            /// @brief Starts renaming a pet in its name box.
            /// @param order Pet order (1 to 4).
            void beginRename(int order);

            /// @brief Ends the renaming and sends the new name.
            void commitRename();

            /// @brief Sets the picture of a pet's assign button.
            /// @param order Pet order (1 to 4).
            /// @param state Assign state (0 rest, 1 fight, 2 ride).
            void setAssignPicture(int order, int state);

            /// @brief Refreshes the teammate rows.
            void refreshTeam();

            /// @brief Asks whether to leave or disband the team.
            /// @details Original ConfirmLeaveTeam.
            void askLeave();

            /// @brief Asks whether to send a member away.
            /// @details Original AskKickTeamMember.
            /// @param slot Teammate slot (1 to 3 of FMemberIds).
            void askKick(int slot);

            /// @brief Asks whether to make a member the leader.
            /// @details Original AskChangeTeamLeader.
            /// @param slot Teammate slot (1 to 3 of FMemberIds).
            void askLead(int slot);

            /// @brief Asks whether that party member leaves the team and logs out (qol multi-login).
            /// @param slot Teammate slot (1 to 3).
            void askLogout(int slot);

            /// @brief Gives the id of a shown teammate.
            /// @details Original GetTeamMemberId.
            /// @param index Position among the shown teammates (1 to 3).
            /// @return The player id, 0 when there is none.
            std::int32_t shownMate(int index) const;

            /// @brief Checks whether the leader may offer the lead now.
            /// @details Original SendChangeTeamLeader's checks.
            /// @return True when allowed, false after telling why not.
            bool mayOfferLead();

            /// @brief Paints the teammates part.
            /// @param canvas Target canvas.
            /// @param at Top left corner of the window.
            void paintTeam(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the pet rows.
            /// @param canvas Target canvas.
            /// @param at Top left corner of the window.
            /// @param nowMs Current time in milliseconds.
            void paintPets(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the hints (tooltips) under the mouse.
            /// @param canvas Target canvas.
            /// @param at Top left corner of the window.
            void paintHints(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            PetTeam& m_pets;
            const Teams& m_teams;
            Players m_players;
            std::int32_t m_newLeaderId = 0;
            PlayerMessages& m_messages;
            TalkDialog& m_talk;
            GuildTab m_guildTab;
            InstanceTab m_instanceTab;
            ui::Window& m_window;
            ui::Window& m_commands;
            ui::ImageButton& m_leave;
            ui::ImageButton& m_guildButton;
            ui::ImageButton& m_instancesButton;
            ui::ImageButton& m_partyLoginButton;
            PartyLoginWindow m_partyLogin;
            const ui::Spot& m_instancesAloneSpot;
            const ui::Spot& m_instancesWithGuildSpot;
            const ui::Spot& m_petPartsSpot;
            const ui::Spot& m_commandSpot;
            const ui::Spot& m_mateRowSpot;
            const ui::Spot& m_matePortraitSpot;
            const ui::Spot& m_nameColorSpot;
            const ui::Spot& m_fixedNameColorSpot;
            const ui::Spot& m_editingColorSpot;
            bool m_onGuildTab = false;
            bool m_onInstanceTab = false;
            std::vector<PetRow> m_rows;
            std::vector<MateRow> m_mateRows;
            std::vector<PetInfoPainter> m_petInfos;
            std::array<int, PetTeam::kPlaces + 1> m_assignStates{};
            std::array<std::int32_t, 4> m_memberIds{};
            int m_commandOrder = 0;
            int m_renameOrder = 0;
            std::string m_nameBeforeRename;
    };

}

#endif
