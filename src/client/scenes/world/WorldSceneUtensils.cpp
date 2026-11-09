#include "client/scenes/world/WorldScene.h"

#include "client/rules/GmIds.h"
#include "protocol/UtensilStallRequests.h"
#include "protocol/UtensilTradeRequests.h"

namespace client {

    UtensilTradeWindow::Hooks WorldScene::utensilTradeHooks() {
        UtensilTradeWindow::Hooks hooks;
        hooks.ownName = [this] { return m_player.name; };
        hooks.ownStall = [this] { return m_player.stall.has_value() || m_itemStall.isOpen() || m_petStall.isOpen(); };
        hooks.partnerStall = [this](std::int32_t id) {
            const auto partner = m_others.find(id);
            return partner != m_others.end() && partner->second.stall.has_value();
        };
        hooks.onVehicle = [this] { return m_player.vehicle.has_value(); };
        hooks.sportActive = [this] { return m_activeSport != 0; };
        hooks.bagChanged = [this] { bagMarksChanged(); };
        return hooks;
    }

    void WorldScene::askToTradeUtensils(std::int32_t playerId) {
        constexpr int kLeastLevel = 10; // minimum player level to trade utensils
        constexpr int kWatchingFight = 4; // fight state 4: watching a battle
        const auto target = m_others.find(playerId);
        if (target == m_others.end()) {
            return;
        }
        if (target->second.fishing) {
            m_messages.show("The target is fishing", 2000); // message duration in ms
        } else if (m_activeSport != 0) {
            return;
        } else if (m_player.stall || m_itemStall.isOpen() || m_petStall.isOpen()) {
            m_messages.show("Operation canceled because you are keeping a stall", 2000); // message duration in ms
        } else if (target->second.stall) {
            m_messages.show("Operation canceled because the other party is keeping a stall", 2000); // message duration in ms
        } else if (target->second.fightState == kWatchingFight) {
            m_messages.show("Unable to trade, the target is watching combat.", 1500); // message duration in ms
        } else if (const auto record = m_otherRecords.find(playerId); record != m_otherRecords.end() && record->second.level < kLeastLevel) {
            m_messages.show("The player less than level 10", 1500); // message duration in ms
        } else if (isGmId(playerId)) {
            m_messages.show("Unable to trade with Cupid", 1200); // message duration in ms
        } else {
            m_services.connection.send(protocol::utensilTradeRequest(playerId));
        }
    }

    void WorldScene::onUtensilTradeStart(std::int32_t partnerId) {
        m_utensilTrade.open();
        if (const auto found = m_others.find(partnerId); found != m_others.end()) {
            m_utensilTrade.setPartner(partnerId, found->second.name);
        }
    }

    void WorldScene::onUtensilTradeOffer(const protocol::TradeOffer& offer) { m_utensilTrade.showPartnerOffer(offer); }

    void WorldScene::onUtensilTradeResult(int code) { m_utensilTrade.showResult(code); }

    UtensilStallWindow::Hooks WorldScene::utensilStallHooks() {
        UtensilStallWindow::Hooks hooks;
        hooks.mapId = [this] { return m_map ? m_map->id() : 0; };
        hooks.noStallScene = [this] { return (sceneFlags() & 2) != 0; }; // scene flag bit 2: no stalls in this scene
        hooks.inTeam = [this] { return m_teams.leaderOf(m_player.id).has_value(); };
        hooks.keepsUtensilStall = [this] { return m_player.stall && m_player.stall->kind == StallSign::Kind::Furniture; };
        hooks.keepsAnyStall = [this] { return m_player.stall.has_value(); };
        hooks.riding = [this] { return m_player.riding.has_value(); };
        hooks.chatLine = [this](const std::string& line) { m_chat.add(10, line, m_player.id); }; // chat channel 10
        hooks.usesVehicle = [this] { return m_player.vehicle.has_value(); };
        hooks.fishing = [this] { return m_player.fishing; };
        hooks.remoteOn = [this] { return m_remote.isOn(); };
        hooks.bagChanged = [this] { bagMarksChanged(); };
        return hooks;
    }

    void WorldScene::onOwnUtensilStall(const protocol::OwnStall& stall) {
        m_player.stall = StallSign{StallSign::Kind::Furniture, stall.name, stall.sign * 2 - 1}; // sign 2n - 1: the item stalls' picture for n pieces
        for (const protocol::OwnStall::Price& price : stall.prices) {
            m_bagMarks.setPriced(price.bagSlot, true);
        }
        m_utensilStall.stallOpened();
        bagMarksChanged();
    }

    void WorldScene::onUtensilStalls(const std::vector<protocol::StallSignRecord>& stalls) {
        for (const protocol::StallSignRecord& record : stalls) {
            const auto found = m_others.find(record.playerId);
            if (found != m_others.end()) {
                found->second.stall =
                    StallSign{StallSign::Kind::Furniture, record.name, record.sign * 2 - 1}; // sign 2n - 1: the item stalls' picture for n pieces
            }
        }
    }

    void WorldScene::onUtensilStallClosed(std::int32_t playerId) {
        if (playerId == m_player.id) {
            m_player.stall.reset();
            m_bagMarks.clearPriced();
            m_utensilStall.stallClosed();
            bagMarksChanged();
            return;
        }
        const auto found = m_others.find(playerId);
        if (found != m_others.end()) {
            found->second.stall.reset();
            if (m_buyUtensil.isOpen() && m_buyUtensil.ownerId() == playerId) {
                m_buyUtensil.close();
                m_messages.show("The target has pulled his stall", 2000); // message duration in ms
            }
        }
    }

    void WorldScene::onUtensilStallGoods(const protocol::UtensilStallGoods& goods) { m_buyUtensil.show(goods); }

    void WorldScene::onUtensilStallSale(const protocol::StallSale& sale) { m_utensilStall.showSale(sale); }

}
