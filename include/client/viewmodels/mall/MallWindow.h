#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MALL_MALL_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MALL_MALL_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "client/models/character/FigureAnimation.h"
#include "client/models/character/PlayerStatus.h"
#include "client/models/items/BagMarks.h"
#include "client/models/items/Inventory.h"
#include "client/models/mall/MallForge.h"
#include "client/models/mall/MallShop.h"
#include "client/viewmodels/items/ItemInfoPopup.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/viewmodels/chat/TalkDialog.h"
#include "client/models/world/MapCharacter.h"
#include "ui/IconGrid.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <array>
#include <functional>

namespace client {

    /// @brief The item mall window with catalog pages, try-on room, slot machine tab and forging room.
    /// @details Original: MallWindow.xml, THL_ShoppingForm. Catalog tabs show eight rows a page, Add in Cart, the
    /// try-on room with the player's figure, the hovered row's description and the cart; the slot machines' tab
    /// has eight game icons (75/4) and the forging room uses the bag, one piece of gear, Confirm and Cancel (MallForge).
    class MallWindow {
        public:
            /// @brief Callbacks into the battle state, the cart, the contents window and the bag.
            struct Hooks {
                std::function<bool()> fighting;
                std::function<int()> mapId;
                std::function<void()> bagChanged;
                std::function<void()> toggleCart;
                std::function<void()> showCart;
                std::function<void()> hideCart;
                std::function<bool(int itemId, std::uint64_t nowMs)> luckyBag;
                std::function<void(int boxId)> showContents;
                std::function<void()> hideContents;
            };

            /// @brief Loads the window layout and wires its buttons.
            /// @param services Shared game services.
            /// @param screen Screen the window is loaded into.
            /// @param shop Mall shop state (catalog, cart).
            /// @param status Player status.
            /// @param player The player's map character (for the try-on figure).
            /// @param inventory Player bag.
            /// @param marks Bag marks.
            /// @param messages Player message line.
            /// @param talk Talk dialog.
            /// @param info Item info popup.
            /// @param hooks Callbacks into the surrounding forms.
            MallWindow(
                GameServices& services,
                ui::Screen& screen,
                MallShop& shop,
                const PlayerStatus& status,
                const MapCharacter& player,
                Inventory& inventory,
                BagMarks& marks,
                PlayerMessages& messages,
                TalkDialog& talk,
                ItemInfoPopup& info,
                Hooks hooks
            );

            /// @brief Tells whether the window is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens or closes the mall; refused while the catalog is missing (it is asked for again).
            /// @details Original: OpenItemMall.
            void toggle();

            /// @brief Shows the mall.
            /// @details Original: THL_ShoppingForm.ShowCtrl.
            void show();

            /// @brief Hides the mall.
            /// @details Original: THL_ShoppingForm.HideCtrl.
            void hide();

            /// @brief Hides the mall in a battle and shows the info of the item under the mouse in the forging room's bag.
            /// @details Original: DrawControl. Runs after the item window's own update, which hides the info.
            void update();

            /// @brief Selects the first catalog tab.
            /// @details Original: ShopTab1Click.
            void showFirstTab() { selectTab(1); }

            /// @brief Sets whether the slot machines' tab may open.
            /// @details Original: RecvShopFlag (75/7).
            /// @param open True when the tab may open.
            void setGamesTab(bool open) { m_gamesTab = open; }

            /// @brief Refreshes the forging room's copy of the bag after the bag changed.
            /// @details Original: SyncShopBagGrid.
            void refresh();

            /// @brief Passes a forge result to the forging room.
            /// @details Original: RecvForgeResult (75/6).
            /// @param outcome Result code.
            /// @param bagSlot Bag slot of the forged piece.
            /// @param fusionLevel Fusion level reached.
            void showForgeResult(int outcome, int bagSlot, int fusionLevel) { m_forge.receive(outcome, bagSlot, fusionLevel); }

        private:
            /// @brief Selects a catalog tab and sets the figure's pose.
            /// @details Original: SelectShopTab.
            /// @param tab Tab number.
            void selectTab(int tab);

