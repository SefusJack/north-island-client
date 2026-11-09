#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_SKILL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CHARACTER_SKILL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/pets/PetTeam.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/models/world/MapCharacter.h"
#include "core/Bytes.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/ScrollBar.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <functional>
#include <optional>
#include <vector>

namespace client {

    /// @brief The skill window (SkillWindow.xml, TRe_SkillForm): own skills by kind, hovered skill details, and the element's skill tree.
    /// @details Original: the arrows over the list switch the owner to the pets in joining order (SwitchSkillOwner).
    class SkillWindow {
        public:
            /// @brief Loads SkillWindow.xml and wires the window.
            /// @param services Game services (layout, skills, pictures, formula).
            /// @param screen Screen the window is loaded into.
            /// @param status Player status (skills, SP, element).
            /// @param pets Pet team.
            /// @param player The player's map character.
            /// @param messages Short on-screen message line.
            SkillWindow(
                GameServices& services,
                ui::Screen& screen,
                const PlayerStatus& status,
                const PetTeam& pets,
                const MapCharacter& player,
                PlayerMessages& messages
            );

            /// @brief Whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the window with the list as it is now, scrolled where it was left.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the window and remembers the scroll position of the tab.
            void close();

            /// @brief Opens the window when closed, else closes it.
            void toggle();

            /// @brief Sets the sender for packets such as learn skill.
            /// @param send Callback that sends the bytes.
            void setSend(std::function<void(const core::Bytes&)> send) { m_send = std::move(send); }

            /// @brief Sets the battle hook taken when a skill is clicked in a battle.
            /// @param pick Returns nothing when no battle is on, else whether the skill was taken (the window then closes).
            /// @details Original: UseSkillFromList in a battle makes the skill the command (ChooseBattleSkill).
            void setBattlePick(std::function<std::optional<bool>(int skillId)> pick) { m_battlePick = std::move(pick); }

            /// @brief Sets the handler for a skill dropped on the F1-F8 bar.
            /// @param putOnBar Receives the bar slot and the skill id.
            /// @details Original: SetHotKeySkill.
            void setHotKeyDrop(std::function<void(int slot, int skillId)> putOnBar) { m_putOnBar = std::move(putOnBar); }

            /// @brief Sets the handler for a skill dropped on a slot (1 .. 9) of the Remote Control Plus.
            /// @param putOnPlus Receives the slot and the skill id.
            /// @details Original: DropSkillOnAutoPlaySlot.
            void setPlusDrop(std::function<void(int slot, int skillId)> putOnPlus) { m_putOnPlus = std::move(putOnPlus); }

            /// @brief Refreshes the list after the skills or the status changed.
            void refresh();

        private:
            /// @brief One list row: the skill shown and the level beside its icon.
            struct Row {
                int skillId = 0;
                int level = 0;
            };

            /// @brief Switches to a tab and refills the list.
            /// @param tab Tab number 1 .. 5 (5 is the skill tree).
            /// @details Original: SelectSkillTab1..4, SelectSkillTreeTab.
            void selectTab(int tab);

            /// @brief Moves the owner to the next or previous of the player and the pets, round.
            /// @param forward True for the next owner.
            /// @details Original: SwitchSkillOwner.
            void switchOwner(bool forward);

            /// @brief The pet whose skills are shown.
            /// @return The pet, or nothing for the player or when it left the team.
            std::optional<std::reference_wrapper<const Pet>> shownPet() const;

            /// @brief Rebuilds the rows of the current tab and owner.
            /// @details Original: RefreshSkillList / AddSkillListRow.
            void fillList();

            /// @brief Scrolls the list, clamped to its range.
            /// @param top Index of the first visible row.
            void scrollTo(int top);

            /// @brief Shows the learn arrow beside rows of level-0 pet skills the skill points pay for.
            /// @details Original: UpdateSkillLearnButtons.
            void updateLearnButtons();

            /// @brief Sends the learn request for a row.
            /// @param row Row index.
            /// @details Original: SendLearnSkill, packet 28/1.
            void learnSkill(int row);

            /// @brief Number of list rows that fit the list area.
            /// @return Row count.
            int visibleRows() const;

            /// @brief The list row under the mouse.
            /// @return Row index, or -1 when none.
            /// @details Original: GetListHoverIndex.
            int hoveredRow() const;

            /// @brief Handles a click on a list row (battle command or SP check).
            /// @details Original: UseSkillFromList.
            void clicked();

            /// @brief Picks up the hovered row's icon for dragging.
            /// @details Original: TSe_SelectText.HandleMouseDown with dragging on.
            void pressed();

            /// @brief The skill record a row shows (the role's own skill for Newbie's Stunt).
            /// @param skillId Row skill id.
            /// @return The record, or nothing.
            std::optional<formats::SkillRecord> shownSkill(int skillId) const;

            /// @brief The player's learned entry of a skill.
            /// @param skillId Skill id.
            /// @return The entry, or nothing when not learned.
            std::optional<protocol::LearnedSkill> learned(int skillId) const;

            /// @brief Paints the owner's name and the tree or hovered-skill details.
            /// @param canvas Target canvas.
            /// @param at Origin of the drawing area.
            /// @details Original: TRe_SkillForm.DrawControl.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the hover bar and each row's icon, name with SP and level.
            /// @param canvas Target canvas.
            /// @param at Origin of the drawing area.
            /// @details Original: TRe_ImgListItem.DrawControl.
            void paintList(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the hovered skill: description, level, proficiency and kind.
            /// @param canvas Target canvas.
            /// @param at Origin of the drawing area.
            void paintHovered(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the element's skill tree with learned roots, and the needs of the cell under the mouse.
            /// @param canvas Target canvas.
            /// @param at Origin of the drawing area.
            /// @details Original: DrawSkillTreePage / BuildSkillTreeGrid.
            void paintTree(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const PlayerStatus& m_status;
            const PetTeam& m_pets;
            const MapCharacter& m_player;
            PlayerMessages& m_messages;
            ui::Window& m_window;
            ui::DrawingArea& m_list;
            ui::ScrollBar& m_scrollBar;
            ui::ImageButton& m_scrollUp;
            ui::ImageButton& m_scrollDown;
            const ui::DrawingArea& m_paint;
            const ui::Spot& m_rowsSpot;
            const ui::Spot& m_treeCellSpot;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_infoSpot;
            const ui::Spot& m_levelSpot;
            std::array<std::reference_wrapper<ui::ImageButton>, 5> m_tabs;
            std::array<std::reference_wrapper<ui::ImageButton>, 8> m_learn;
            std::vector<Row> m_rows;
            int m_tab = 1;
            int m_owner = 0;
            int m_top = 0;
            std::array<int, 6> m_tabTops{};
            std::function<void(int, int)> m_putOnBar;
            std::function<void(int, int)> m_putOnPlus;
            std::function<void(const core::Bytes&)> m_send;
            std::function<std::optional<bool>(int)> m_battlePick;
    };

}

#endif
