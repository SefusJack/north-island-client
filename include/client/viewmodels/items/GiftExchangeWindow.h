#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_ITEMS_GIFT_EXCHANGE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_ITEMS_GIFT_EXCHANGE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/items/GiftCard.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "ui/DrawingArea.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <array>
#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief Gift Exchange Card window: choose what a card is exchanged for and how many times.
    /// @details Original: GiftExchangeWindow.xml, HL_Menu.pas TGiftExchangeCardForm, items of type 40. Four choices a page with a
    /// tick each, 1 to 50 times; Exchange asks and sends 23/133/2. Lists are kept in Data\Gec.Dat with the server's version: using
    /// a card sends 23/133/1 with that version, the server answers 23/108/2 with the list only when it differs, 23/108/4 opens the form.
    class GiftExchangeWindow {
        public:
            /// @brief Callbacks into the rest of the game.
            struct Hooks {
                std::function<bool()> fighting;
                std::function<void(const std::string&, std::function<void()>, std::function<void()>)> ask;
                std::function<void(const std::string&)> chatLine;
            };

            static constexpr int kPerPage = 4; // gift rows per page
            static constexpr int kMostTimes = 50; // most exchanges that can be typed in at once

            /// @brief Loads the window, wires its events and reads the kept lists.
            /// @param services Game services.
            /// @param screen Screen the window is loaded into.
            /// @param inventory Player inventory (read only).
            /// @param marks Bag cell marks (read only).
            /// @param messages Player message line.
            /// @param hooks Callbacks into the rest of the game.
            GiftExchangeWindow(
                GameServices& services,
                ui::Screen& screen,
                const Inventory& inventory,
                const BagMarks& marks,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Tells whether the window is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Uses the card of a bag place: opens the form if the list is fresh, else asks the server.
            /// @details Original: UseGiftCard(slot, 0).
            /// @param bagSlot Bag place of the card.
            void use(int bagSlot);

            /// @brief Handles the server's gift exchange packet.
            /// @details Original: RecvGiftExchangePacket (23/108): sub 1 the versions of the server's lists, 2 lists, 3 an exchange done,
            /// 4 open the form for a bag place.
            /// @param payload Packet bytes.
            void receive(const core::Bytes& payload);

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void hide() { m_window.setVisible(false); }

            /// @brief Per-frame check: closes the form when the card is gone or a fight begins and clamps the number of times to 0..50.
            /// @details Original: DrawControl.
            void update();

        private:
            /// @brief Opens the form for the card of a bag place (after 23/108/4).
            /// @details Original: UseGiftCard(slot, 1).
            /// @param bagSlot Bag place of the card.
            void open(int bagSlot);

            /// @brief Shows the form unless fighting or no card is set.
            /// @details Original: ShowCtrl.
            void show();

            /// @brief Clears the current card, page, selection and times.
            /// @details Original: ResetCard.
            /// @param clearFlags Also forget which lists the server confirmed this time.
            void reset(bool clearFlags);

            /// @brief Shows the rows and tick of the current page.
            /// @details Original: RefreshPage.
            void refreshPage();

            /// @brief Goes to the previous page.
            void prevPage();

            /// @brief Goes to the next page, or tells it is the last.
            void nextPage();

            /// @brief Toggles the tick of a row (one choice at a time).
            /// @details Original: CheckClick.
            /// @param tag Row on the page, 1..kPerPage.
            void check(int tag);

            /// @brief Asks whether to exchange the chosen item, then closes the form.
            /// @details Original: ExchangeClick.
            void exchange();

            /// @brief Checks cards and bag space and sends the exchange (23/133/2).
            /// @details Original: ConfirmExchange.
            void confirmExchange();

            /// @brief Number of pages of the current card.
            /// @return Page count (0 when no choices).
            int pageCount() const;

            /// @brief Handles 23/108/1: forgets unlisted cards and asks for lists whose version differs.
            /// @details Original: RecvGiftExchangePacket sub 1; the request is 90/1/1.
            /// @param payload Packet bytes.
            void receiveVersions(const core::Bytes& payload);

            /// @brief Handles 23/108/2: stores the sent lists and saves them to Data\Gec.Dat.
            /// @param payload Packet bytes.
            void receiveLists(const core::Bytes& payload);

            /// @brief Handles 23/108/3: prints the line about an exchange that was done.
            /// @param payload Packet bytes.
            void receiveDone(const core::Bytes& payload);

            /// @brief Finds a card in the kept lists.
            /// @param cardId Card item id (0 finds the first free place).
            /// @return Index, or -1 when none.
            int indexOf(int cardId) const;

            /// @brief Reads the number of times from the box.
            /// @return The number, 0 when the box is empty.
            int times() const;

            /// @brief Draws the card name, page numbers, row names and count pictures.
            /// @details Original: DrawControl of the form.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Draws the icon of one row's item.
            /// @param canvas Target canvas.
            /// @param at Top-left of the row's area.
            /// @param row Row on the page, 0-based.
            void paintItem(gfx::Canvas& canvas, core::Point at, int row);

            /// @brief Looks up an item's name.
            /// @param itemId Item id.
            /// @return The name, empty when unknown.
            std::string itemName(int itemId) const;

            GameServices& m_services;
            ui::Screen& m_screen;
            const Inventory& m_inventory;
            const BagMarks& m_marks;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_times;
            std::array<std::reference_wrapper<ui::ImageButton>, kPerPage> m_checks;
            std::array<std::reference_wrapper<ui::DrawingArea>, kPerPage> m_items;
            const ui::Spot& m_nameSpot;
            const ui::Spot& m_pageSpot;
            const ui::Spot& m_quantitySpot;
            const ui::Spot& m_markSpot;
            const ui::Spot& m_rowNameSpot;
            std::vector<GiftCard> m_cards;
            std::vector<bool> m_fresh;
            GiftCard m_current;
            int m_bagSlot = 0;
            int m_page = 1;
            int m_selected = 0;
            gfx::PictureId m_checkedPicture;
            gfx::PictureId m_uncheckedPicture;
    };

}

#endif
