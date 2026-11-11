#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_BATTLE_BATTLE_H

#include "client/services/GameServices.h"
#include "client/subsystems/battle/ActionContext.h"
#include "client/subsystems/battle/BattleCamera.h"
#include "client/subsystems/battle/BattleField.h"
#include "client/subsystems/battle/BattleGround.h"
#include "client/subsystems/battle/BattleTween.h"
#include "client/rendering/effects/LightEffect.h"
#include "client/models/remote/AutoBattleSettings.h"
#include "client/models/remote/RemoteBattleView.h"
#include "client/subsystems/battle/BattlePainter.h"
#include "client/subsystems/battle/SkillAction.h"
#include "client/subsystems/battle/SkillUser.h"
#include "client/subsystems/battle/SkillZoom.h"
#include "client/models/character/CharacterLook.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "core/Color.h"
#include "formats/NpcInfo.h"
#include "formats/SceneTable.h"
#include "protocol/BattleAction.h"
#include "protocol/BattleStart.h"
#include "protocol/FighterCast.h"
#include "protocol/FighterWeaponLook.h"
#include "ui/CursorKind.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace client {

    /// @brief A battle the own player is in: fighters, command rounds against the clock, and the server's actions played out.
    /// @details Original: TFightField with TFightManage.
    class Battle {
        public:
            /// @brief Callbacks into the rest of the client that the battle needs.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<std::optional<std::pair<CharacterLook, std::string>>(std::int32_t)> player;
                std::function<std::optional<formats::NpcInfo>(int)> mapNpc;
                std::function<std::optional<std::string>(std::int32_t, int)> petName;
                std::function<void(const std::string&)> chat;
                std::function<SkillUser(const Fighter&)> skillUser;
                std::function<void()> toggleSkills;
                std::function<void()> toggleItems;
                std::function<void(int skillId, bool pet)> skillCommanded;
                std::function<int()> mapId;
                std::function<std::array<int, 3>(bool pet)> recentSkills;
                std::function<int(int itemId)> bagSlotOf;
                std::function<void()> hideItems;
                std::function<std::optional<std::pair<int, std::int32_t>>(std::int32_t, int)> petWeapon;
                std::function<bool()> remoteOn;
                std::function<AutoBattleSettings(int kind)> playSets;
                std::function<int()> activePetId;
                std::function<bool()> skillZoomOn; // the system window's camera switch is on (BattleSwitch 0)
                std::function<std::optional<RemoteDecision>(const RemoteBattleView&)> scriptDecision; // the Electron Control's script
                std::function<void(const RemoteDecision&, int actorKind, bool used)> scriptOutcome; // whether the script's choice became the command
                std::function<bool()> scriptOn; // the Electron Control runs with a script loaded
            };

            /// @brief Creates an idle battle.
            /// @param services Shared game services.
            /// @param messages Player message store.
            /// @param hooks Callbacks into the rest of the client.
            Battle(
                GameServices& services,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Whether a battle is running.
            /// @details Original: FightField <> nil.
            /// @return True while the battle is on.
            bool isOn() const { return m_on; }

            /// @brief Starts the battle: the field, its ground and fighters.
            /// @details Original: RevFightStart.
            /// @param start The battle-start packet.
            /// @param ownId The own player's id.
            /// @param scenes Scene table for the battle ground.
            /// @param tint Colour tint of the ground.
            void start(const protocol::BattleStart& start, std::int32_t ownId, const formats::SceneTable& scenes, core::Color tint);

            /// @brief Sets the battle type, which decides the command time.
            /// @details Original: RevBattleType; types 2, 4, 7, 8 and 9 give 30 seconds, the others 20.
            /// @param type Battle type id.
            void setType(int type);

            /// @brief The battle type.
            /// @return Battle type id.
            int type() const { return m_type; }

            /// @brief Starts the battle's 3-minute time limit.
            /// @details Original: packet 81/3.
            /// @param nowMs Current time in ms.
            void startTimeLimit(std::uint64_t nowMs);

            /// @brief Resets the time limit: "Battle time is reset." and the clock goes off.
            /// @details Original: RevBattleTimeReset, packet 51/2.
            /// @param nowMs Current time in ms.
            void resetTimeLimit(std::uint64_t nowMs);

            /// @brief Uses the Delay Clock item (34166): sends 23/107 and an empty item command.
            /// @details Original: UseGridItem for item 34166.
            /// @param bagSlot Bag slot holding the clock.
            /// @return False when the battle's type or state refuses it.
            bool useDelayClock(int bagSlot);

            /// @brief Starts a command round.
            /// @details Original: RevCommandRoundStart.
            /// @param nowMs Current time in ms.
            void startRound(std::uint64_t nowMs);

            /// @brief Ends the running actions: fighters reset for the round and the zoom goes.
            /// @details Original: TFightForm1.ConfirmUseWinPlate, ResetFightersForRound.
            /// @param nowMs Current time in ms.
            void stopRunningActions(std::uint64_t nowMs);

            /// @brief Puts the fighters back at their cells for the item phase.
            /// @details Original: RevAttackItem (50/2).
            /// @param nowMs Current time in ms.
            void playItems(std::uint64_t nowMs);

            /// @brief Applies a status-effect 241/242 cast to its target once the camera's caster is back.
            /// @details Original: RecvFightSkillCast (50/3); the target takes the attribute change and shows it.
            /// @param cast The cast record.
            void castAttribute(const protocol::FighterCast& cast);

            /// @brief Plays the mark sound for each fighter listed.
            /// @details Original: RecvFighterMarks (50/5).
            /// @param count Number of fighters listed.
            void marks(int count);

            /// @brief Sets a damage value on a fighter.
            /// @details Original: RecvFighterDamage (50/4).
            /// @param kind Damage kind.
            /// @param row Fighter row.
            /// @param col Fighter column.
            /// @param damage Damage amount.
            void setFighterDamage(int kind, int row, int col, std::int32_t damage);

            /// @brief Sets the attack area of a fighter.
            /// @details Original: RecvFighterAttackArea (50/6).
            /// @param row Fighter row.
            /// @param col Fighter column.
            /// @param area Attack area id.
            void setAttackArea(int row, int col, int area);

            /// @brief Puts the fighters back at their cells, then plays each action (the camera follows the first).
            /// @details Original: RevAttackSkill.
            /// @param actions The actions to play.
            /// @param nowMs Current time in ms.
            void play(const std::vector<protocol::BattleAction>& actions, std::uint64_t nowMs);

            /// @brief Sets an attribute of a fighter.
            /// @details Original: RevFightHumAttrs.
            /// @param row Fighter row.
            /// @param col Fighter column.
            /// @param attribute Attribute id.
            /// @param value New value.
            void setAttribute(int row, int col, int attribute, std::int32_t value);

            /// @brief A fighter runs off the field and leaves, or leaves at once (flying off, field full, or a player with an owner).
            /// @details Original: RevFightHumLeave.
            /// @param row Fighter row.
            /// @param col Fighter column.
            /// @param nowMs Current time in ms.
            void leave(int row, int col, std::uint64_t nowMs);

            /// @brief Fighters enter the running battle and come onto their cells.
            /// @details Original: SupportRoleInfo (11/5).
            /// @param records The entering fighters.
            /// @param placeOthers One watching or waiting sees the others put straight on their cells.
            void join(const std::vector<protocol::FighterRecord>& records, bool placeOthers);

            /// @brief Sets the weapons the pets of the battle hold.
            /// @details Original: RecvFighterEquipLook (11/251).
            /// @param looks Weapon looks per fighter.
            void setWeaponLooks(const std::vector<protocol::FighterWeaponLook>& looks);

            /// @brief Another player runs off the field.
            /// @details Original: RevLeaveFight (11/0).
            /// @param playerId Id of the leaving player.
            /// @param nowMs Current time in ms.
            void playerLeft(std::int32_t playerId, std::uint64_t nowMs);

            /// @brief Sets a status icon on a fighter.
            /// @details Original: RevFightHumStatus (53/1).
            /// @param row Fighter row.
            /// @param col Fighter column.
            /// @param kind Status kind.
            /// @param skillId Skill that caused the status.
            /// @param place Icon place.
            void setStatus(
                int row,
                int col,
                int kind,
                int skillId,
                int place
            );

            /// @brief The fighter at a cell will throw an item to each of the target cells.
            /// @details Original: RevThrowItem (53/4).
            /// @param itemId Item thrown.
            /// @param row Thrower row.
            /// @param col Thrower column.
            /// @param targets Target cells as (row, col).
            void throwItem(int itemId, int row, int col, const std::vector<std::pair<int, int>>& targets);

            /// @brief Moves the fighter at a cell onto another cell at once (a leaving fighter there is gone).
            /// @details Original: RevMoveFighterCell (53/7).
            /// @param fromRow Source row.
            /// @param fromCol Source column.
            /// @param toRow Target row.
            /// @param toCol Target column.
            void moveFighter(int fromRow, int fromCol, int toRow, int toCol);

            /// @brief Knocks a fighter off the field, its ghost rising 300 px before it leaves.
            /// @details Original: RevFightHumDie.
            /// @param row Fighter row.
            /// @param col Fighter column.
            /// @param nowMs Current time in ms.
            void knockOut(int row, int col, std::uint64_t nowMs);

            /// @brief Whether the own player has left the field and the battle is closing.
            /// @details Original: FMapChangePending 3.
            /// @return True when the own player is gone.
            bool ownGone() const { return m_ownGone; }

            /// @brief Takes the own-gone flag: true once, then the battle is over.
            /// @details Original: FighterLeaveBattle of the own player after flying off.
            /// @return True the first time after the own player left.
            bool takeOwnGone() {
                const bool gone = m_ownGone;
                m_ownGone = false;
                return gone;
            }

            /// @brief Marks a fighter as having acted.
            /// @details Original: RevFightHumActed.
            /// @param row Fighter row.
            /// @param col Fighter column.
            void acted(int row, int col);

            /// @brief Closes the battle field.
            /// @details Original: CloseFightField.
            void end();

            /// @brief Advances the battle by one tick.
            /// @param nowMs Current time in ms.
            void update(std::uint64_t nowMs);

            /// @brief Paints the battle ground.
            /// @details Original: DrawBattleGround.
            /// @param canvas Target canvas.
            void paintGround(gfx::Canvas& canvas) const;

            /// @brief Paints the actions' layers behind and in front of the fighters, the numbers, the clock and the HP bars.
            /// @details Original: DrawFightField.
            /// @param canvas Target canvas.
            /// @param mouse Mouse position (the fighter under it is lit).
            /// @param nowMs Current time in ms.
            void paintField(gfx::Canvas& canvas, core::Point mouse, std::uint64_t nowMs);

            /// @brief Paints the fighter labels and the hover info.
            /// @details Original: DrawFightScene and DrawHoverFighterInfo. They are drawn after the skill zoom's stretch, so the caller
            /// draws them outside it; the names then move by the zoom's scale (paintNames).
            /// @param canvas Target canvas.
            /// @param mouse Mouse position.
            void paintLabels(gfx::Canvas& canvas, core::Point mouse);

            /// @brief Handles a left click in the fight.
            /// @details Original: HandleFightLeftClick.
            /// @param point Click position.
            void click(core::Point point);

            /// @brief Cancels the command in progress on a right click: chosen skill or item let go, plain cursor back, ring shown again.
            /// @details Original: DXDraw1MouseDown right button, ResetFighterCommand, ShowBattleCommandButtons 1.
            void cancelCommand();

            /// @brief Handles a click on a command ring button.
            /// @details Original: HandleBattleCommandClick; 1 defend, 2 capture, 3 flee, 4 items, 5 auto, 6 attack, 7 skills, 8 the last command again.
            /// @param button Ring button number.
            void command(int button);

            /// @brief Whether auto battle is on for the player or the pet.
            /// @details Original: FAutoCommands; goes on until cancelled or the battle is over (TFightField.Create clears it).
            /// @return True when auto is on.
            bool autoOn() const { return m_autoCommands[1] != 0 || m_autoCommands[2] != 0; }

            /// @brief Cancels auto battle.
            /// @details Original: ClearAutoBattleCommand.
            /// @param which 1 the player's, 2 the pet's, 3 both.
            void cancelAuto(int which);

            /// @brief The commanding fighter's last command kind (the eighth button's picture).
            /// @return Command kind, 0 for none.
            int lastCommandKind() const;

            /// @brief Takes a skill picked in the battle skill window (a skill on oneself goes at once, others wait for a target).
            /// @details Original: UseSelectedBattleSkill; skillForm is UseSkillFromList, which shows the magic cursor whatever the skill
            /// and asks whether the command's skill (not the listed one) goes on oneself.
            /// @param skillId Skill picked.
            /// @param skillForm True when picked from the skill window's list.
            /// @return True when the skill was taken.
            bool useListedSkill(int skillId, bool skillForm = false);

            /// @brief The commanding fighter as CheckCanUseSkill sees it.
            /// @return The skill user, or nothing when no own fighter commands.
            std::optional<SkillUser> actorAsUser() const;

            /// @brief Whether the commander (or between commands the own player) has the SP for a bar skill and may use it.
            /// @details Original: RefreshHotKeyAvailability.
            /// @param skillId Skill on the bar.
            /// @return True when usable.
            bool hotKeySkillUsable(int skillId) const;

            /// @brief Chooses a bag item for the commanding fighter to use on the target clicked next.
            /// @details Original: ChooseBattleItem.
            /// @param itemId Item id.
            /// @param bagSlot Bag slot of the item.
            /// @param count How many of the item lie there.
            void chooseItem(int itemId, int bagSlot, int count);

            /// @brief The battle field.
            /// @return The field.
            const BattleField& field() const { return m_field; }

            /// @brief The time the last tick's held sleeps reached.
            /// @return Held-until time in ms.
            std::uint64_t heldUntilMs() const { return m_heldUntilMs; }

            /// @brief Begins presenting: drawing shows the field between the last two ticks.
            /// @param nowMs Current time in ms.
            void beginPresent(std::uint64_t nowMs);

            /// @brief Ends presenting.
            void endPresent();

            /// @brief The part of the game screen the skill zoom stretches over it in this drawing.
            /// @details Original: FZoomRect while ApplySkillZoom has set FZoomLevel 1 (StretchBufToScreen). The ground, the fighters and the
            /// effects are stretched; the labels are not (paintLabels). With free frames it lies between the last two ticks' parts (the
            /// whole screen standing for none), as the field does.
            /// @return The part in game screen coordinates, or nothing when the screen is not zoomed.
            std::optional<core::Rect> skillZoomPart() const;

            /// @brief The own side's players shown in the battle panel, by slot.
            /// @details Original: AddBattleMember.
            /// @return Slots of the members.
            const std::vector<int>& members() const { return m_members; }

            /// @brief The skill chosen by the commanding fighter.
            /// @details Original: FCmdSkillId.
            /// @return Skill id, 0 when none chosen yet.
            int chosenSkill() const { return m_skill; }

            /// @brief The slot of the commanding own fighter.
            /// @details Original: FActorSlot.
            /// @return Slot, 0 when no own fighter commands.
            int actor() const { return m_actor; }

            /// @brief Whether the commanding fighter has yet to pick a command (the ring is up).
            /// @return True while choosing.
            bool choosing() const { return m_on && m_roundCountdown >= 0 && m_actor != 0 && m_skill == 0 && m_item == 0 && m_cursor == ui::CursorKind::Normal; }

            /// @brief The cursor over the field.
            /// @details Original: SetMouseCursorType.
            /// @return Cursor kind.
            ui::CursorKind cursor() const { return m_cursor; }

        private:
            /// @brief Builds the context the skill actions run in.
            /// @param nowMs Current time in ms.
            /// @return The action context.
            ActionContext actionContext(std::uint64_t nowMs);

            /// @brief Puts every fighter at its cell with its last action over (not a flight still under way).
            /// @details Original: ResetFightersForRound.
            /// @param context Action context.
            void resetForRound(ActionContext& context);

            /// @brief Frees a slot's action; a run-off ending takes its fighter off the field.
            /// @details Original: FreeFightAction, FinishEscapeSkill.
            /// @param slot Fighter slot.
            /// @param context Action context.
            void freeAction(int slot, ActionContext& context);

            /// @brief A fighter leaves: for the own player the battle is over, others are taken off the field.
            /// @details Original: FighterLeaveBattle.
            /// @param slot Fighter slot.
            void leaveField(int slot);

            /// @brief Paints the skill and hit effect layers behind or in front of the fighters.
            /// @details Original: DrawSkillEffectsBack / Front and DrawHitEffectsBack / Front.
            /// @param canvas Target canvas.
            /// @param front True for the front layer.
            /// @param nowMs Current time in ms.
            void paintLayers(gfx::Canvas& canvas, bool front, std::uint64_t nowMs);

            /// @brief Paints the damage numbers.
            /// @details Original: DrawFightDamageNumbers.
            /// @param canvas Target canvas.
            void paintNumbers(gfx::Canvas& canvas);

            /// @brief Builds a fighter from a server record.
            /// @param record Fighter record.
            /// @return The fighter, or nothing when the record cannot be built.
            std::optional<Fighter> makeFighter(const protocol::FighterRecord& record) const;

            /// @brief Finds the next own fighter after a slot that can command.
            /// @details Original: FindNextOwnFighter.
            /// @param afterSlot Slot to start after.
            /// @return Slot, 0 when none.
            int nextOwnFighter(int afterSlot) const;

            /// @brief Hands the command to the next own fighter.
            /// @details Original: SelectNextOwnFighter / EndCommandInput.
            void nextCommander();

            /// @brief Whether a fighter under a status 1..50 or 173 gives no command.
            /// @details Original: the end of SelectNextOwnFighter.
            /// @param slot Fighter slot.
            /// @return True when it cannot command.
            bool cannotCommand(int slot) const;

            /// @brief Clears the command: the new commander starts without one.
            /// @details Original: ResetFighterCommand.
            void resetCommand();

            /// @brief Makes a skill the command as CheckCanUseSkill does (a refusal says why) and sets the cursor it brings.
            /// @details Original: ChooseBattleSkill.
            /// @param skillId Skill id.
            /// @param showMessage Whether a refusal is shown in the chat.
            void chooseSkill(int skillId, bool showMessage = true);

            /// @brief Submits the chosen skill on a fighter, checked against whom it may reach.
            /// @details Original: SubmitActionOnTarget.
            /// @param targetSlot Target fighter slot.
            void submit(int targetSlot);

            /// @brief The item branch of submit.
            /// @details Original: packet 50/2.
            /// @param actor The commanding fighter.
            /// @param target The target fighter.
            /// @param targetSlot Target slot.
            void submitItem(const Fighter& actor, const Fighter& target, int targetSlot);

            /// @brief Whether the sealing status (effect 178 in the third or eighth place) leaves the skill unusable (only plain kinds remain).
            /// @param actor The commanding fighter.
            /// @param skillId Skill id.
            /// @return True when sealed.
            bool skillSealed(const Fighter& actor, int skillId) const;

            /// @brief Whether a glaze skill lacks its glaze: the four glaze skills take the first glaze of their element from the bag.
            /// @details Original: IsGlazeMissingForSkill; without a glaze the command starts over.
            /// @param skillId Skill id.
            /// @return True when the glaze is missing.
            bool glazeMissing(int skillId);

            /// @brief The command is sent: plays its sound and the next own fighter commands.
            void commandSent();

            /// @brief The command kind of a slot.
            /// @details Original: GetCommandSlotKind.
            /// @param slot Fighter slot.
            /// @return 1 the own player, 2 a pet, 0 others.
            int commandSlotKind(int slot) const;

            /// @brief Remembers the command for the last-command button.
            /// @details Original: RememberLastCommand.
            /// @param casterSlot Caster slot.
            /// @param targetSlot Target slot.
            /// @param skillId Skill id.
            void rememberLastCommand(int casterSlot, int targetSlot, int skillId);

            /// @brief Selects the fighter about to command; auto commands for it.
            /// @details Original: SelectBattleActor.
            void selectActor();

            /// @brief Issues the auto command: the last attack, else the first offensive recent skill, else an attack, on the first living enemy.
            /// @details Original: IssueAutoCommand (mode 3); mode 5 (the Remote Control with its Plus sets) tries RunAutoBattleSettings first.
            /// @param mode Auto mode.
            void issueAutoCommand(int mode);

            /// @brief Runs the Remote Control Plus sets to get a command.
            /// @details Original: RunAutoBattleSettings.
            /// @param sets The auto battle sets.
            /// @return False when none applies and the plain auto command follows.
            bool runAutoSettings(const AutoBattleSettings& sets);

            /// @brief Builds the battle as the Electron Control's script sees it for the commanding fighter.
            /// @return The view.
            RemoteBattleView remoteView() const;

            /// @brief Makes the script's choice the command and submits it, checked as the Plus sets' choices are.
            /// @param decision The choice.
            /// @param actorKind 1 the player commands, 2 the pet (for the outcome told to scriptOutcome).
            /// @return False when it cannot be used and the Plus sets follow.
            bool applyDecision(const RemoteDecision& decision, int actorKind);

            /// @brief Whether the commanding fighter may use a skill on a target now: learnt (a stunt only the role's own), SP enough,
            /// not sealed, and a target the skill may reach.
            /// @param skillId Skill id.
            /// @param targetSlot Target slot.
            /// @return True when the script's choice of it would be used.
            bool scriptSkillReady(int skillId, int targetSlot) const;

            /// @brief The own player's slot.
            /// @details Original: FindPlayerFighter.
            /// @return Slot, 0 when none.
            int ownFighterSlot() const;

            /// @brief The fighting pet's slot.
            /// @details Original: FindPetFighter.
            /// @return Slot, 0 when none.
            int ownPetSlot() const;

            /// @brief Finds an auto-battle target on a side.
            /// @details Original: FindAutoTarget. Mode 0: a side's nth living fighter in row/column order; 1 the nth with the least HP,
            /// 2 the nth with the least SP, that is hurt and does not hold the skill's status already; 3 the nth fallen one.
            /// The last one found when fewer are.
            /// @param side Side to search.
            /// @param nth Which match to take.
            /// @param mode Search mode 0..3.
            /// @param skillId Skill to be used.
            /// @param isItem Whether the command is an item.
            /// @return Slot, 0 when none.
            int findAutoTarget(
                int side,
                int nth,
                int mode,
                int skillId,
                bool isItem
            ) const;

            /// @brief Whether a skill is usable on a target.
            /// @details Original: IsSkillUsableOnTarget.
            /// @param skillId Skill id.
            /// @param casterSlot Caster slot.
            /// @param targetSlot Target slot.
            /// @return True when usable.
            bool skillUsableOn(int skillId, int casterSlot, int targetSlot) const;

            /// @brief Makes the bag item the commanding fighter's command.
            /// @details Original: ChooseBattleItemById.
            /// @param itemId Item id.
            /// @return True when the item was found and taken.
            bool chooseItemById(int itemId);

            /// @brief The first fighter standing on a side.
            /// @details Original: FindLivingFighterOnSide.
            /// @param side Side to search.
            /// @return Slot, 0 when none.
            int livingFighterOnSide(int side) const;

            /// @brief The enemy side of the own player.
            /// @return Side number.
            int enemySide() const;

            /// @brief Clears the status icons of a player on the own side.
            /// @details Original: ClearFighterStatuses.
            /// @param fighter The fighter.
            void clearStatusIcons(Fighter& fighter);

            /// @brief Adds a fighter to the field.
            /// @details Original: AddFightHum; showEnter brings it onto its cell with TEnterFight.
            /// @param record Fighter record.
            /// @param showEnter Whether it enters onto its cell.
            /// @param placeOthers Whether the others are put straight on their cells.
            void addFighter(const protocol::FighterRecord& record, bool showEnter, bool placeOthers);

            /// @brief Adds a fighter to the battle panel's member list.
            /// @details Original: AddBattleMember.
            /// @param slot Fighter slot.
            void addMember(int slot);

            /// @brief Advances a fighter entering the field; ending it puts the fighter in its cell's pose.
            /// @details Original: UpdateEnterFight and UpdateMove.
            /// @param fighter The entering fighter.
            /// @param nowMs Current time in ms.
            void updateEntering(Fighter& fighter, std::uint64_t nowMs);

            /// @brief Advances the battle's time limit clock.
            /// @param nowMs Current time in ms.
            void updateTimeLimit(std::uint64_t nowMs);

            /// @brief Ticks the entry action (kind 38).
            /// @details Original: TickFightAction of the entry; UpdateEnterFight sees FMoveDone one tick after the fighter arrived and
            /// IsFightActionDone frees it the tick after that.
            /// @param fighter The entering fighter.
            void tickEntering(Fighter& fighter);

            /// @brief Ends a fighter's entry.
            /// @param fighter The entering fighter.
            void endEntering(Fighter& fighter);

            /// @brief Repeats the commanding fighter's last command.
            /// @details Original: HandleBattleCommandClick button 8.
            void repeatLastCommand();

            GameServices& m_services;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            BattleField m_field;
            BattleGround m_ground;
            BattlePainter m_painter;
            std::vector<LightEffect> m_itemFlights;
            BattleCamera m_camera;
            BattleTween m_tween;
            SkillZoom m_skillZoom;
            std::optional<core::Rect> m_zoomBefore; // the skill zoom's part at the tick before the last (free frames)
            std::optional<core::Rect> m_zoomAfter; // the skill zoom's part at the last tick (free frames)
            std::uint64_t m_zoomTickMs = 0;
            std::optional<std::optional<core::Rect>> m_zoomPresented; // the part between the two ticks while presenting
            std::uint64_t m_holdMs = 0;
            std::uint64_t m_heldUntilMs = 0;
            std::array<std::optional<SkillAction>, BattleField::kSlots + 1> m_actions{};
            std::array<bool, BattleField::kSlots + 1> m_runningOff{};
            bool m_on = false;
            std::int32_t m_ownId = 0;
            int m_type = 0;
            int m_turnSeconds = 0;
            int m_actor = 0;
            int m_countdown = -1;
            std::uint64_t m_countdownMs = 0;
            int m_roundCountdown = -1;
            std::uint64_t m_roundCountdownMs = 0;
            bool m_timeRunningOut = false;
            bool m_commandPhase = false;
            int m_skill = 0;
            int m_item = 0;
            int m_itemSlot = 0;
            int m_glazeSlot = 0;
            ui::CursorKind m_cursor = ui::CursorKind::Normal;
            std::vector<int> m_members;
            bool m_ownGone = false;
            bool m_timeLimitOn = false;
            // 180 seconds: the battle's 3-minute limit
            int m_timeLeft = 180;
            std::uint64_t m_timeTickMs = 0;
            std::array<int, 3> m_autoCommands{};
            std::array<int, 3> m_lastCommandKinds{};
            std::array<int, 3> m_autoRotation{};
            std::uint32_t m_battleCount = 0; // battles started, for the Electron Control's script
            int m_round = 0; // command rounds of this battle
    };

}

#endif
