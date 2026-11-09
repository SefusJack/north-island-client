#include "client/viewmodels/menus/MoneyMenu.h"

#include "client/rules/PvpRankText.h"
#include "gfx/TextStyle.h"
#include "ui/DrawingArea.h"

#include <string>

namespace client {

    namespace {

        constexpr int kLeastLevel = 10; // minimum player level for money, trade and stall functions
        constexpr int kNoTradeMap = 10701; // map 10701: the jail (name not in the tables), no trading there
        constexpr int kTeamPkMap1 = 12064; // map 12064: 2F, the Capitol (team PK, no trading)
        constexpr int kTeamPkMap2 = 11073; // map 11073: 2F, Activity Centre (team PK, no trading)
        constexpr int kTradeItems = 9; // mouse mode 9: trade items
        constexpr int kTradePets = 10; // mouse mode 10: trade pets
        constexpr int kTradeFurniture = 19; // mouse mode 19: trade utensils (furniture)

        /// @brief Loads MoneyMenu.xml into the screen and returns its window.
        /// @param services Shared game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The "MoneyMenu" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MoneyMenu.xml"), services.loadContext());
            return screen.get<ui::Window>("MoneyMenu");
        }

    }

    MoneyMenu::MoneyMenu(
        GameServices& services,
        ui::Screen& screen,
        const PlayerStatus& status,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_screen(screen),
          m_status(status),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_goldSpot(screen.get<ui::Spot>("MoneyGoldColor")) {
        screen.get<ui::DrawingArea>("MoneyGold").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            const std::string gold = std::to_string(m_status.gold);
            // gold text at (70, 70); box width 8 px per digit, height 12
            canvas.drawText({at.x + 70, at.y + 70}, gold, {m_goldSpot.color(), gfx::TextEffect::Plain, std::nullopt}, static_cast<int>(gold.size()) * 8, 12);
        });
        screen.on("onMoneyClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onMoneyDiscard", [this](const ui::EventArgs&) { placeMoney(); });
        screen.on("onMoneyTradeItem", [this](const ui::EventArgs&) { trade(kTradeItems); });
        screen.on("onMoneyTradePet", [this](const ui::EventArgs&) { trade(kTradePets); });
        screen.on("onMoneyTradeUtensil", [this](const ui::EventArgs&) { trade(kTradeFurniture); });
        screen.on("onMoneyStallItem", [this](const ui::EventArgs&) { stall(1); }); // stall kind 1: items
        screen.on("onMoneyStallPet", [this](const ui::EventArgs&) { stall(2); }); // stall kind 2: pets
        screen.on("onMoneyStallUtensil", [this](const ui::EventArgs&) { stall(3); }); // stall kind 3: furniture
    }

    void MoneyMenu::toggle() {
        m_window.setVisible(!m_window.isVisible());
        if (m_window.isVisible() && m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    bool MoneyMenu::levelAndTimeAllow() {
        if (m_status.level < kLeastLevel) {
            m_messages.show("Level 10 is required", 2000); // message display time in ms
            return false;
        }
        if (!m_enoughOnlineTime) {
            m_messages.show("Not enough time online, cannot use this function", 2000); // message display time in ms
            return false;
        }
        return true;
    }

    void MoneyMenu::placeMoney() {
        if (!levelAndTimeAllow()) {
            return;
        }
        if (m_hooks.mapId() == kNoTradeMap) {
            m_messages.show("Unable to use the function here.", 1200); // message display time in ms
            return;
        }
        m_hooks.placeMoney();
        close();
    }

    void MoneyMenu::trade(int mode) {
        if (!levelAndTimeAllow()) {
            return;
        }
        const int map = m_hooks.mapId();
        if (map == kTeamPkMap1 || map == kTeamPkMap2) {
            m_messages.show("Team PK, cannot trade", 2000); // message display time in ms
            return;
        }
        if (m_hooks.onTrojanWarMap()) {
            m_messages.show("This is the field for the Trojan War event, cannot trade", 2000); // message display time in ms
            return;
        }
        if (m_hooks.onQuizZoneMap() || isPvpMap(map)) {
            m_messages.show("Can't use the function in this scene", 1200); // message display time in ms
            return;
        }
        if (map == kNoTradeMap) {
            m_messages.show("Unable to use the function here.", 1200); // message display time in ms
            return;
        }
        if (m_hooks.stallKept() != 0) {
            m_messages.show("Operation canceled because you are keeping a stall", 2000); // message display time in ms
            return;
        }
        m_hooks.pickTradePartner(mode);
        close();
    }

    void MoneyMenu::stall(int kind) {
        if (!m_hooks.onFlatGround()) {
            m_messages.show("It's not flat, cannot set up a stall", 1500); // message display time in ms
            return;
        }
        if (!levelAndTimeAllow()) {
            return;
        }
        if (m_hooks.noStallScene()) {
            m_messages.show("Unable to keep a stall in this scenario", 2000); // message display time in ms
            return;
        }
        const int map = m_hooks.mapId();
        if (map == kTeamPkMap1 || map == kTeamPkMap2) {
            m_messages.show("Team PK, cannot set stall", 2000); // message display time in ms
            return;
        }
        if (m_hooks.onTrojanWarMap()) {
            m_messages.show("This is the field for the Trojan War event, cannot make a stall", 2000); // message display time in ms
            return;
        }
        if (m_hooks.onQuizZoneMap() || isPvpMap(map)) {
            m_messages.show("Can't use the function in this scene", 1200); // message display time in ms
            return;
        }
        if (m_hooks.bathing()) {
            m_messages.show("Unable to set a stall while taking a bath", 2000); // message display time in ms
            return;
        }
        if (m_hooks.inTeam()) {
            m_messages.show("Unable to keep stall while in group", 2000); // message display time in ms
            return;
        }
        const int kept = m_hooks.stallKept();
        if (kept != 0 && kept != kind) {
            m_messages.show("One kind of stall every time", 2000); // message display time in ms
            return;
        }
        if (map == kNoTradeMap && kind != 3) { // stall kind 3 (furniture) is allowed on the no-trade map
            m_messages.show("Unable to use the function here.", 1200); // message display time in ms
            return;
        }
        m_hooks.openStall(kind);
        close();
    }

}
