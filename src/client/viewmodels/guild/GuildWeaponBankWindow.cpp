#include "client/viewmodels/guild/GuildWeaponBankWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "gfx/TextStyle.h"
#include "protocol/GuildRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kMaterialType = 53; // item type 53: weapon materials the bank takes (Gold Eagle Totem, Stone Eagle Totem, Torch, Tsiji)
        // icon picture per weapon material kind 1..7 (original FWeaponIconPics[0..6]: icon_<36964, 36967, 36966, 36961, 36960, 36963, 36962>s)
        const char* const kKindIcons[GuildWeaponBankWindow::kKinds] =
            {"icon_36964s", "icon_36967s", "icon_36966s", "icon_36961s", "icon_36960s", "icon_36963s", "icon_36962s"};

        /// @brief Loads GuildWeaponBankWindow.xml into the screen and returns its window.
        /// @param services Game services (layouts, load context).
        /// @param screen Screen to load into.
        /// @return The loaded GuildWeaponBank window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("GuildWeaponBankWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("GuildWeaponBank");
        }

    }

    GuildWeaponBankWindow::GuildWeaponBankWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& bag,
        const std::array<std::int32_t, kKinds + 1>& weaponCounts
    )
        : m_services(services),
          m_screen(screen),
          m_bag(bag),
          m_weaponCounts(weaponCounts),
          m_window(loadWindow(services, screen)),
          m_items(screen.get<ui::IconGrid>("GuildWeaponBankItems")),
          m_textSpot(screen.get<ui::Spot>("GuildWeaponBankText")),
          m_chosenSpot(screen.get<ui::Spot>("GuildWeaponBankChosenCell")) {
        for (int kind = 0; kind < kKinds; ++kind) {
            ui::TextBox& box = screen.get<ui::TextBox>("GuildWeaponBankAmount" + std::to_string(kind));
            box.setText("0");
            m_amounts.push_back(box);
        }
        screen.get<ui::DrawingArea>("GuildWeaponBankPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.get<ui::DrawingArea>("GuildWeaponBankChosen").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            paintChosen(canvas, at);
        });
        screen.on("onGuildWeaponBankPress", [this](const ui::EventArgs& args) { press(args.index); });
        screen.on("onGuildWeaponBankDeposit", [this](const ui::EventArgs&) { deposit(); });
        screen.on("onGuildWeaponBankWithdraw", [this](const ui::EventArgs&) { withdraw(); });
        screen.on("onGuildWeaponBankClose", [this](const ui::EventArgs&) { hide(); });
    }

    void GuildWeaponBankWindow::toggle() {
        if (m_window.isVisible()) {
            hide();
            return;
        }
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    bool GuildWeaponBankWindow::isMaterial(int slot) const {
        const auto record = m_services.items.find(m_bag.bagItem(slot).itemId);
        return record && record->type == kMaterialType;
    }

    void GuildWeaponBankWindow::refresh() {
        m_items.clearCells();
        for (int slot = 1; slot <= kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_bag.bagItem(slot);
            if (item.empty()) {
                m_chosen[static_cast<std::size_t>(slot)] = false;
                continue;
            }
            ui::IconCell cell = itemNameCell(m_services, item);
            if (!isMaterial(slot)) {
                cell.count = 0;
                cell.hint.clear();
                m_chosen[static_cast<std::size_t>(slot)] = false;
            }
            m_items.setCell(slot, cell);
        }
    }

    void GuildWeaponBankWindow::press(int slot) {
        if (slot >= 1 && slot <= kBagSlots && isMaterial(slot)) {
            m_chosen[static_cast<std::size_t>(slot)] = !m_chosen[static_cast<std::size_t>(slot)];
        }
    }

    void GuildWeaponBankWindow::deposit() {
        std::vector<std::pair<int, int>> amounts;
        for (int kind = 0; kind < kKinds; ++kind) {
            ui::TextBox& box = m_amounts[static_cast<std::size_t>(kind)];
            const std::string& text = box.text();
            // at most 10 digits, value at most INT32_MAX; the amount goes as a 16-bit word (mask 0xFFFF)
            if (!text.empty() && text.size() <= 10 && text.find_first_not_of("0123456789") == std::string::npos && std::stoll(text) <= 2147483647) {
                amounts.emplace_back(kind + 1, static_cast<int>(std::stoll(text) & 0xFFFF));
            }
            box.setText("0");
        }
        if (!amounts.empty()) {
            m_services.connection.send(protocol::guildWeaponDepositRequest(amounts));
        }
    }

    void GuildWeaponBankWindow::withdraw() {
        std::vector<int> slots;
        for (int slot = 1; slot <= kWithdrawSlots; ++slot) {
            if (m_chosen[static_cast<std::size_t>(slot)]) {
                slots.push_back(slot);
            }
        }
        if (!slots.empty()) {
            m_services.connection.send(protocol::guildWeaponWithdrawRequest(slots));
        }
    }

    void GuildWeaponBankWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const auto draw = [&](const char* name, core::Point position) {
            if (const auto picture = m_services.pictures.find(name)) {
                canvas.drawPicture(*picture, {at.x + position.x, at.y + position.y});
            }
        };
        draw("icon_weaponry", {15, 7}); // pixel position of the title picture
        draw("icon_ItemGrids", {16, 66}); // pixel position of the bag grid picture
        draw("icon_withDraw", {16, 40}); // pixel position of the withdraw arrow picture
        const gfx::TextStyle style{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        for (int kind = 0; kind < kKinds; ++kind) {
            draw(kKindIcons[kind], {230, 52 + kind * 30}); // kind icon position; rows are 30 px apart
            draw("panel16", {262, 56 + kind * 30}); // count panel position; rows are 30 px apart
            // count text position (264, 58), row step 30 px; text box size 200 x 20; weaponCounts is indexed 1..7 by kind
            canvas.drawText({at.x + 264, at.y + 58 + kind * 30}, std::to_string(m_weaponCounts[static_cast<std::size_t>(kind + 1)]), style, 200, 20);
        }
    }

    void GuildWeaponBankWindow::paintChosen(gfx::Canvas& canvas, core::Point at) {
        for (int slot = 1; slot <= kBagSlots; ++slot) {
            if (m_chosen[static_cast<std::size_t>(slot)]) {
                const core::Point cell = m_items.bounds().position() + m_items.cellPosition(slot);
                canvas.fillRectAlpha({at.x + cell.x, at.y + cell.y, m_chosenSpot.size().width, m_chosenSpot.size().height}, m_chosenSpot.color());
            }
        }
    }

}
