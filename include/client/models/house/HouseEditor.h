#ifndef NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_EDITOR_H
#define NORTH_ISLAND_CLIENT_MODELS_HOUSE_HOUSE_EDITOR_H

#include "client/models/house/HouseDrag.h"
#include "client/models/house/HouseInterior.h"
#include "client/rules/HouseWallDrag.h"
#include "client/models/house/HouseWallLight.h"
#include "client/models/world/Camera.h"
#include "core/Bytes.h"
#include "core/Point.h"
#include "formats/ItemTable.h"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief What the owner does with the furniture of the house: carry a piece, put a bag item down, turn or take back a piece, use a Work Platform or a recycling bin.
    /// @details Original: DZ_Thing.pas TGdUtensil's mouse handlers and HandleUtensilDrop, Se_EquipUI.pas HandleItemDrop in a house,
    /// RE_Menu.pas TRE_FurAdjust.RotateFurniture / GatherFurniture.
    class HouseEditor {
        public:
            /// @brief Callbacks into the rest of the client.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<void(const std::string&, int)> notice;
                std::function<void(const std::string&, std::function<void()>)> confirm;
                std::function<void(const std::string&, bool, std::function<void()>, std::function<void()>)> choose;
                std::function<std::int32_t()> ownId;
                std::function<core::Point()> playerFeet;
                std::function<bool()> locked;
                std::function<bool(int)> queueBlocks;
                std::function<bool(const std::vector<int>&)> roomInBag;
                std::function<void(int)> used;
                std::function<bool(int, int)> shelfDrop;
            };

            /// @brief Creates the editor.
            /// @param interior The house interior edited.
            /// @param items Item table.
            /// @param hooks Callbacks into the rest of the client.
            HouseEditor(
                HouseInterior& interior,
                const formats::ItemTable& items,
                Hooks hooks
            );

            /// @brief The piece being carried, if any.
            /// @return The drag state.
            const std::optional<HouseDrag>& drag() const { return m_drag; }

            /// @brief What the carried wall piece (a placed piece or a bag item) lights up.
            /// @return The light, if any.
            const std::optional<HouseWallLight>& wallLight() const { return m_light; }

            /// @brief Once a frame: follows a carried bag item or wall piece to the wall cell or pillar under the mouse.
            /// @param bagItem The bag item carried over the house, if any.
            /// @param mouse The mouse position.
            /// @param camera The camera.
            void update(std::optional<int> bagItem, core::Point mouse, const Camera& camera);

            /// @brief The cell the mouse is over, on level 1.
            /// @details Original: UpdateMouseCell.
            /// @param mouse The mouse position.
            /// @param camera The camera.
            /// @return The cell.
            static HouseGrid::Cell cellUnder(core::Point mouse, const Camera& camera);

            /// @brief The owner takes hold of a piece.
            /// @details Original: TGdUtensil.HandleMouseDown.
            /// @param slot The utensil slot.
            /// @param mouse The mouse position.
            /// @param camera The camera.
            /// @return True when the piece is held.
            bool press(int slot, core::Point mouse, const Camera& camera);

            /// @brief The owner takes a piece of the pick-up area by its row; it is carried like a standing one.
            /// @details Original: DragRetainedFurniture.
            /// @param furnIndex The piece's number.
            /// @param mouse The mouse position.
            /// @param camera The camera.
            /// @return True when the piece is held.
            bool pressRetained(int furnIndex, core::Point mouse, const Camera& camera);

            /// @brief The mouse moves while a piece is held: works out where it would drop.
            /// @details Original: CalcDragPlaceCell.
            /// @param mouse The mouse position.
            /// @param camera The camera.
            void dragTo(core::Point mouse, const Camera& camera);

            /// @brief The button comes up: drops the held piece (move, recycle, onto a platform, or a click).
            /// @details Original: HandleUtensilDrop.
            /// @param mouse The mouse position.
            /// @param targetSlot The utensil slot of the piece under the mouse, 0 for none.
            /// @param camera The camera.
            void release(core::Point mouse, int targetSlot, const Camera& camera);

            /// @brief Right click on a piece: asks Rotate or Gather, or refuses while it holds things.
            /// @details Original: TGdUtensil.HandleRightMouseUp.
            /// @param slot The utensil slot.
            void rightClick(int slot);

            /// @brief A bag item let go over the map.
            /// @details Original: Se_EquipUI.pas HandleItemDrop in a house, DZ_Thing.pas RequestDropOneThing.
            /// @param bagSlot The bag place.
            /// @param itemId The item.
            /// @param count The stack size.
            /// @param mouse The mouse position.
            /// @param targetSlot The utensil slot of the piece under the mouse, 0 for none.
            /// @param camera The camera.
            /// @return True when the house took it.
            bool dropBagItem(
                int bagSlot,
                int itemId,
                int count,
                core::Point mouse,
                int targetSlot,
                const Camera& camera
            );

            /// @brief A stack of the Storeroom let go over the map; a Recycle Bin under the mouse takes it.
            /// @details Original: HandleGridItemDrop (a slot of kind 11), request 62/12.
            /// @param cell The Storeroom cell.
            /// @param itemId The item.
            /// @param count The stack size.
            /// @param piece The piece number the item goes with.
            /// @param targetSlot The utensil slot of the piece under the mouse.
            /// @return True when the bin took it.
            bool dropStoreroomItem(
                int cell,
                int itemId,
                int count,
                int piece,
                int targetSlot
            );

        private:
            /// @brief Tells whether the player owns the house.
            /// @return True for the owner.
            bool owner() const { return m_interior.furniture().ownerId() == m_hooks.ownId(); }

            /// @brief The room a held piece would stand in (a pick-up area piece goes to the player's room).
            /// @return The room index.
            int roomOfHeld() const;

            /// @brief The cell under the held piece's middle, with the level it would lie at.
            /// @details Original: CalcDragPlaceCell.
            /// @param mouse The mouse position.
            /// @param camera The camera.
            /// @return The cell.
            HouseGrid::Cell placeCell(core::Point mouse, const Camera& camera) const;

            /// @brief A piece dropped on a recycling bin: after the question the piece goes in.
            /// @param piece The piece's number.
            /// @param bin The bin piece's number.
            void dropPieceOnBin(int piece, int bin);

            /// @brief A bag item dropped on a recycling bin: after the question the item goes in.
            /// @param bagSlot The bag place.
            /// @param itemId The item.
            /// @param count The stack size.
            /// @param bin The bin piece's number.
            void dropItemOnBin(int bagSlot, int itemId, int count, int bin);

            /// @brief Applies the Recycle Bin's refusals for an item, else asks the question before the request goes.
            /// @param itemId The item.
            /// @param request The request sent when confirmed.
            void askRecycle(int itemId, core::Bytes request);

            /// @brief The checks for a tool dropped on a Work Platform.
            /// @param toolItem The tool item.
            /// @param platformPiece The platform piece's number.
            /// @return True when the tool may be put there.
            bool toolFitsPlatform(int toolItem, int platformPiece);

            /// @brief Takes a piece back into the bag (and the tools on a Work Platform), when the bag has room.
            /// @details Original: GatherFurniture.
            /// @param slot The utensil slot.
            void gather(int slot);

            /// @brief Turns a piece to its other facing at the same cell.
            /// @details Original: RotateFurniture.
            /// @param slot The utensil slot.
            void rotate(int slot);

            /// @brief The question text before an item is destroyed.
            /// @param itemId The item.
            /// @return The text.
            std::string destroyText(int itemId) const;

            /// @brief A wall piece's drop cell and facing, and what it lights.
            /// @details Original: HitTestDragWallCell and the wall part of CalcDragPlaceCell.
            /// @param itemId The item.
            /// @param mouse The mouse position.
            /// @param middle The middle of the held picture.
            /// @param direction The facing it has now.
            /// @param camera The camera.
            /// @return The place, nothing when the item has no wall level.
            std::optional<HouseWallDrag::Place> placeOnWall(
                int itemId,
                core::Point mouse,
                core::Point middle,
                int direction,
                const Camera& camera
            );

            /// @brief Tells whether an item is a wall piece.
            /// @param itemId The item.
            /// @return True for a wall piece.
            bool isWallPiece(int itemId) const;

            /// @brief The room and pillar checks of a wall piece put down from the bag.
            /// @param cell The cell.
            /// @param direction The facing.
            /// @return True when it may stand there.
            bool wallStands(const HouseGrid::Cell& cell, int direction) const;

            HouseInterior& m_interior;
            const formats::ItemTable& m_items;
            Hooks m_hooks;
            HouseWallDrag m_walls;
            std::optional<HouseDrag> m_drag;
            std::optional<HouseWallLight> m_light;
    };

}

#endif
