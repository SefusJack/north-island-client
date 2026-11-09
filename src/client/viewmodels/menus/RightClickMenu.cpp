#include "client/viewmodels/menus/RightClickMenu.h"

#include "client/rules/ItemInfo.h"

#include "ui/ImageButton.h"

namespace client {

    namespace {

        constexpr int kMenuWidth = 71; // width of the menu panel, px
        constexpr int kMenuHeight = 40; // height of the menu panel, px
        constexpr int kCellWidth = 32; // width of a bag cell, px (the menu opens to its right)
        constexpr int kMostFill = 2000000; // most HP/SP an HP or SP bottle can hold
        constexpr int kBagSlots = 50; // number of bag slots to search for potions
        constexpr int kStoreTankItem = 34399; // item 34399: Fuel Bank (opens the tank form instead of filling)

        /// @brief Loads RightClickMenu.xml and returns its root element.
        /// @param services Game services.
        /// @param screen Screen to load into.
        /// @return The "RightClickMenu" element.
        ui::Element& loadMenu(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("RightClickMenu.xml"), services.loadContext());
            return screen.get<ui::Element>("RightClickMenu");
        }

    }

    RightClickMenu::RightClickMenu(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        const PlayerStatus& status,
        PlayerMessages& messages,
        BagRequests& requests,
        std::function<void(int)> openTank
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_status(status),
          m_messages(messages),
          m_requests(requests),
          m_openTank(std::move(openTank)),
          m_panel(loadMenu(services, screen)) {
        screen.on("onRightClickPutIn", [this](const ui::EventArgs&) {
            if (m_forTank) {
                storeFiring();
            } else {
                putIn();
            }
        });
        screen.on("onRightClickCancel", [this](const ui::EventArgs&) { hide(); });
    }

    void RightClickMenu::open(int bagSlot, core::Point cell) {
        const auto item = m_services.items.find(m_inventory.bagItem(bagSlot).itemId);
        const int kind = item ? bottleKind(*item) : 0;
        const bool tank = m_inventory.bagItem(bagSlot).itemId == kStoreTankItem;
        // bottle kind 3 = HP Bottle, 4 = SP Bottle
        if (kind != 3 && kind != 4 && !tank) {
            m_bottleSlot = 0;
            hide();
            return;
        }
        m_bottleSlot = bagSlot;
        m_forTank = tank;
        m_screen.get<ui::ImageButton>("RightClickPutIn")
            .setPicture(m_services.pictures.find(tank ? "Btn_StoreFiring_1" : "Btn_MagicPutIn_1").value_or(gfx::PictureId{}));
        m_panel.setBounds({cell.x + kCellWidth, cell.y, kMenuWidth, kMenuHeight});
        m_panel.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_panel.id());
        }
    }

    void RightClickMenu::hide() {
        m_panel.setVisible(false);
        m_bottleSlot = 0;
    }

    void RightClickMenu::update() {
        if (!isOpen()) {
            return;
        }
        const core::Point mouse = m_screen.mouse();
        const core::Rect area = m_panel.bounds();
        if (mouse.x < area.x - kCellWidth || mouse.x > area.x + kMenuWidth || mouse.y < area.y || mouse.y > area.y + kMenuHeight) {
            hide();
        }
    }

    void RightClickMenu::storeFiring() {
        if (m_bottleSlot != 0 && m_inventory.bagItem(m_bottleSlot).itemId == kStoreTankItem) {
            m_openTank(m_bottleSlot);
        }
        hide();
    }

    void RightClickMenu::putIn() {
        if (m_bottleSlot == 0) {
            return;
        }
        const auto bottle = m_services.items.find(m_inventory.bagItem(m_bottleSlot).itemId);
        int lastKind = bottle ? bottleKind(*bottle) : 0;
        // bottle kind 3 (HP) fills potion kind 1, kind 4 (SP) fills potion kind 2
        const int fillKind = lastKind == 3 ? 1 : lastKind == 4 ? 2 : 0;
        int fill = lastKind == 3 ? m_status.hpBottle : m_status.spBottle;
        if (fillKind != 0 && fill >= kMostFill) {
            m_messages.show("The bottle is full", 2000); // message duration in ms
            return;
        }
        if (fillKind != 0) {
            bool moved = false;
            for (int slot = 1; slot <= kBagSlots; ++slot) {
                const auto item = m_services.items.find(m_inventory.bagItem(slot).itemId);
                if (!item) {
                    continue;
                }
                lastKind = bottleKind(*item);
                if (lastKind != fillKind) {
                    continue;
                }
                if (fill >= kMostFill) {
                    moved = true;
                    break;
                }
                // attributeValues[0] holds the potion amount plus an offset of 100
                const int amount = item->attributeValues[0] - 100 - m_inventory.bagItem(slot).usedAmount;
                m_requests.moveItem(slot, 1, m_bottleSlot);
                fill += amount;
                moved = true;
            }
            if (!moved) {
                if (lastKind == 3) { // bottle kind 3 = HP
                    m_messages.show("You don't have (a) HP potion", 1200); // message duration in ms
                } else if (lastKind == 4) { // bottle kind 4 = SP
                    m_messages.show("You don't have (a) SP potion", 1200); // message duration in ms
                }
            }
        }
        hide();
    }

}
