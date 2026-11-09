#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_PETS_NPC_UPGRADE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_PETS_NPC_UPGRADE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/FigureAnimation.h"
#include "client/models/items/Inventory.h"
#include "client/models/pets/PetTeam.h"
#include "client/rules/PetWeapon.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "formats/NpcInfo.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief The pet rebirth form (original NpcUpgradeWindow.xml, Se_NpcUpgrade.pas TSe_NPCUpgradeForm).
    /// @details Opened by server 44/2; lists own pets of level 30 and friendship 60 that a type-37 bag item names, beside the pet each is reborn as. OK sends 44/1 with the pet's place.
    class NpcUpgradeWindow {
        public:
            /// @brief Callbacks the window uses to reach the rest of the client.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<void()> closed;
            };

            /// @brief Builds the window and its form.
            /// @param services Shared game services.
            /// @param screen Screen the window is added to.
            /// @param pets The player's pet team.
            /// @param inventory The player's bag.
            /// @param messages Sink for player messages.
            /// @param hooks Callbacks to the client.
            NpcUpgradeWindow(
                GameServices& services,
                ui::Screen& screen,
                const PetTeam& pets,
                const Inventory& inventory,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True while the form is open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form.
            /// @details Original ShowCtrl: CollectRebornCandidates, ShowRebornPreview and LoadRebornSkillIcons.
            void open();

        private:
            /// @brief One pet that can be reborn (original FPetList) with the bag place of the item that names it (FList).
            struct Candidate {
                int petPlace = 0;
                int bagSlot = 0;
            };

            /// @brief One of the two figures of the form (original FPetHumans): the pet and the pet it is reborn as.
            struct Side {
                bool shown = false;
                formats::NpcInfo info;
                std::string name;
                int level = 0;
                int hp = 0;
                int sp = 0;
                std::array<int, 7> stats{};
                PetWeapon weapon;
                std::array<std::string, 3> skillNames;
            };

            /// @brief Closes the form and lets the event go on (hooks.closed).
            void close();

            /// @brief Collects the pets that can be reborn.
            /// @details Original CollectRebornCandidates.
            void collect();

            /// @brief Shows the selected candidate and its reborn form.
            /// @details Original ShowRebornPreview and LoadRebornSkillIcons.
            void preview();

            /// @brief Fills the left side from a pet.
            /// @param pet The pet being shown.
            void previewPet(const Pet& pet);

            /// @brief Fills the right side with the pet the item reborns into.
            /// @param itemId Id of the bag item naming the new pet.
            void previewReborn(int itemId);

            /// @brief Loads the skill names of one side.
            /// @param side The side to fill.
            void loadSkills(Side& side);

            /// @brief Moves to the next or previous candidate.
            /// @details Original NextRebornPet and PrevRebornPet.
            /// @param direction Step, positive for next and negative for previous.
            void step(int direction);

            /// @brief Sends the reborn request (44/1 with the pet's place).
            /// @details Original RequestPetReborn.
            void request();

            /// @brief Paints the form.
            /// @details Original DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top left corner of the window.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints one figure with its stats.
            /// @param canvas Target canvas.
            /// @param at Top left corner of the window.
            /// @param nowMs Current time in milliseconds.
            /// @param side Index of the side (0 pet, 1 reborn pet).
            void paintSide(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs, int side);

            /// @brief Paints a number with the digit sprites.
            /// @param canvas Target canvas.
            /// @param at Position of the number.
            /// @param value The number.
            void paintNumber(gfx::Canvas& canvas, core::Point at, int value);

            /// @brief Paints the name of the skill under the mouse.
            /// @details Original RenderWithChildren.
            /// @param canvas Target canvas.
            /// @param at Top left corner of the window.
            void paintTooltip(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const PetTeam& m_pets;
            const Inventory& m_inventory;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            const ui::Spot& m_figureSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_elementSpot;
            const ui::Spot& m_skillSpot;
            const ui::Spot& m_skillSideSpot;
            const ui::Spot& m_levelSpot;
            const ui::Spot& m_hpSpot;
            const ui::Spot& m_spSpot;
            const ui::Spot& m_statsSpot;
            const ui::Spot& m_digitSpot;
            const ui::Spot& m_textColorSpot;
            std::vector<Candidate> m_candidates;
            int m_index = 0;
            std::array<Side, 2> m_sides;
            std::array<FigureAnimation, 2> m_animations;
    };

}

#endif