            /// @brief Turns the catalog page.
            /// @details Original: NextShopPage / PrevShopPage.
            /// @param step -1 for previous, 1 for next.
            void turnPage(int step);

            /// @brief Shows or hides the page buttons.
            /// @details Original: UpdateShopPageButtons.
            void updatePageButtons();

            /// @brief Adds the row at a page slot to the cart.
            /// @details Original: AddToCartClick.
            /// @param slot Row slot on the page.
            void addToCart(int slot);

            /// @brief Turns the try-on figure.
            /// @details Original: RotateModelNext / Prev.
            /// @param step Direction step.
            void rotate(int step);

            /// @brief Sets the try-on figure's pose.
            /// @details Original: SetModelPose.
            /// @param pose Pose number.
            void setPose(int pose);

            /// @brief Clears everything tried on.
            /// @details Original: ResetTryOn.
            void resetTryOn();

            /// @brief Wears or takes off the row under the mouse; the place takes an item and the figure is dressed again.
            /// @details Original: TryOnItem (ApplyTryOnStats).
            void tryOn();

            /// @brief Puts an item on a try-on place.
            /// @param equipSlot Equipment place.
            /// @param itemId Item id to wear.
            void wear(int equipSlot, int itemId);

            /// @brief Dresses the preview figure with the worn and tried items.
            void dress();

            /// @brief The rows of the page shown (empty rows skipped as the game does).
            /// @return The rows.
            std::vector<protocol::MallRow> pageRows() const;

            /// @brief Finds the row place under the mouse.
            /// @details Original: HitTestTryOnSlot.
            /// @return The place, 0 for none.
            int hoveredPlace() const;

            /// @brief Finds the game icon under the mouse.
            /// @details Original: HitTestShopSlot.
            /// @return The icon number, 0 for none.
            int hoveredGame() const;

            /// @brief Asks for the machine of the game icon pressed (75/4).
            /// @details Original: RequestShopCategory.
            void requestGame();

            /// @brief Paints the window's drawing area.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the catalog rows.
            /// @param canvas Target canvas.
            void paintRows(gfx::Canvas& canvas);

            /// @brief Paints the slot machines' game icons.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintGames(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the forging room.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintForge(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the try-on room with the figure and its attributes.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @param nowMs Current time in milliseconds.
            void paintTryOnRoom(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints the hovered row's description over everything.
            /// @details Original: RenderWithChildren.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paintDescription(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            MallShop& m_shop;
            const PlayerStatus& m_status;
            const MapCharacter& m_player;
            const Inventory& m_inventory;
            BagMarks& m_marks;
            PlayerMessages& m_messages;
            ItemInfoPopup& m_info;
            Hooks m_hooks;
            MallForge m_forge;
            ui::Window& m_window;
            ui::IconGrid& m_bag;
            ui::IconGrid& m_forgeCell;
            ui::ImageButton& m_forgeConfirm;
            ui::ImageButton& m_forgeCancel;
            ui::ImageButton& m_pageNext;
            ui::ImageButton& m_pagePrevious;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_descriptionSpot;
            const ui::Spot& m_gainSpot;
            const ui::Spot& m_lossSpot;
            std::array<std::reference_wrapper<ui::ImageButton>, 7> m_tabs;
            std::array<std::reference_wrapper<ui::ImageButton>, 8> m_addToCart;
            int m_tab = 1;
            bool m_gamesTab = false;
            int m_pose = 0;
            int m_direction = 13; // try-on facing 13 (standing pose directions run 8..15; other poses 16..17, 46..53)
            std::array<int, 7> m_base{};
            std::array<int, 7> m_tryOnBonus{};
            std::array<std::uint16_t, 6> m_tried{};
            std::array<std::uint16_t, 6> m_wearing{};
            CharacterLook m_look;
            bool m_tryOnCleared = true;
            int m_lastTried = 0;
            int m_hoverPack = 0;
            static constexpr int kGameIcons = 8;
            FigureAnimation m_animation;
            std::optional<gfx::PictureId> m_tryOnRoom;
            std::optional<gfx::PictureId> m_rowsRoom;
    };

}

#endif
