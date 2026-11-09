#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_REMOTE_PLUS_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_SYSTEM_REMOTE_PLUS_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/pets/PetTeam.h"
#include "client/models/remote/AutoBattleSettings.h"
#include "client/models/remote/RemoteControl.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/CheckBox.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief Remote Control Plus form: what the battle's auto button does for the player (tab 1) and his pet (tab 2).
    /// @details Original: RemotePlusWindow.xml, HL_Menu.pas THL_AutoPlayFormPlus, opened by the Level-up Setting button of the Remote Control 34128. The sets are copied from and to RemoteControl::playSets() as the game's Player.FAfkSettingsSets; the Remote Control's own window reads and writes the file and presses OK.
    class RemotePlusWindow {
        public:
            /// @brief Callbacks into the game state the form needs.
            /// @details Original: skillLevel = Player.FLearnedSkills[key].FLevel; roleSkill = GetRoleSkillId(Player.FBodyType, Player.FHead); remoteFormOpen = TH_AutoPlayForm.FIsVisible; remoteItem = the item of Player.FAutoPlayItemSlot.
            struct Hooks {
                std::function<int(int skillKey)> skillLevel;
                std::function<int()> roleSkill;
                std::function<bool()> remoteFormOpen;
                std::function<int()> remoteItem;
            };

            /// @brief Loads the form and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the window is loaded into.
            /// @param remote The Remote Control state holding the sets in force.
            /// @param pets The player's pet team.
            /// @param messages Message line for refusals.
            /// @param hooks Callbacks into the game state.
            RemotePlusWindow(
                GameServices& services,
                ui::Screen& screen,
                RemoteControl& remote,
                const PetTeam& pets,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the form is visible.
            /// @return True while the form is open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form or closes it.
            /// @details Original: OpenAutoPlayPlus / ToggleAutoPlayPlusForm.
            void toggle();

            /// @brief Hides the form and gives up its changes.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Per-frame update: closes with the Remote Control form and mirrors typed values into the set.
            /// @details Original: DrawControl.
            void update();

            /// @brief Gives access to the form's own sets.
            /// @details Original: FSettings[1..2], [0] unused; the Remote Control's file is read into and written from them.
            /// @return The three-entry set array (index 0 unused).
            std::array<AutoBattleSettings, 3>& formSets() { return m_sets; }

            /// @brief Copies the sets in force to the form.
            /// @details Original: LoadAutoPlaySettings.
            void loadFromPlayer();

            /// @brief Puts the form's sets in force and closes the form.
            /// @details Original: SaveAutoPlaySettings.
            void save();

            /// @brief Takes a bag item dropped on a slot.
            /// @details Original: DropItemOnAutoPlaySlot.
            /// @param slot Slot number 1..9.
            /// @param itemId Id of the dropped item.
            void dropItem(int slot, int itemId);

            /// @brief Takes a listed skill dropped on a slot.
            /// @details Original: DropSkillOnAutoPlaySlot.
            /// @param slot Slot number 1..9.
            /// @param skillId Id of the dropped skill.
            void dropSkill(int slot, int skillId);

        private:
            /// @brief What a slot shows.
            /// @details Original: THL_MethodKey.
            struct Slot {
                int kind = 0;
                int id = 0;
                int defaultAction = 0;
                gfx::PictureId icon;
                bool hasIcon = false;
                std::string hint;
            };

            /// @brief Shows the form, only beside another Remote Control than the first, with the sets in force.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Shows a tab's controls and values.
            /// @details Original: ShowAutoPlayTab.
            /// @param tab Tab number (1 player, 2 pet, 3 the other tab).
            void showTab(int tab);

            /// @brief Selects a tab and shows it.
            /// @details Original: SelectAutoPlayTab.
            /// @param tab Tab number.
            void selectTab(int tab);

            /// @brief Sets the modified flag by comparing the form's sets with those in force.
            /// @details Original: CheckAutoPlayModified.
            void checkModified();

            /// @brief Flips an option tick of the current set.
            /// @details Original: ToggleAutoPlayOption.
            /// @param option Option number 1..6.
            void toggleOption(int option);

            /// @brief Opens or closes the target menu of a role button.
            /// @details Original: PickSkillTargetRole / PickHealTargetRole.
            /// @param selection 1..4 for a skill role button, 11 or 12 for a heal role button.
            void pickTargetRole(int selection);

            /// @brief Shows the target menu next to the picked role button.
            /// @details Original: ShowTargetRoleMenu.
            /// @param kind 1 for the skills' menu, 2 for the heal menu.
            void showTargetMenu(int kind);

            /// @brief Hides the target menu.
            /// @details Original: HideTargetRoleMenu.
            /// @param mode 0 both menus, 1 the skills', 2 the heal's.
            void hideTargetMenu(int mode);

            /// @brief Sets the picked target for the slot the menu came from, if the skill suits it.
            /// @details Original: SetAutoPlayTarget.
            /// @param target 1 enemy, 2 the player, 3 the pet, 4 the party.
            void setTarget(int target);

            /// @brief Sets the compare mode of the faint HP value.
            /// @details Original: ToggleAutoPlayCompare / SetAutoPlayCompare.
            /// @param mode 1 smaller, 2 surpass.
            void setCompare(int mode);

            /// @brief Empties a slot and its entry in the current set.
            /// @details Original: ClearAutoPlaySlot.
            /// @param slot Slot number 1..9.
            void clearSlot(int slot);

            /// @brief Puts the next common action (attack, defence, flee) into a slot on a double click.
            /// @details Original: SetDefaultActionKey.
            /// @param slot Slot number 1..9.
            void cycleDefaultAction(int slot);

            /// @brief Tells whether a slot takes the given entry, saying so when it does not.
            /// @details Original: CanSetAutoPlaySlot. Slots 1 and 2 take attacks, 3 to 5 anything that is not a seal, 6 and 7 the supplies, 8 the party's death, 9 a skill.
            /// @param slot Slot number 1..9.
            /// @param kind 1 for an item, 2 for a skill.
            /// @param id Item or skill id.
            /// @return True when the slot accepts the entry.
            bool canSet(int slot, int kind, int id);

            /// @brief Shows an entry's icon and hint in a slot.
            /// @details Original: SetAutoPlaySlotIcon.
            /// @param slot Slot number 1..9.
            /// @param kind 1 for an item, 2 for a skill.
            /// @param id Item or skill id, 0 for an empty entry.
            /// @param icon The entry's picture, if found.
            /// @param noMove True to leave the slot unset (the entry was empty or the move is off).
            /// @return True when the slot now shows the entry.
            bool setSlotIcon(
                int slot,
                int kind,
                int id,
                std::optional<gfx::PictureId> icon,
                bool noMove
            );

            /// @brief Stores an entry in the current set, with the target its kind of skill takes at first.
            /// @details Original: StoreAutoPlaySlot.
            /// @param slot Slot number 1..9.
            /// @param kind 1 for an item, 2 for a skill.
            /// @param id Item or skill id.
            /// @param noStore True to clear the slot's entry instead.
            /// @return True when the set was changed.
            bool storeSlot(int slot, int kind, int id, bool noStore);

            /// @brief Finds the picture of a skill (the three common ones have their own).
            /// @param skillId Skill id.
            /// @param found Set true when the picture exists.
            /// @return The picture id.
            gfx::PictureId skillIcon(int skillId, bool& found);

            /// @brief Finds the picture of an item.
            /// @param itemId Item id.
            /// @param found Set true when the picture exists.
            /// @return The picture id.
            gfx::PictureId itemIcon(int itemId, bool& found);

            /// @brief Says that the skill is not suitable for the target.
            void notSuitable();

            /// @brief Paints a slot's icon.
            /// @param canvas Target canvas.
            /// @param at Slot origin on the canvas.
            /// @param slot Slot number 1..9.
            void paintSlot(gfx::Canvas& canvas, core::Point at, int slot);

            /// @brief Paints the target menu panel and the arrows from a skill to its target.
            /// @details Original: DrawControl (before the controls).
            /// @param canvas Target canvas.
            /// @param at Window origin on the canvas.
            void paintOverlay(gfx::Canvas& canvas, core::Point at);

            /// @brief Sets a role button's picture, Btn_Role_<n>.
            /// @param button The button.
            /// @param role Role number 1..4.
            void setRolePicture(ui::ImageButton& button, int role);

            /// @brief Gives the set of the current tab.
            /// @return The current tab's settings.
            AutoBattleSettings& current() { return m_sets[static_cast<std::size_t>(m_tab)]; }

            /// @brief Reads a number from a text box.
            /// @param box The text box.
            /// @return The number typed in, or 0 when it is not a number.
            static int numberIn(const ui::TextBox& box);

            /// @brief Gives the position of the menu button of a place on the form.
            /// @param selection 1..4 for a skill role button, 11 or 12 for a heal role button.
            /// @return The button's top-left corner.
            core::Point roleButtonPosition(int selection) const;

            GameServices& m_services;
            ui::Screen& m_screen;
            RemoteControl& m_remote;
            const PetTeam& m_pets;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            std::array<std::reference_wrapper<ui::DrawingArea>, 10> m_keys; // PlusKey1..PlusKey9 (index 0 repeats key 1, so keys index from 1)
            std::array<std::reference_wrapper<ui::ImageButton>, 5> m_skillRoles;
            std::array<std::reference_wrapper<ui::ImageButton>, 3> m_healRoles;
            std::array<std::reference_wrapper<ui::ImageButton>, 4> m_skillMenu;
            std::array<std::reference_wrapper<ui::ImageButton>, 4> m_healMenu;
            std::array<std::reference_wrapper<ui::ImageButton>, 4> m_tabs;
            std::array<std::reference_wrapper<ui::CheckBox>, 7> m_checks;
            std::array<std::reference_wrapper<ui::TextBox>, 4> m_values;
            ui::ImageButton& m_compare;
            ui::ImageButton& m_cancel;
            std::array<AutoBattleSettings, 3> m_sets;
            std::array<Slot, 10> m_slots; // one per key 1..9 (index 0 unused)
            int m_tab = 1;
            bool m_modified = false;
            bool m_menuShown = false;
            int m_menuKind = 0;
            core::Point m_menuAt;
            int m_selection = 0;
            std::array<gfx::PictureId, 5> m_rolePictures;
            gfx::PictureId m_smaller;
            gfx::PictureId m_surpass;
            gfx::PictureId m_to;
            gfx::PictureId m_frame;
    };

}

#endif
