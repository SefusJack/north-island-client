#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_COMPOUND_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_COMPOUND_WINDOW_H

#include "client/services/GameServices.h"
#include "client/rendering/effects/LightEffect.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/viewmodels/chat/TalkDialog.h"
#include "ui/DrawingArea.h"
#include "ui/IconGrid.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <vector>

namespace client {

    /// @brief The compound window (CompoundWindow.xml): a bag copy, five material slots, Synthesis, the cauldron and the made item flying into its cell.
    /// @details Original: TRe_CompoundForm; ConfirmCompound sends 23/14 and its kinds, the result is 23/13. The junior alchemy box (skill 15998) and the skill menu (with 15999 as well) are kept in the character's save file.
    class CompoundWindow {
        public:
            /// @brief Loads the window layout and wires its buttons, bag, slots and painters.
            /// @param services Game services (layouts, items, skills, pictures, connection).
            /// @param screen Screen the layout is loaded into.
            /// @param inventory The player's inventory (bag).
            /// @param marks Bag cell marks (in-use, hidden).
            /// @param status The player's status (known skills).
            /// @param messages Player message line.
            /// @param talk Talk dialog used for confirmations.
            /// @param bagChanged Called when the bag cells' marks changed (the item window redraws them).
            /// @param mapId Returns the map the player is on.
            CompoundWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                BagMarks& marks,
                const PlayerStatus& status,
                PlayerMessages& messages,
                TalkDialog& talk,
                std::function<void()> bagChanged,
                std::function<int()> mapId
            );

            /// @brief Whether the window is showing.
            /// @return True while visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the window with the settings and skills loaded and empty slots (ShowCtrl).
            void open();

            /// @brief Closes the window: settings kept, slots and marks freed, cauldron idle (HideCtrl).
            void close();

            /// @brief Opens the window if closed, closes it if open.
            void toggle();

            /// @brief Redraws the bag copy and empties slots whose bag item is gone (RefreshGridItems, RefreshCompoundSlots).
            void refresh();

            /// @brief Sets the settings file (user/save<id xor 121>.dat) holding the junior box and chosen skill; read only when it names the player as its User.
            /// @param file Path of the settings file.
            /// @param playerId The player's id.
            void setSettingsFile(const std::filesystem::path& file, std::int32_t playerId) {
                m_settingsFile = file;
                m_playerId = playerId;
            }

            /// @brief Handles the compound result (RecvCompoundResult): the made item flies into its bag cell.
            /// @param itemId Made item id.
            /// @param bagSlot Bag cell the item lands in.
            void showResult(int itemId, int bagSlot);

        private:
            /// @brief One material slot: the bag cell it uses and a copy of the item and count.
            struct Slot {
                int bagSlot = 0;
                int itemId = 0;
                int count = 0;
            };

            /// @brief Puts a bag item into the first free material slot (BagSlotClicked / AddCompoundItem).
            /// @param bagSlot Bag cell clicked.
            void addFromBag(int bagSlot);

            /// @brief Empties one material slot and frees its bag mark (ClearCompoundSlot).
            /// @param slot Slot number 1..5.
            void clearSlot(int slot);

            /// @brief Empties all material slots (ClearCompoundSlots).
            void clearSlots();

            /// @brief Empties the slot that uses a bag cell (RemoveCompoundItemByBagSlot).
            /// @param bagSlot Bag cell.
            void removeByBagSlot(int bagSlot);

            /// @brief Checks the materials' rules and picks the kind of compound by the skills (ConfirmCompound), then sends or asks.
            void confirm();

            /// @brief Sends the compound request (23/14) and boils the cauldron once (SendCompoundConfirmed).
            void send();

            /// @brief Reads the junior box and skill from the settings file (LoadCompoundSettings); only a file naming this player as User counts.
            void loadSettings();

            /// @brief Writes the junior box and skill into an existing settings file (SaveCompoundSettings).
            void saveSettings() const;

            /// @brief Shows the skill menu or the junior alchemy box by the skills the player has (UpdateCompoundSkillOptions, ShowCompoundSkillMenu, ShowJuniorAlchemyOption).
            void layoutSkills();

            /// @brief Whether the player knows a skill.
            /// @param skillId Skill record id.
            /// @return True when the player's status lists it.
            bool hasSkill(int skillId) const;

            /// @brief Updates the junior alchemy check box picture.
            void showJunior();

            /// @brief Chooses a compound skill from the menu and closes the menu.
            /// @param skill Skill choice 1..3.
            void chooseSkill(int skill);

            /// @brief Starts the cauldron's idle animation.
            /// @param nowMs Current time in milliseconds.
            void startIdleCauldron(std::uint64_t nowMs);

            /// @brief Lands the flying made item at once and unhides its bag cell.
            void finishFlight();

            /// @brief Paints the skill marks, the cauldron and the flying result item (DrawControl).
            /// @param canvas Target canvas.
            /// @param at Top-left of the paint area.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints one slot's item icon and count (TComImage).
            /// @param canvas Target canvas.
            /// @param at Top-left of the slot.
            /// @param slot Slot number 1..5.
            void paintSlot(gfx::Canvas& canvas, core::Point at, int slot);

            static constexpr int kSlots = 5;

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            BagMarks& m_marks;
            const PlayerStatus& m_status;
            PlayerMessages& m_messages;
            TalkDialog& m_talk;
            std::function<void()> m_bagChanged;
            std::function<int()> m_mapId;
            ui::Window& m_window;
            ui::IconGrid& m_bag;
            const ui::Spot& m_cauldronSpot;
            const ui::Spot& m_flightSpot;
            const ui::Spot& m_countSpot;
            std::array<Slot, kSlots + 1> m_slots{};
            std::filesystem::path m_settingsFile;
            std::int32_t m_playerId = 0;
            std::vector<int> m_request;
            int m_requestKind = 0;
            bool m_junior = false;
            int m_skill = 0;
            bool m_skillMenuOpen = false;
            bool m_hasJunior = false;
            bool m_hasHigh = false;
            LightEffect m_cauldron;
            LightEffect m_flight;
            bool m_flying = false;
            int m_resultItem = 0;
            int m_litCell = 0;
            bool m_resultShown = false;
            std::uint64_t m_nowMs = 0;
    };

}

#endif
