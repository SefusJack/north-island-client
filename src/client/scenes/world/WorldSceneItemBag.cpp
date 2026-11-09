#include "client/scenes/world/WorldScene.h"

namespace client {

    namespace {

        /// @brief Formats the remaining time of the Space Contract as days and hours.
        /// @param hours Remaining time in hours.
        /// @return Text like "2 Day 3 Hour ", or "In one hour" when less is left.
        std::string timeText(std::int32_t hours) {
            const int days = hours / 24; // hours per day
            const int rest = hours % 24; // hours per day
            std::string text;
            if (days > 0) {
                text = std::to_string(days) + " Day ";
            }
            if (rest > 0) {
                text += std::to_string(rest) + " Hour ";
            }
            if (days <= 0 && rest <= 0) {
                text = "In one hour";
            }
            return text;
        }

    }

    ItemBagWindow::Hooks WorldScene::itemBagHooks() {
        ItemBagWindow::Hooks hooks;
        hooks.takeFromBag = [this](int slot, int count) {
            m_inventory.takeFromBag(slot, count);
            inventoryChanged();
        };
        hooks.addToBag = [this](int slot, const protocol::ItemInstance& item) {
            m_inventory.addToBag(slot, item);
            m_remote.bagFilled();
            inventoryChanged();
        };
        hooks.closed = [this] { m_eventRunner.moveOn(); };
        return hooks;
    }

    void WorldScene::onSpaceContract(int kind, int flags, std::int32_t hours) {
        const auto say = [this](const std::string& line) { m_chat.add(0, "(System Prompt):" + line); };
        switch (kind) {
            case 1: // 23/136 kind 1: the Space Contract is on
                m_itemBag.setSpaceContract(true);
                break;
            case 2: // 23/136 kind 2: the Space Contract is off
                m_itemBag.setSpaceContract(false);
                break;
            case 3: // 23/136 kind 3: the pet restrain is hidden
                m_petStorage.setRestrainHidden(true);
                break;
            case 4: // 23/136 kind 4: the pet restrain is shown
                m_petStorage.setRestrainHidden(false);
                break;
            case 5: { // 23/136 kind 5: the extra spaces that are open (flag bits 4 Tent, 2 Hotel, 1 Stock Keeper, else Garage) and the time left
                std::string info;
                const auto add = [&info](const char* name) { info += (info.empty() ? "" : ",") + std::string(name); };
                if (flags - 4 >= 0) { // flag bit value 4: Tent
                    info = "Tent";
                    flags -= 4;
                }
                if (flags - 2 >= 0) { // flag bit value 2: Hotel
                    add("Hotel");
                    flags -= 2;
                }
                if (flags - 1 >= 0) {
                    add("Stock Keeper");
                    flags -= 1;
                }
                if (flags != 0) {
                    add("Garage");
                }
                say(info.empty() ? "Time accumulated Effectivie time:" + timeText(hours)
                                 : "Extra space: (" + info + ") is open Effectivie time:" + timeText(hours));
                break;
            }
            case 6: // 23/136 kind 6: the bonus time is over
                say("The time of the bonus value is over, the space has been taken back");
                break;
            case 7: // 23/136 kind 7: the time left of the Space Contract
                say("The left time for Space Contract (Time for it can be accumulated): " + timeText(hours));
                break;
            case 8: // 23/136 kind 8: the car case restrain mode 2
                m_carCase.restrain(2);
                break;
            case 9: // 23/136 kind 9: the car case restrain mode 1
                m_carCase.restrain(1);
                break;
            case 10: // 23/136 kind 10: the upper limit is reached
                say("Has reached the upper limit, can't be used");
                break;
            default: // other kinds are ignored
                break;
        }
    }

    void WorldScene::onItemBagMoved(int kind, int first, int second, int count) { m_itemBag.moved(kind, first, second, count); }

    void WorldScene::onItemBagList(const std::vector<protocol::BagItemRecord>& records) { m_itemBag.setList(records); }

    void WorldScene::onItemBagOpened() { m_itemBag.open(); }

}
