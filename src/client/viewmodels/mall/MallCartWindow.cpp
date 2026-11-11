#include "client/viewmodels/mall/MallCartWindow.h"

#include "client/rules/ItemInfo.h"
#include "core/StringUtil.h"
#include "gfx/TextStyle.h"
#include "protocol/MallMessages.h"
#include "ui/DrawingArea.h"

#include <string>

namespace client {

    namespace {

        const core::Color kTextColor = core::Color::fromRgb565(2113); // RGB565 0x0841: near black
        constexpr int kMostTyped = 50; // most pieces of one item a player can type into the amount box

        /// @brief Loads MallCartWindow.xml into the screen and returns its window.
        /// @param services Shared game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The "Cart" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MallCartWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Cart");
        }

        /// @brief The plain text style in the form colour.
        /// @return The text style.
        gfx::TextStyle plain() { return {kTextColor, gfx::TextEffect::Plain, std::nullopt}; }

        /// @brief Item whose icon and info a cart row shows (the alias of a type 17 item, the item itself for type 26).
        /// @param items Item table.
        /// @param itemId Item id of the row.
        /// @return The item id to show.
        int shownItemId(const formats::ItemTable& items, int itemId) {
            const auto item = items.find(itemId);
            // item type 17 (boxes such as Angel Wings Box, with an alias item) shows its alias, type 26 (gold packs such as 100,000 Gold Pack) shows itself
            if (item && ((item->type == 17 && item->aliasItemId != 0) || item->type == 26)) {
                return item->type == 26 ? item->id : item->aliasItemId;
            }
            return itemId;
        }

