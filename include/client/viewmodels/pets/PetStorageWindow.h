#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_PETS_PET_STORAGE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_PETS_PET_STORAGE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/FigureAnimation.h"
#include "client/rendering/painters/PetInfoPainter.h"
#include "client/models/pets/Pet.h"
#include "client/models/pets/PetTeam.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "protocol/PetRecord.h"
#include "ui/BlockList.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace client {

    /// @brief The inn's pet storage form (original PetStorageWindow.xml, TRe_InnForm) and the pets in it (TPlayer.FInnPets, 10 places).
    /// @details Carried and stored pets are checked and moved over with the arrows; the server answers place by place and the form waits for it (FWaiting).
    class PetStorageWindow {
        public:
            /// @brief Builds the window and its form.
            /// @param services Shared game services.
            /// @param screen Screen the window is added to.
            /// @param pets The player's pet team.
            /// @param messages Sink for player messages.
            PetStorageWindow(
                GameServices& services,
                ui::Screen& screen,
                PetTeam& pets,
                PlayerMessages& messages
            );

            /// @brief Tells whether the window is visible.
            /// @return True while the form is open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Replaces worn item ids of the stored pets within a range.
            /// @details Original RevertItemVariantsTo20747: what the stored pets wear with an id from first to last takes the id to.
            /// @param first First id of the range.
            /// @param last Last id of the range.
            /// @param to The id that replaces them.
            void replaceWornIds(int first, int last, int to);

            /// @brief Opens the form.
            /// @details Original ShowCtrl (RefreshInnPetLists); onClosed is HideCtrl (the event goes on).
            /// @param onClosed Called when the form closes.
            void show(std::function<void()> onClosed);

            /// @brief Sets the stored pets from the server's records.
            /// @details Original RevInitINNFNpc.
            /// @param records The stored pet records.
            void setStored(const std::vector<protocol::PetRecord>& records);

            /// @brief Copies a carried pet into a storage place.
            /// @details Original RecvInnStorePet: the carried pet copied into a place (15/2 then takes it from the team).
            /// @param place Storage place (1 to 10).
            /// @param petSlot Slot of the carried pet.
            void stored(int place, int petSlot);

            /// @brief Copies the place as it was at the request into a carried slot.
            /// @details Original RecvInnPetRecord / RecvInnRetrievePet.
            /// @param place Storage place (1 to 10).
            /// @param petSlot Slot that receives the pet.
            void taken(int place, int petSlot);

            /// @brief Swaps a carried pet and a stored pet.
            /// @details Original RecvInnPetSwapped: all they are changes but the pet's order in the team.
            /// @param petSlot Slot of the carried pet.
            /// @param place Storage place of the stored pet.
            void swapped(int petSlot, int place);

            /// @brief Removes the stored pet of a place.
            /// @details Original RecvInnPetRemoved.
            /// @param place Storage place (1 to 10).
            void cleared(int place);

            /// @brief Gives the item a stored pet wears in a slot.
            /// @details Original RecvThingExpired 8.
            /// @param place Storage place (1 to 10).
            /// @param slot Worn slot (1 to 6).
            /// @return The item id, 0 for none.
            int wornBy(int place, int slot) const;

            /// @brief Replaces the item a stored pet wears in a slot (0: none).
            /// @param place Storage place (1 to 10).
            /// @param slot Worn slot (1 to 6).
            /// @param itemId The new item id.
            void replaceWorn(int place, int slot, int itemId);

            /// @brief Gives the name of a stored pet.
            /// @param place Storage place (1 to 10).
            /// @return The name, empty when the place holds no pet.
            std::string nameOf(int place) const;

            /// @brief Hides or shows the panel over the last rows of the stored pets.
            /// @details Original FInnRestrainHidden (23/136 kinds 3 and 4).
            /// @param hidden True to hide the panel.
            void setRestrainHidden(bool hidden) { m_restrain.setVisible(!hidden); }

            /// @brief Marks the server's answer as done (31/12) and refreshes.
            void done();

            /// @brief Rebuilds the rows after the team changed.
            /// @details Original RefreshInnPetLists.
            void refresh();

        private:
            /// @brief Checks or unchecks a carried pet row, refusing pets that may not be stored.
            /// @details Original CheckInnPetSelectable / CountCheckedStoredItems.
            /// @param row Row index.
            void clickCarried(int row);

            /// @brief Checks or unchecks a stored pet row.
            /// @param row Row index.
            void clickStored(int row);

            /// @brief Stores the checked carried pets, taking checked stored ones out at the same time.
            /// @details Original StorePetsInInn.
            void store();

            /// @brief Takes the checked stored pets out.
            /// @details Original TakePetsFromInn.
            void take();

            /// @brief Collects the slots of the checked carried pets.
            /// @return The slots, or nothing when one may not go.
            std::optional<std::vector<int>> checkedCarried();

            /// @brief Collects the places of the checked stored pets.
            /// @return The places.
            std::vector<int> checkedStored() const;

            /// @brief Counts the stored pets.
            /// @return The number of occupied places.
            int storedCount() const;

            /// @brief Finds the place of the n-th stored pet.
            /// @details Original GetInnStoredPetIndex.
            /// @param row Position among the stored pets, from 1.
            /// @return The place, or 0 when there is none.
            int storedPlace(int row) const;

            /// @brief Sends a request and waits for the answer.
            /// @param request The request packet.
            void send(const core::Bytes& request);

            /// @brief Closes the form and runs the close callback.
            void close();

            /// @brief Paints the friend picture, the inn icon and the pet under the mouse.
            /// @param canvas Target canvas.
            /// @param at Top left corner of the window.
            /// @param nowMs Current time in milliseconds.
            void paintForm(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the info block of every listed pet.
            /// @param canvas Target canvas.
            /// @param at Top left corner of the window.
            /// @param nowMs Current time in milliseconds.
            void paintInfos(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the pet names and the carried pets' elements.
            /// @param canvas Target canvas.
            /// @param at Top left corner of the window.
            void paintNames(gfx::Canvas& canvas, core::Point at);

            static constexpr int kPlaces = 10; // pet storage places, 1-based
            static constexpr int kCarried = 4; // pets the player carries with them

            GameServices& m_services;
            ui::Screen& m_screen;
            PetTeam& m_pets;
            PlayerMessages& m_messages;
            ui::Window& m_window;
            ui::Element& m_restrain;
            ui::BlockList& m_storedList;
            ui::BlockList& m_carriedList;
            const ui::Spot& m_previewSpot;
            const ui::Spot& m_textColorSpot;
            const ui::Spot& m_carriedInfoSpot;
            const ui::Spot& m_storedInfoSpot;
            const ui::Spot& m_nameOffsetSpot;
            const ui::Spot& m_carriedElementSpot;
            const ui::Spot& m_storedElementSpot;
            const ui::Spot& m_carriedNameSpot;
            const ui::Spot& m_untradableNameSpot;
            const ui::Spot& m_storedNameSpot;
            PetInfoPainter m_info;
            FigureAnimation m_animation;
            std::array<std::optional<Pet>, kPlaces + 1> m_stored{};
            std::array<std::optional<Pet>, kPlaces + 1> m_asRequested{};
            bool m_waiting = false;
            std::function<void()> m_onClosed;
    };

}

#endif
