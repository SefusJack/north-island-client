#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_COMPUTER_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_COMPUTER_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HomeFurniture.h"
#include "client/models/crafting/MakeJobs.h"
#include "client/rules/MakeMaterials.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "gfx/Canvas.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief The house computer's form (ComputerWindow.xml, TH_ComputerForm): the five things under way, each with its place in the make queue.
    /// @details Original: TH_Menu.pas TH_ComputerForm, the owner's click on the Computer 39050. Place 0 means not queued; OK starts the queue one after the other (StartNextMaking, 64/2) as the server tells each is done, STOP clears it.
    class ComputerWindow {
        public:
            /// @brief Callbacks into the rest of the client that the computer needs.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
                std::function<bool()> inOwnHouse;
                std::function<bool()> bathing;
                std::function<int()> furnitureFlags;
                std::function<std::uint64_t()> nowMs;
                std::function<bool()> hasMaid;
                std::function<void(std::function<bool(int, int)>)> choosePartner;
                std::function<void()> partnerLeft;
            };

            /// @brief The make queue state.
            enum class State { None, Asked, Running };

            /// @brief Loads the window layout and wires the OK, STOP and close buttons.
            /// @param services Game services (layouts, recipes, items).
            /// @param screen Screen the window is loaded into.
            /// @param jobs The five make jobs under way.
            /// @param furniture The pieces of the house.
            /// @param materials Where blueprints and materials are found.
            /// @param messages Player message line for refusals.
            /// @param hooks Callbacks into the rest of the client.
            ComputerWindow(
                GameServices& services,
                ui::Screen& screen,
                MakeJobs& jobs,
                const HomeFurniture& furniture,
                const MakeMaterials& materials,
                PlayerMessages& messages,
                Hooks hooks
            );

            static constexpr int kComputerItem = 39050; // item 39050: Desktop Computer (house furniture, type 30)

            /// @brief Gives the make queue state.
            /// @return The state (Player.FComputerMakeState).
            State state() const { return m_state; }

            /// @brief Shows the window.
            /// @details Original: ShowCtrl, from the owner's click on the computer.
            void open();

            /// @brief Handles the job-stopped packet (64/3): the queue is cleared.
            /// @details Original: HandleMakePacket.
            void stopped();

            /// @brief Handles a finished or failed job (64/4, 64/5, 64/6 to 8): the slot leaves the queue and the next one starts.
            /// @details Original: HandleMakePacket.
            /// @param slot Make slot (1..5).
            void ended(int slot);

            /// @brief Ends the queue state (64/9); the queue itself stays.
            /// @details Original: FComputerMakeState 0.
            void endQueue() { m_state = State::None; }

            /// @brief Ends the asking when the form that asks who makes the things was closed.
            /// @details Original: TFNPC_MakeForm.HideCtrl.
            void partnersClosed() {
                if (m_state == State::Asked) {
                    m_state = State::None;
                }
            }

            /// @brief Per frame: checks the editors while the form is up; out of the house the form goes.
            /// @details Original: DrawControl's checks.
            void update();

        private:
            /// @brief Hides the window.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Takes the typed places and starts the queue; a house with a maid asks which partner makes the things.
            /// @details Original: StartMakingQueue.
            void start();

            /// @brief Clears the queue: the places go back to 0 (what was queued stays unfinished).
            /// @details Original: ClearMakingQueue.
            void clear();

            /// @brief Starts the lowest queued place (64/2); things no piece of the house can make leave the queue; no place left ends the queue.
            /// @details Original: StartNextMaking(AMakePetSlot, AMakePetId).
            /// @param petSlot Slot of the partner pet making the thing, 0 for none.
            /// @param petId Id of the partner pet, 0 for none.
            void startNext(int petSlot, int petId);

            /// @brief Finds the first piece of the house that is the facility (not in the retained list).
            /// @param facilityId Item id of the facility.
            /// @return The piece number, 0 for none.
            int facilityPiece(int facilityId) const;

            /// @brief Looks for the blueprint of a recipe the way StartNextMaking does: the bag, then the Bookcase, then the Desk.
            /// @param blueprintItemId Item id of the blueprint.
            /// @param source Receives the source of the blueprint (the Bookcase and the Desk both as 3).
            /// @param slot Receives the slot of the blueprint in its source.
            /// @return False when it is needed and not at hand.
            bool blueprintFound(int blueprintItemId, int& source, int& slot) const;

            /// @brief Sets the text of a slot's place field when it differs.
            /// @param slot Make slot (1..5).
            /// @param text New field text.
            void setEditor(int slot, const std::string& text);

            /// @brief Reads the place typed in a slot's field.
            /// @details Original: StrToInt of a field.
            /// @param slot Make slot (1..5).
            /// @return The queue place, 0 for an empty field.
            int order(int slot) const;

            /// @brief Paints each thing under way with its name and the bar of how far it is.
            /// @details Original: DrawControl.
            /// @param canvas Target canvas.
            /// @param at Top-left corner of the paint area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Keeps each field at most the number of things under way, and 0 when the blueprint or the making piece is gone or no thing is there.
            /// @details Original: DrawControl's per-slot part.
            void checkEditors();

            GameServices& m_services;
            ui::Screen& m_screen;
            MakeJobs& m_jobs;
            const HomeFurniture& m_furniture;
            const MakeMaterials& m_materials;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            std::array<std::reference_wrapper<ui::TextBox>, MakeJobs::kSlots> m_editors;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_frameSpot;
            const ui::Spot& m_barSpot;
            State m_state = State::None;
            std::array<int, MakeJobs::kSlots + 1> m_queue{};
    };

}

#endif