        /// @brief Builds the attributes' text of a cart row.
        /// @details Original: THL_ShoppingCarForm.DrawControl.
        /// @param item Item whose attributes are described.
        /// @return The description line.
        std::string effectText(const formats::ItemRecord& item) {
            std::string text;
            for (std::size_t k = 0; k < item.attributeKinds.size(); ++k) {
                std::string stat;
                switch (item.attributeKinds[k]) {
                    case 25: stat = "HP"; break; // attribute kind 25: HP
                    case 26: stat = "SP"; break; // attribute kind 26: SP
                    case 207: stat = "MaxHp"; break; // attribute kind 207: max HP
                    case 208: stat = "MaxSp"; break; // attribute kind 208: max SP
                    case 210: stat = "ATK"; break; // attribute kind 210: attack
                    case 211: stat = "DEF"; break; // attribute kind 211: defence
                    case 215: stat = "MATK"; break; // attribute kind 215: magic attack
                    case 216: stat = "MDEF"; break; // attribute kind 216: magic defence
                    case 214: stat = "SPD"; break; // attribute kind 214: speed
                    case 64: stat = "Amity Degree"; break; // attribute kind 64: amity degree
                    case 36: stat = "Exp"; break; // attribute kind 36: experience
                    case 100: stat = "resistance of shoot"; break; // attribute kind 100: resistance of shoot
                    case 250: continue; // attribute kind 250: unused slot, skipped
                    default: break; // other kinds: no stat name
                }
                constexpr int kStink = 33053; // item 33053: Korean Cake (item type 24), thrown at enemies
                if (item.id == kStink || bottleKind(item) != 0) {
                    continue;
                }
                if (!text.empty() && !stat.empty()) {
                    text += ", ";
                }
                text += stat;
                const int value = item.attributeValues[k];
                // attribute values are stored offset by 100 (100 = no change)
                if (value == 100) {
                    continue;
                }
                text += value > 100 ? "+" + std::to_string(value - 100) : std::to_string(value - 100);
                // item subtype 113 with kind 36 (exp) and subtype 119 with kind 100 (resistance of shoot) are percentages
                if ((item.subType == 113 && item.attributeKinds[k] == 36) || (item.subType == 119 && item.attributeKinds[k] == 100)) {
                    text += "%";
                }
            }
            const auto pad = [&text](std::size_t to) {
                if (text.size() % to > 0) {
                    text += std::string(to - text.size() % to, ' ');
                }
            };
            // item subtype 137: critical strike chance 10% + 2% per rank; pad(22) aligns the text to a 22 character line
            if (item.subType == 137 && item.rank != 0) {
                if (!text.empty()) {
                    text += ", ";
                }
                pad(22);
                text += "Have" + std::to_string(item.rank * 2 + 10) + "%Critical strike chance";
            }
            if (item.id == 33052) { // item id 33052 (System Grenade): gets its own fixed description
                if (!text.empty()) {
                    text += ", ";
                }
                pad(22);
                text += "Can't be transfered\xA1"
                        "AHas no impact on players";
            }
            if (item.id == 33055) { // item id 33055 (Big Bowling): gets its own fixed description
                if (!text.empty()) {
                    text += ", ";
                }
                text += "Attack all enemies\xA1"
                        "ACan't be transfered\xA1"
                        "AHas no impact on players";
            }
            if (item.id == 33053) { // item id 33053 (Korean Cake): gets its own fixed description (kStink)
                if (!text.empty()) {
                    text += ", ";
                }
                pad(22);
                text += "Throw at enemies decreasing half of their HP\xA1"
                        "ACan't be transfered\xA1"
                        "AHas no impact on players";
            }
            if (bottleKind(item) != 0) {
                text = item.description;
            }
            return text;
        }

    }

    MallCartWindow::MallCartWindow(
        GameServices& services,
        ui::Screen& screen,
        MallShop& shop,
        const PlayerStatus& status,
        const Inventory& inventory,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_shop(shop),
          m_status(status),
          m_inventory(inventory),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_delete{
              std::ref(screen.get<ui::ImageButton>("CartDelete1")),
              std::ref(screen.get<ui::ImageButton>("CartDelete2")),
              std::ref(screen.get<ui::ImageButton>("CartDelete3")),
              std::ref(screen.get<ui::ImageButton>("CartDelete4")),
              std::ref(screen.get<ui::ImageButton>("CartDelete5")),
              std::ref(screen.get<ui::ImageButton>("CartDelete6")),
              std::ref(screen.get<ui::ImageButton>("CartDelete7"))
          },
          m_counts{
              std::ref(screen.get<ui::TextBox>("CartCount1")),
              std::ref(screen.get<ui::TextBox>("CartCount2")),
              std::ref(screen.get<ui::TextBox>("CartCount3")),
              std::ref(screen.get<ui::TextBox>("CartCount4")),
              std::ref(screen.get<ui::TextBox>("CartCount5")),
              std::ref(screen.get<ui::TextBox>("CartCount6")),
              std::ref(screen.get<ui::TextBox>("CartCount7"))
          } {
        screen.get<ui::DrawingArea>("CartPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onCartDelete", [this](const ui::EventArgs& args) {
            for (std::size_t i = 0; i < m_delete.size(); ++i) {
                if (args.source == m_delete[i].get().name()) {
                    remove(static_cast<int>(i) + 1);
                }
            }
        });
        screen.on("onCartReturn", [this](const ui::EventArgs&) {
            if (!m_hooks.fighting()) {
                hide();
                m_hooks.showMall();
            }
        });
        screen.on("onCartPay", [this](const ui::EventArgs&) { pay(); });
        screen.on("onCartExit", [this](const ui::EventArgs&) { hide(); });
    }

    void MallCartWindow::show() {
        if (m_hooks.fighting()) {
            hide();
            return;
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_hooks.hideMall();
        refreshCounts();
    }

    void MallCartWindow::toggle() {
        refreshCounts();
        if (m_window.isVisible()) {
            hide();
        } else {
            show();
        }
    }

    void MallCartWindow::update() {
        if (m_window.isVisible() && m_hooks.fighting()) {
            hide();
            return;
        }
        if (m_shop.purchaseDone) {
            refreshCounts();
            m_shop.purchaseDone = false;
        }
        if (m_window.isVisible()) {
            readCounts();
        }
    }

    void MallCartWindow::remove(int line) {
        if (m_hooks.fighting() || line < 1 || line > static_cast<int>(m_shop.cart().size())) {
            return;
        }
        const auto item = m_services.items.find(m_shop.cart()[static_cast<std::size_t>(line - 1)].row.itemId);
        m_messages.show("Add [" + (item ? item->name : std::string()) + "] Delete from cart", 1500); // message display time in ms
        m_shop.removeFromCart(line);
        refreshCounts();
    }

    void MallCartWindow::pay() {
        if (m_hooks.fighting()) {
            return;
        }
        if (!m_shop.paymentPending) {
            m_shop.paymentPending = true;
            m_services.connection.send(protocol::imPointsRequest(1));
        }
        hide();
    }

    void MallCartWindow::pointsArrived() {
        if (!m_shop.paymentPending) {
            return;
        }
        m_shop.paymentPending = false;
        if (m_shop.cartTotal() > m_shop.points) {
            m_messages.show("Unable to trade without enough points", 1500); // message display time in ms
            return;
        }
        if (!bagHasRoom()) {
            m_messages.show("Not enough space", 1500); // message display time in ms
            return;
        }
        std::vector<protocol::MallPurchaseLine> lines;
        for (const MallCartLine& line : m_shop.cart()) {
            if (line.count != 0) {
                lines.push_back({line.row.itemId, line.row.category, line.count, line.row.key});
            }
        }
        if (lines.empty()) {
            m_messages.show("The cart is empty", 1500); // message display time in ms
            return;
        }
        m_services.connection.send(protocol::mallBuyRequest(lines));
    }

    bool MallCartWindow::bagHasRoom() const {
        Inventory bag = m_inventory;
        for (const MallCartLine& line : m_shop.cart()) {
            const int times = line.row.hot == 4 ? 2 : 1; // mall mark 4 (special item) gives the goods twice
            for (int t = 0; t < times; ++t) {
                const int count = line.row.count * line.count;
                if (count <= 0) {
                    continue;
                }
                const int slot = bag.placeFor(line.row.itemId, count);
                if (slot == 0) {
                    return false;
                }
                bag.addToBag(slot, protocol::ItemInstance{line.row.itemId, count});
            }
        }
        return true;
    }

    void MallCartWindow::refreshCounts() {
        const std::vector<MallCartLine>& cart = m_shop.cart();
        for (std::size_t i = 0; i < m_counts.size(); ++i) {
            m_counts[i].get().setText(i < cart.size() ? std::to_string(cart[i].count) : std::string());
        }
    }

    void MallCartWindow::readCounts() {
        std::vector<MallCartLine>& cart = m_shop.cart();
        for (std::size_t i = 0; i < m_counts.size(); ++i) {
            ui::TextBox& box = m_counts[i].get();
            const bool used = i < cart.size();
            box.setVisible(used);
            m_delete[i].get().setVisible(used);
            if (!used) {
                box.setText({});
                continue;
            }
            MallCartLine& line = cart[i];
            // mall categories 1 and 2 (armour, weapons) and non-stacking items count one; categories 4 and 5 count packs
            if (line.row.category == 1 || line.row.category == 2 || !m_inventory.isStackable(line.row.itemId)) {
                box.setText("1");
                box.setReadOnly(true);
            } else if (line.row.category == 4 || line.row.category == 5) {
                box.setText(std::to_string(line.count * line.row.count));
                box.setReadOnly(true);
            } else {
                if (box.text().empty()) {
                    box.setText("0");
                }
                int typed = core::parseInt(box.text()).value_or(0);
                if (typed > kMostTyped) {
                    typed = kMostTyped;
                    box.setText(std::to_string(typed));
                }
                box.setReadOnly(false);
                line.count = typed;
            }
        }
    }

    void MallCartWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (const auto mark = m_services.pictures.find("Icon_ShopMark_2")) {
            canvas.drawPicture(*mark, {at.x + 18, at.y + 456}); // points icon position (18, 456)
        }
        const std::string total = std::to_string(m_shop.cartTotal());
        // total right-aligned in 8 digits of 8 px starting at x 438, y 464; box height 12
        canvas.drawText({at.x + 438 + (8 - static_cast<int>(total.size())) * 8, at.y + 464}, total, plain(), static_cast<int>(total.size()) * 8, 12);
        const std::vector<MallCartLine>& cart = m_shop.cart();
        for (std::size_t j = 0; j < cart.size(); ++j) {
            const MallCartLine& line = cart[j];
            const int left = 147; // x of the row's picture
            const int top = static_cast<int>(j) * 59 + 75; // rows are 59 px apart, the first at y 75
            const auto item = m_services.items.find(line.row.itemId);
            const auto shown = m_services.items.find(shownItemId(m_services.items, line.row.itemId));
            if (!item || !shown) {
                continue;
            }
            const bool typed = !(line.row.category == 1 || line.row.category == 2 || !m_inventory.isStackable(line.row.itemId)) && line.row.category != 4 &&
                               line.row.category != 5;
            if (typed) {
                if (const auto editor = m_services.pictures.find("Icon_InputEditor")) {
                    canvas.drawPicture(*editor, {left + 371, top + 5}); // input editor icon offset (371, 5) from the row
                }
            }
            // item icon is 32x32
            if (const auto icon = m_services.pictures.findStretched(std::to_string(item->iconId), 32, 32)) {
                canvas.drawPictureScaled(*icon, {left, top, 32, 32});
            }
            if (line.row.hot == 4) { // mall mark 4: special item
                if (const auto special = m_services.pictures.find("Icon_SpecItem")) {
                    canvas.drawPicture(*special, {left + 13, top + 10}); // special mark offset (13, 10) on the icon
                }
            }
            const std::string price = std::to_string(MallShop::linePrice(line.row) * line.count);
            // price right-aligned in 8 digits of 8 px (4 = half) at x +290, y +6; box height 12
            canvas.drawText({left + 290 + (8 - static_cast<int>(price.size())) * 4, top + 6}, price, plain(), static_cast<int>(price.size()) * 8, 12);
            std::string name = item->name;
            const auto padName = [&name] {
                // names are padded to a multiple of 8 characters when fewer than 6 are free in the last block
                const std::size_t rest = name.size() % 8;
                if (8 - rest < 6) {
                    name += std::string(8 - rest, ' ');
                }
            };
            if (line.row.category == 4) { // mall category 4: sold in packs
                padName();
                canvas.drawText({left, top + 29}, std::to_string(line.row.count), plain(), 64, 15); // pack size 29 px below the row top; box 64x15
            } else if (isMultiUse(*shown) && shown->maxUses > 0) {
                padName();
                name += "(" + std::to_string(shown->maxUses) + "Time)";
            }
            canvas.drawText({left + 39, top - 10}, name, plain(), 64, 15); // name at (+39, -10) from the row; box 64x15
            int extra = 0;
            if (shown->requiredLevel > m_status.level) {
                // level line at (+114, -10); box 176x15
                canvas.drawText({left + 114, top - 10}, "Required Level:" + std::to_string(shown->requiredLevel), plain(), 176, 15);
                extra = 1;
            }
            const auto meaningful = [](int kind) { return kind != 0 && kind != 250; }; // attribute kinds 0 and 250 mean no attribute
            std::string text = meaningful(shown->attributeKinds[0]) || meaningful(shown->attributeKinds[1]) ? effectText(*shown) : shown->description;
            const std::size_t most = static_cast<std::size_t>(66 - extra * 22); // at most 66 characters, 22 fewer when the level line takes a row
            if (text.size() > most) {
                text = text.substr(0, most);
            }
            canvas.drawText(
                {left + 114, top - 10 + extra * 15},
                text,
                plain(),
                176,
                15
            ); // description at (+114, -10), one 15 px line lower with the level line; box 176x15
        }
    }

}
