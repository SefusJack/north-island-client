#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_UTENSILS_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_UTENSILS_H

#include "client/models/house/FurnitureRecord.h"
#include "client/models/house/HomeFurniture.h"
#include "client/models/house/HouseUtensil.h"
#include "core/Point.h"
#include "core/Rect.h"
#include "formats/ItemTable.h"
#include "gfx/PictureLibrary.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief The pieces of the house as they are drawn: a slot for each piece that stands in a room, found by the piece's number.
    /// @details Original: DZ_Thing.pas TUtensilManage's FUtensils, FFurnUtensilSlot and the carpets of THumManage.
    class HouseUtensils {
        public:
            static constexpr int kMost = HomeFurniture::kMost;

            /// @brief Creates an empty set of utensils.
            /// @param pictures Picture library.
            /// @param items Item table.
            /// @param furniture The house's furniture.
            HouseUtensils(
                gfx::PictureLibrary& pictures,
                const formats::ItemTable& items,
                const HomeFurniture& furniture
            );

            /// @brief Gives a placed piece a utensil slot and its picture and animation.
            /// @details Original: AddHomeUtensil after PlaceFurniture. A tool on a platform or a retained piece gets no slot.
            /// @param record The piece.
            /// @param furnIndex The piece's number.
            /// @param slot The slot to use, 0 for the first free one.
            /// @param nowMs Current time in milliseconds (animation start).
            /// @return The slot, 0 for none.
            int add(const FurnitureRecord& record, int furnIndex, int slot, std::uint64_t nowMs);

            /// @brief Frees a piece's slot.
            /// @details Original: RemoveHomeUtensil after RemoveFurniture.
            /// @param furnIndex The piece's number.
            /// @return The slot freed, 0 when the piece had none.
            int remove(int furnIndex);

            /// @brief Frees every slot and the carpets.
            /// @details Original: ClearAllUtensils.
            void clear();

            /// @brief Finds the slot of a piece.
            /// @details Original: FindUtensilByFurnIndex.
            /// @param furnIndex The piece's number.
            /// @return The slot, 0 for none.
            int slotOfPiece(int furnIndex) const;

            /// @brief Tells whether a piece of an item stands in a room.
            /// @param itemId The item.
            /// @return True when one does.
            bool standsIn(int itemId) const;

            /// @brief Where the picture of a piece is when the screen shows the map from an origin (centred on its floor point, 15 pixels down).
            /// @param slot The utensil slot.
            /// @param origin The map point at the screen's top left.
            /// @return The rectangle, empty without a picture.
            core::Rect pictureArea(int slot, core::Point origin) const;

            /// @brief The slot a piece was given.
            /// @details Original: FFurnUtensilSlot.
            /// @param furnIndex The piece's number.
            /// @return The slot, 0 for none.
            int slotOfFurniture(int furnIndex) const { return furnIndex >= 1 && furnIndex <= kMost ? m_slotOfPiece[static_cast<std::size_t>(furnIndex)] : 0; }

            /// @brief The utensil in a slot.
            /// @param slot The slot.
            /// @return The utensil.
            const HouseUtensil& utensil(int slot) const { return m_utensils[static_cast<std::size_t>(slot)]; }

            /// @brief The utensil in a slot, writable.
            /// @param slot The slot.
            /// @return The utensil.
            HouseUtensil& utensil(int slot) { return m_utensils[static_cast<std::size_t>(slot)]; }

            /// @brief The slots of the carpets in the order they were put, 0 for a free place.
            /// @details Original: HumManage.FCarpetSlots.
            /// @return The carpet slot list.
            const std::array<int, kMost + 1>& carpets() const { return m_carpets; }

            /// @brief Moves a piece standing on others by what the ones below ask.
            /// @details Original: ApplyUtensilStackOffset.
            /// @param slot The utensil slot.
            void applyStackOffset(int slot);

            /// @brief The picture of an item's facing.
            /// @param itemId The item.
            /// @param rotation The facing, 0 or 1.
            /// @return The picture, invalid when the item lacks it.
            gfx::PictureId facingPicture(int itemId, int rotation) const;

            /// @brief The size of a picture.
            /// @param picture The picture.
            /// @return Width and height, zero for an invalid picture.
            core::Point pictureSize(gfx::PictureId picture) const;

            /// @brief Tells whether the pixel of a picture at a point is not see-through.
            /// @details Original: IsPicPixelOpaque.
            /// @param picture The picture.
            /// @param local The point inside the picture.
            /// @return True when opaque.
            bool isOpaque(gfx::PictureId picture, core::Point local) const;

            /// @brief Sets a piece's animation state.
            /// @details Original: RecvNewYearCrackerPlay (4), RecvGuguClockChime (1), the lift (0).
            /// @param slot The utensil slot.
            /// @param state The animation state.
            void setAnimState(int slot, int state);

        private:
            gfx::PictureLibrary& m_pictures;
            const formats::ItemTable& m_items;
            const HomeFurniture& m_furniture;
            std::array<HouseUtensil, kMost + 1> m_utensils{};
            std::array<int, kMost + 1> m_slotOfPiece{};
            std::array<int, kMost + 1> m_carpets{};
    };

}

#endif
