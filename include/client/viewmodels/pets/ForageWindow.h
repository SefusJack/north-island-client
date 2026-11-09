#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_PETS_FORAGE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_PETS_FORAGE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/FigureAnimation.h"
#include "client/models/pets/ForageFile.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/models/pets/PetTeam.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/DrawingArea.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>

namespace client {

    /// @brief The Dog Bone's form: the pet that goes out to find items and three targets filled by dragging stackable items from the bag.
    /// @details Original: ForageWindow.xml, MK_Menu.pas TForageForm. OK sends the pet for the first target (23/88); the server answers each find (23/146) and the form goes on to the next, until none is left or the search fails or is cancelled (23/147).
    class ForageWindow {
        public:
            /// @brief Callbacks into the rest of the game.
            struct Hooks {
                std::function<core::Rect()> itemWindowBounds;
                std::function<bool(int place)> petFree;
                std::function<void()> marksChanged;
                std::function<std::int32_t()> playerId;
            };

            static constexpr int kTargets = 3;
            static constexpr int kBoneMark = BagMarks::kForageBone;

            /// @brief Loads the form and wires its painters and buttons.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param inventory The player's bag.
            /// @param marks Bag cell marks (use / busy).
            /// @param pets The pet team.
            /// @param messages Sink for on-screen messages.
            /// @param hooks Callbacks into the rest of the game.
            ForageWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                BagMarks& marks,
                const PetTeam& pets,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the form is showing.
            /// @return True when the window is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form beside the item window because a Dog Bone was used.
            /// @details Original: OpenForageWithBone.
            /// @param bagSlot Bag place of the Dog Bone.
            void open(int bagSlot);

            /// @brief Tells whether a pet is out finding, for the forms that refuse such a pet.
            /// @details Original: TFollowNpc.FForaging.
            /// @param place Pet place in the team.
            /// @return True when that pet is foraging.
            bool isForaging(int place) const { return place >= 1 && place <= PetTeam::kPlaces && m_petForaging[static_cast<std::size_t>(place)]; }

            /// @brief Puts the stackable item of a bag place on the target under the mouse.
            /// @details Original: AddForageTargetItem (an item of the bag dropped on the form).
            /// @param bagSlot Bag place of the dropped item.
            void addTarget(int bagSlot);

            /// @brief Handles a find: shows it, clears the target and sends the next one.
            /// @details Original: RecvForageFound (23/146).
            /// @param itemId Item found.
            /// @param count Quantity found.
            void found(int itemId, int count);

            /// @brief Gives the bone's cell the mark that keeps it where it is.
            /// @details Original: MarkForageBone (23/152): the bone's cell cannot be moved (the use mark that does the same, seen only after entering the game with a search going on).
            /// @param boneSlot Bag place of the Dog Bone.
            void markBone(int boneSlot);

            /// @brief Handles the end of a search and resets the form.
            /// @details Original: RecvForageEnd (23/147): code 1 the search failed, else it was cancelled.
            /// @param boneSlot Bag place of the Dog Bone.
            /// @param petPlace Place of the pet that was out.
            /// @param code 1 the search failed, otherwise it was cancelled.
            void ended(int boneSlot, int petPlace, int code);

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void hide() { m_window.setVisible(false); }

        private:
            /// @brief Saves the targets and sends the pet for the first one.
            /// @details Original: StartForage, SaveForageSettings.
            void start();

            /// @brief Cancels a running search (asks the server) and resets the form.
            /// @details Original: CancelForage.
            void cancel();

            /// @brief Closes the form: a pet out finding goes on, otherwise the bone is let go.
            /// @details Original: CloseForageForm.
            void closeForm();

            /// @brief Clears targets, marks and pet; hides the form.
            /// @details Original: ResetForage.
            void reset();

            /// @brief Finds the first pet of the join order that may go.
            /// @details Original: FindForagePet.
            void findPet();

            /// @brief Looks the pet up again by its kind, then asks the server for the first target that can be found.
            /// @details Original: SendNextForageTarget (23/88).
            void sendNext();

            /// @brief Finds the target under the mouse.
            /// @return Target 1..3, or 0 for none.
            int targetUnderMouse() const;

            /// @brief Empties a target.
            /// @details Original: ClearForageTarget.
            /// @param target Target 1..3.
            void clearTarget(int target);

            /// @brief Right button over a target empties it (not while a pet is out).
            /// @details Original: HandleForageTargetClick.
            void clickTarget();

            /// @brief Tells whether any target holds an item.
            /// @return True when at least one target is filled.
            bool hasTargets() const;

            /// @brief Fills the targets from the saved file.
            /// @details Original: LoadForageSettings, LoadForageTargetsToImages.
            void loadTargets();

            /// @brief Draws the pet (standing or turned away while out) and its name.
            /// @details Original: DrawControl.
            /// @param canvas Canvas to draw on.
            /// @param at Position of the drawing area.
            /// @param nowMs Current time in milliseconds.
            void paintPet(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Draws a target's item icon whole.
            /// @param canvas Canvas to draw on.
            /// @param at Position of the target's area.
            /// @param target Target 1..3.
            void paintTarget(gfx::Canvas& canvas, core::Point at, int target);

            /// @brief Sets a target area's hint to its item's name.
            /// @param target Target 1..3.
            void setTargetHint(int target);

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            BagMarks& m_marks;
            const PetTeam& m_pets;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            std::array<std::reference_wrapper<ui::DrawingArea>, kTargets> m_targetAreas;
            const ui::Spot& m_petSpot;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_textColorSpot;
            std::array<int, kTargets + 1> m_targets{};
            std::array<int, kTargets + 1> m_saved{};
            int m_boneSlot = 0;
            int m_petPlace = 0;
            int m_petKind = 0;
            bool m_foraging = false;
            bool m_boneUsed = false;
            int m_current = 0;
            int m_targetIndex = 0;
            int m_targetCount = 0;
            int m_ordinal = 0;
            std::array<bool, PetTeam::kPlaces + 1> m_petForaging{};
            FigureAnimation m_animation;
    };

}

#endif
