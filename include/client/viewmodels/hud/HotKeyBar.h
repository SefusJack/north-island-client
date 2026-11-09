#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_HOT_KEY_BAR_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HUD_HOT_KEY_BAR_H

#include "client/services/GameServices.h"
#include "client/models/character/HotKeySlot.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "protocol/HotKeyRecord.h"
#include "core/Point.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/IconGrid.h"
#include "ui/ImageView.h"
#include "ui/Screen.h"

#include <array>
#include <functional>
#include <optional>
#include <vector>

namespace client {

    /// @brief The F1-F8 hot key bar HUD form (HotKeyBar.xml, TSe_HotKeyForm): three pages of eight slots for items and skills.
    /// @details Original: slots are kept by the server (packet 40/1 both ways).
    class HotKeyBar {
        public:
            /// @brief Callbacks into the rest of the client that the bar needs.
            struct Hooks {
                std::function<int(int itemId)> bagSlotOf;
                std::function<bool(int itemId)> isStackable;
                std::function<bool(int skillId)> playerKnows;
                std::function<bool(int skillId)> petKnows;
                std::function<int()> roleSkillId;
                std::function<std::optional<bool>(int skillId)> usableInBattle;
                std::function<int()> mapId;
                std::function<bool()> fighting;
                std::function<void(int itemId, int bagSlot)> battleItem;
                std::function<void(int skillId)> battleSkill;
                std::function<void(int bagSlot)> useItem;
                std::function<void(int kind, int id)> aim;
                std::function<int()> sp;
                std::function<std::optional<int>()> roleClothesNeeded;
            };

            /// @brief Loads the bar's layout, wires the screen events and places the form.
            /// @param services Shared game services (pictures, items, skills, connection).
            /// @param screen Screen the form is loaded into.
            /// @param messages Player message line used for refusals.
            /// @param hooks Callbacks into bag, skills and battle state.
            HotKeyBar(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Sets the handler for a slot dropped on a Remote Control Plus slot.
            /// @details Original: HandleHotKeyDragDrop on a Remote Control Plus slot (THL_MethodKey); the skill of the dragged place goes to the slot (DropSkillOnAutoPlaySlot).
            /// @param putOnPlus Receives the Plus slot number and the skill id.
            void setPlusDrop(std::function<void(int slot, int skillId)> putOnPlus) { m_putOnPlus = std::move(putOnPlus); }

            /// @brief Shows the bar.
            void show() { m_panel.setVisible(true); }

            /// @brief Hides the bar.
            void hide() { m_panel.setVisible(false); }

            /// @brief Fills the slots from the server's hot key records.
            /// @details Original: LoadHotKeysFromServer (40/1).
            /// @param records Records received from the server.
            void load(const std::vector<protocol::HotKeyRecord>& records);

            /// @brief Puts a bag item in a slot of the page shown and tells the server.
            /// @details Original: SetHotKeyItem.
            /// @param slot Slot number (1-8).
            /// @param itemId Item id.
            void putItem(int slot, int itemId);

            /// @brief Puts a skill in a slot of the page shown and tells the server.
            /// @details Original: SetHotKeySkill.
            /// @param slot Slot number (1-8).
            /// @param skillId Skill id.
            void putSkill(int slot, int skillId);

            /// @brief Per-frame refresh: drops items gone from the bag, greys unusable battle skills.
            /// @details Original: RefreshHotKeyAvailability, every frame the bar is up.
            void update();

            /// @brief Uses the item or skill in a slot of the page shown.
            /// @details Original: UseHotKeySlot, from F1-F8 or a click.
            /// @param slot Slot number (1-8).
            void use(int slot);

        private:
            /// @brief The game window in the panel's coordinates (a bigger window lets the form go to its edges).
            /// @return Window rectangle relative to the panel.
            core::Rect windowArea() const {
                const core::Rect& view = m_services.view;
                const core::Point panel = m_panel.bounds().position();
                return {view.x - panel.x, view.y - panel.y, view.width, view.height};
            }

            /// @brief Stores an item in a slot after validity checks.
            /// @details Original: SetHotKeyItem.
            /// @param slot Slot number (1-8).
            /// @param itemId Item id.
            /// @param send Whether to send the change to the server.
            /// @param page Page to store on; the page shown when nullopt.
            void setItem(int slot, int itemId, bool send, std::optional<int> page);

            /// @brief Stores a skill in a slot after checking the player or a pet knows it.
            /// @details Original: SetHotKeySkill.
            /// @param slot Slot number (1-8).
            /// @param skillId Skill id.
            /// @param send Whether to send the change to the server.
            /// @param page Page to store on; the page shown when nullopt.
            void setSkill(int slot, int skillId, bool send, std::optional<int> page);

            /// @brief Empties a slot.
            /// @details Original: ClearHotKeySlot.
            /// @param slot Slot number (1-8).
            /// @param page Page number (1-3).
            void clear(int slot, int page);

            /// @brief Sends a slot's content to the server.
            /// @details Original: SendHotKeySlot.
            /// @param slot Slot number (1-8).
            /// @param page Page number (1-3).
            void send(int slot, int page);

            /// @brief Handles an icon dragged from a slot: swap, drop on a Plus slot, or clear.
            /// @details Original: HandleHotKeyDragDrop.
            /// @param args Drag event data.
            void dropped(const ui::EventArgs& args);

            /// @brief Turns the page by a step, wrapping over the three pages.
            /// @param step +1 next, -1 previous.
            void turn(int step);

            /// @brief Shows the icon and hint of a slot of the page shown.
            /// @details Original: SetHotKeySlotImage.
            /// @param slot Slot number (1-8).
            /// @param grey Whether to use the grey icon of a skill.
            void showSlot(int slot, bool grey);

            /// @brief Uses an item from the bar: at once, refused, or as battle command / target cursor.
            /// @param itemId Item id.
            void useItem(int itemId);

            /// @brief Uses a skill from the bar: battle command or target cursor in the world.
            /// @param skillId Skill id.
            void useSkill(int skillId);

            /// @brief Lays out the frame picture, slots, arrows and page number at the bar's origin; minimized, only the small box and its button.
            /// @details Original: SetHotKeyLayout (se_mainUI.pas:15747-15804).
            void place();

            /// @brief Folds or restores the bar via the minimize button.
            /// @details Original: ToggleHotKeyMinimize (se_mainUI.pas:15806-15840).
            void toggleMinimized();

            static constexpr int kPages = 3;
            static constexpr int kSlots = 8;

            GameServices& m_services;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            std::function<void(int slot, int skillId)> m_putOnPlus;
            ui::Element& m_panel;
            ui::ImageView& m_pageNumber;
            ui::IconGrid& m_grid;
            ui::ImageView& m_background;
            ui::Element& m_frame;
            ui::ImageButton& m_previous;
            ui::ImageButton& m_next;
            ui::ImageButton& m_minimize;
            core::Point m_origin{744, 163}; // default top-left of the bar on screen (x, y)
            int m_layout = 1;
            int m_previousLayout = 0;
            core::Point m_grab;
            bool m_dragging = false;
            std::array<std::array<HotKeySlot, kSlots + 1>, kPages + 1> m_slots{};
            int m_page = 1;
    };

}

#endif
