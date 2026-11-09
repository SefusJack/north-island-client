#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_REFER_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_REFER_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/data/ReferMaterials.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "core/Rect.h"
#include "gfx/Canvas.h"
#include "protocol/ItemInstance.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <functional>
#include <optional>
#include <string>

namespace client {

    /// @brief The Magnifying Glass form: shows the numbers and materials of a bag item.
    /// @details Original: ReferWindow.xml, CJ_ReferToMaterial.pas TReferToMaterial, item 34179. An item with a rank and a level is dropped on the place, Query asks "Confirm <glass> Search for file?" and sends 23/103; the answer (23/166) shows the two numbers of the item and the materials it is made of.
    class ReferWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<core::Rect()> itemWindowBounds;
                std::function<void(const core::Bytes&)> send;
                std::function<void(const std::string&, std::function<void()>)> ask;
                std::function<bool()> inFight;
            };

            static constexpr int kMagnifyingGlass = 34179; // item 34179: Magnifying Glass

            /// @brief Loads the window layout and wires its events.
            /// @param services Game services.
            /// @param screen Screen the window is loaded into.
            /// @param inventory The player's inventory.
            /// @param marks Bag cell marks.
            /// @param messages Player message line.
            /// @param info Item info popup.
            /// @param hooks Owner callbacks.
            ReferWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                const BagMarks& marks,
                PlayerMessages& messages,
                ItemInfoPopup& info,
                Hooks hooks
            );

            /// @brief Opens the form beside the item window when the glass was used.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Takes a bag item dropped on the form.
            /// @details Original: PutItemToRefer.
            /// @param bagSlot Bag slot of the dropped item.
            void put(int bagSlot);

            /// @brief Handles the server's answer: code 1 the numbers and materials, code 2 it cannot be searched.
            /// @details Original: RecvReferResult (23/166).
            /// @param payload The packet payload.
            void result(const core::Bytes& payload);

            /// @brief Drops an item that left its bag place and shows the placed item's info under the mouse.
            /// @details Original: DrawControl: CheckReferItemChanged.
            void update();

        private:
            /// @brief Clears the item and hides the form.
            /// @details Original: CloseReferToMaterial.
            void close();

            /// @brief Clears the item and its place.
            /// @details Original: ClearReferItem.
            void clearItem();

            /// @brief Clears the item and its results.
            /// @details Original: ClearReferMaterial.
            void clearMaterial();

            /// @brief Asks to confirm the search when an item is placed and a glass is at hand.
            /// @details Original: AskReferSearch.
            void ask();

            /// @brief Finds the first Magnifying Glass in the bag that is free of other uses.
            /// @details Original: FindMagnifyingGlassSlot.
            /// @return Bag slot, or 0 for none.
            int glassSlot() const;

            /// @brief Sends the search request.
            /// @details Original: ConfirmReferSearch.
            void confirm();

            /// @brief Paints the item's name, numbers, materials and picture.
            /// @param canvas Target canvas.
            /// @param at Top-left of the area.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            const BagMarks& m_marks;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::Element& m_image;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_firstNumberSpot;
            const ui::Spot& m_secondNumberSpot;
            const ui::Spot& m_materialPictureSpot;
            const ui::Spot& m_materialNameSpot;
            int m_slot = 0;
            int m_itemId = 0;
            std::string m_name;
            int m_glass = 0;
            protocol::ItemInstance m_shown;
            int m_first = -1;
            int m_second = -1;
            std::array<std::optional<ReferMaterials::Material>, 5> m_materials;
    };

}

#endif
