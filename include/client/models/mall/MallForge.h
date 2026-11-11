#ifndef NORTH_ISLAND_CLIENT_MODELS_MALL_MALL_FORGE_H
#define NORTH_ISLAND_CLIENT_MODELS_MALL_MALL_FORGE_H

#include "client/services/GameServices.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/models/mall/MallShop.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/viewmodels/chat/TalkDialog.h"
#include "protocol/ItemInstance.h"

#include <functional>
#include <string>

namespace client {

    /// @brief The forging room of the item mall: one piece of gear picked from the bag, its cost, the question and the server's answer.
    /// @details Original: tab 7 of THL_ShoppingForm. The picked cell is marked in use; it shows what forging costs or the Strong Scrolls it needs; Confirm asks and sends 75/3; the server's answers (75/6) leave it forged, refused or put back.
    class MallForge {
        public:
            /// @brief Creates the forge.
            /// @param services Game services (items, connection, sound).
            /// @param inventory The player's bag.
            /// @param marks Bag cell marks.
            /// @param shop The mall (for the IM points).
            /// @param messages Short message display.
            /// @param talk Question dialog.
            /// @param changed Called when the bag's marks or an item changed (the windows showing the bag redraw).
            MallForge(
                GameServices& services,
                Inventory& inventory,
                BagMarks& marks,
                const MallShop& shop,
                PlayerMessages& messages,
                TalkDialog& talk,
                std::function<void()> changed
            );

            /// @brief The bag place in the forge.
            /// @return The place, or 0 for none.
            int bagSlot() const { return m_bagSlot; }

            /// @brief The copy of the item the forge shows.
            /// @return The item.
            const protocol::ItemInstance& item() const { return m_item; }

            /// @brief The points the forging costs, drawn beside "Points Consumed for Forging".
            /// @return The cost.
            int cost() const { return m_cost; }

            /// @brief The level drawn under the cost: "+3" of the strengthening, or "+" and the item's fusion level.
            /// @return The text.
            std::string levelText() const;

            /// @brief Puts gear of the bag into the forge and marks its cell in use.
            /// @details Original: SelectForgeItem.
            /// @param bagSlot Bag place.
            void select(int bagSlot);

            /// @brief Takes the picked gear out again (right button on the cell).
            /// @details Original: ReleaseForgeItem.
            /// @param bagSlot Bag place clicked.
            void release(int bagSlot);

            /// @brief Empties the forge unless a forge was sent.
            /// @details Original: CancelForge.
            void cancel();

            /// @brief Asks the question whose Confirm sends the forge.
            /// @details Original: ConfirmForge (then SendForge).
            void confirm();

            /// @brief The mall was shown: a forge not yet sent is put back.
            /// @details Original: ShowCtrl of the mall.
            void open();

            /// @brief The mall was hidden: a forge not yet sent is put back.
            /// @details Original: HideCtrl of the mall.
            void close();

            /// @brief Takes in the server's forge answer (75/6).
            /// @details Original: RecvForgeResult.
            /// @param outcome Result code 1..8.
            /// @param bagSlot Bag place of the forged item.
            /// @param fusionLevel New fusion level.
            void receive(int outcome, int bagSlot, int fusionLevel);

        private:
            /// @brief Empties the forge slot and frees the cell.
            /// @details Original: ClearForgeItem.
            void clear();

            /// @brief Makes the checks again, then sends 75/3 and waits for 75/6.
            /// @details Original: SendForge.
            void send();

            /// @brief Why gear of the bag may not be forged.
            /// @param record Item record.
            /// @param sending True when the forge is being sent.
            /// @return The message the game gives, empty when it may.
            std::string refusal(const formats::ItemRecord& record, bool sending) const;

            GameServices& m_services;
            Inventory& m_inventory;
            BagMarks& m_marks;
            const MallShop& m_shop;
            PlayerMessages& m_messages;
            TalkDialog& m_talk;
            std::function<void()> m_changed;
            protocol::ItemInstance m_item;
            int m_bagSlot = 0;
            bool m_sent = false;
            int m_cost = 0;
            bool m_strengthen = false;
            std::string m_costText;
            bool m_requestSent = false;
    };

}

#endif
