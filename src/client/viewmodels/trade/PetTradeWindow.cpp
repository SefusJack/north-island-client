#include "client/viewmodels/trade/PetTradeWindow.h"

#include "client/rules/PriceColors.h"
#include "client/rendering/painters/PetGear.h"
#include "gfx/TextStyle.h"
#include "protocol/TradeRequests.h"
#include "ui/DrawingArea.h"

#include <string>

namespace client {

    namespace {

        constexpr int kAgree = 1; // trade answer code 1: agree
        constexpr int kCancel = 2; // trade answer code 2: cancel
        constexpr int kFacingSlot = 17; // NPC sprite action slot (facing direction) the pets are drawn with
        constexpr int kRebornNpcType = 32; // pet type 32: reborn pet (cannot be traded)
        constexpr int kNoTradeFlag = 2; // item flag bit 2: the item cannot be traded

        /// @brief Loads PetTradeWindow.xml into the screen and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The loaded PetTrade window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PetTradeWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("PetTrade");
        }

    }

    PetTradeWindow::PetTradeWindow(
        GameServices& services,
        ui::Screen& screen,
        const PetTeam& pets,
        const PlayerStatus& status,
        PlayerMessages& messages,
        PetInfoPopup& info
    )
        : m_services(services),
          m_screen(screen),
          m_pets(pets),
          m_status(status),
          m_messages(messages),
          m_info(info),
          m_window(loadWindow(services, screen)),
          m_giveGold(screen.get<ui::TextBox>("PetTradeGiveGold")),
          m_partnerGold(screen.get<ui::TextBox>("PetTradePartnerGold")),
          m_myPetName(screen.get<ui::TextBox>("PetTradeMyPetName")),
          m_textSpot(screen.get<ui::Spot>("PetTradeText")),
          m_fixedNameSpot(screen.get<ui::Spot>("PetTradeFixedName")) {
        screen.get<ui::DrawingArea>("PetTradePaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onPetTradeCancel", [this](const ui::EventArgs&) { cancel(); });
        screen.on("onPetTradeConfirm", [this](const ui::EventArgs&) { confirmOffer(); });
        screen.on("onPetTradeTrade", [this](const ui::EventArgs&) { agree(); });
        screen.on("onPetTradeNext", [this](const ui::EventArgs&) { choose(1); });
        screen.on("onPetTradePrevious", [this](const ui::EventArgs&) { choose(-1); });
    }

    void PetTradeWindow::open(const std::string& ownName, const std::string& partnerName) {
        m_screen.get<ui::TextBox>("PetTradeMyName").setText(ownName);
        m_screen.get<ui::TextBox>("PetTradePartnerName").setText(partnerName);
        m_myPetName.setText("");
        m_screen.get<ui::TextBox>("PetTradePartnerPetName").setText("");
        m_giveGold.setText("0");
        m_partnerGold.setText("0");
        m_partnerGold.setFontColor(priceColor(0));
        m_offerConfirmed = false;
        m_agreed = false;
        m_place = 0;
        m_order = 0;
        m_partner.reset();
        showOwnPet();
        showPetGear(m_services, m_screen.get<ui::IconGrid>("PetTradePartnerGear"), m_screen.get<ui::IconGrid>("PetTradePartnerGear6"), {});
        layoutButtons();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void PetTradeWindow::hide() {
        m_window.setVisible(false);
        m_info.hide();
        m_infoShown = false;
        m_partner.reset();
    }

    void PetTradeWindow::cancel() {
        m_offerConfirmed = false;
        m_agreed = false;
        m_place = 0;
        m_services.connection.send(protocol::petTradeAnswerRequest(kCancel));
        hide();
    }

    void PetTradeWindow::layoutButtons() {
        ui::Element& confirm = m_screen.get<ui::Element>("PetTradeConfirm");
        confirm.setVisible(!m_offerConfirmed);
        m_screen.get<ui::Element>("PetTradeTrade").setVisible(m_partner.has_value() && !confirm.isVisible() && !m_agreed);
    }

    void PetTradeWindow::choose(int step) {
        if (m_offerConfirmed) {
            return;
        }
        const int count = m_pets.count();
        if (step > 0) {
            m_order = m_order + 1 > count ? 0 : m_order + 1;
        } else {
            m_order = m_order == 0 ? count : m_order - 1;
        }
        m_place = m_order != 0 ? m_pets.placeByOrder(m_order) : 0;
        if (m_place == 0) {
            m_order = 0;
        }
        showOwnPet();
    }

    void PetTradeWindow::showOwnPet() {
        const auto& pet = m_pets.at(m_place);
        if (!pet) {
            m_myPetName.setText("");
            showPetGear(m_services, m_screen.get<ui::IconGrid>("PetTradeMyGear"), m_screen.get<ui::IconGrid>("PetTradeMyGear6"), {});
            return;
        }
        m_myPetName.setFontColor(pet->tradable == 0 ? m_fixedNameSpot.color() : m_textSpot.color());
        m_myPetName.setText(pet->info.name);
        showPetGear(m_services, m_screen.get<ui::IconGrid>("PetTradeMyGear"), m_screen.get<ui::IconGrid>("PetTradeMyGear6"), pet->equipmentIds());
    }

    bool PetTradeWindow::petMayGo(bool offering) {
        const auto& pet = m_pets.at(m_place);
        if (!pet) {
            return true;
        }
        if (pet->info.growKey != 0) {
            m_messages.show("This pet cannot be exchanged", 1200); // message display time in ms
            return false;
        }
        if (offering) {
            if (pet->tradable == 0) {
                m_messages.show("Unable to trade the quest NPC", 1200); // message display time in ms
                return false;
            }
            if (m_pets.activePlace() == m_place) {
                m_messages.show("Fighting Pet unavailable for trade", 2000); // message display time in ms
                return false;
            }
            if (m_pets.cityBattleDead(m_place)) {
                m_messages.show("A dead pet cannot be traded during a city battle", 1200); // message display time in ms
                return false;
            }
            if (pet->info.type == kRebornNpcType) {
                m_messages.show("Unable to trade the reborn pet", 1200); // message display time in ms
                return false;
            }
            if (pet->info.mercenary != 0) {
                m_messages.show("The trade of soliders can't be transferred. ", 1200); // message display time in ms
                return false;
            }
            if (m_pets.ridingPlace() == m_place) {
                m_messages.show("Ride pet unavailable for trade", 1200); // message display time in ms
                return false;
            }
        }
        for (const int itemId : pet->equipmentIds()) {
            const auto record = m_services.items.find(itemId);
            if (record && (record->flags & kNoTradeFlag) != 0) {
                m_messages.show("Gear Column[ " + record->name + " ]Unable to trade the untranferred item", 2000); // message display time in ms
                return false;
            }
        }
        return true;
    }

    void PetTradeWindow::confirmOffer() {
        if (!petMayGo(true)) {
            return;
        }
        if (m_giveGold.text().empty()) {
            m_giveGold.setText("0");
        }
        const long long gold = std::stoll(m_giveGold.text());
        if (gold > m_status.gold) {
            m_messages.show("Not enough money", 1200); // message display time in ms
            return;
        }
        m_services.connection.send(protocol::petTradeOfferRequest(static_cast<std::int32_t>(gold), m_place));
        m_offerConfirmed = true;
        layoutButtons();
    }

    void PetTradeWindow::agree() {
        if (!petMayGo(false)) {
            return;
        }
        m_services.connection.send(protocol::petTradeAnswerRequest(kAgree));
        m_agreed = true;
        m_place = 0;
        layoutButtons();
    }

    void PetTradeWindow::showResult(int result) {
        // 3000 and 1000: message display time in ms
        switch (result) {
            case 0: m_messages.show("The system is busy now. Please trade later.", 3000); break; // 25/11 result 0: system busy
            case 1: m_messages.show("Unable to trade, the target is unable to connect to the server.", 3000); break; // 25/11 result 1: target not connected
            case 2: m_messages.show("Unable to trade, the target is busy now.", 3000); break; // 25/11 result 2: target busy
            case 3:
                m_messages.show(isOpen() ? "The target has cancelled the trade." : "Cancel the trade", 3000); // message display time in ms
                break; // 25/11 result 3: cancelled by the target
            case 4: m_messages.show("Trade successful.", 3000); break; // 25/11 result 4: trade done
            case 5: m_messages.show("Unable to trade because you are in different scenes.", 3000); break; // 25/11 result 5: different scenes
            case 6: m_messages.show("Trade has been cancelled due to insufficient funds.", 3000); break; // 25/11 result 6: insufficient funds
            case 7: m_messages.show("Trade has been cancelled because the pets are the same.", 3000); break; // 25/11 result 7: the pets are the same
            case 8: m_messages.show("Unable to trade while in battle.", 3000); break; // 25/11 result 8: in battle
            case 9: m_messages.show("Cancel the trade because of different pets info.", 3000); break; // 25/11 result 9: different pet info
            case 10: m_messages.show("Cancel the trade. You can take 4 pets at best.", 3000); break; // 25/11 result 10: more than 4 pets
            case 11: m_messages.show("Cancel the trade. Someone has the same pet.", 3000); break; // 25/11 result 11: someone has the same pet
            case 12: m_messages.show("0 amity with the pet, unable to trade", 3000); break; // 25/11 result 12: pet has 0 amity
            case 13: m_messages.show("Unable to trade cause the target closed the trade function", 3000); break; // 25/11 result 13: target closed trading
            case 14: m_messages.show("Cannot trade when making an item", 3000); break; // 25/11 result 14: making an item
            default: m_messages.show("The system has interuppted the trade.", 1000); break; // any other 25/11 result: trade interrupted
        }
        hide();
    }

    void PetTradeWindow::showPartnerOffer(const protocol::PetTradeOffer& offer) {
        m_partner = offer;
        const std::string gold = std::to_string(offer.gold);
        m_partnerGold.setText(gold);
        m_partnerGold.setFontColor(priceColor(gold.size()));
        const auto npc = m_services.npcs.find(offer.npcId);
        m_screen.get<ui::TextBox>("PetTradePartnerPetName").setText(npc ? npc->name : offer.name);
        showPetGear(m_services, m_screen.get<ui::IconGrid>("PetTradePartnerGear"), m_screen.get<ui::IconGrid>("PetTradePartnerGear6"), offer.equipment);
        layoutButtons();
    }

    void PetTradeWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (const auto frame = m_services.pictures.find("Bmp_Friend_1")) {
            // positions of the gear backgrounds (the partner's side, the own side)
            canvas.drawPicture(*frame, {at.x + 37, at.y + 286});
            canvas.drawPicture(*frame, {at.x + 247, at.y + 286});
        }
        bool overPartnerPet = false;
        const auto& pet = m_pets.at(m_place);
        if (pet) {
            paintPet(
                canvas,
                at,
                pet->info,
                {333, 210},
                {268, 250},
                pet->level,
                pet->hp,
                pet->maxHp,
                pet->amity
            ); // own pet: feet at (333, 210), numbers at (268, 250)
        }
        if (m_partner) {
            if (const auto npc = m_services.npcs.find(m_partner->npcId)) {
                // partner's pet: feet at (123, 210), numbers at (58, 250)
                paintPet(canvas, at, *npc, {123, 210}, {58, 250}, m_partner->level, m_partner->hp, m_partner->maxHp, m_partner->amity);
                const int slot = m_services.characters.npcSlot(npc->spriteId, npc->directionKind, kFacingSlot);
                const core::Point mouse = m_screen.mouse();
                // {123, 210}: the partner's pet feet position, as above
                if (m_partner->info && m_services.characters.npcContains(*npc, at + core::Point{123, 210}, slot, 0, mouse)) {
                    m_info.show(*m_partner->info, m_partner->maxHp, m_partner->maxSp, mouse);
                    overPartnerPet = true;
                }
            }
        }
        if (m_infoShown && !overPartnerPet) {
            m_info.hide();
        }
        m_infoShown = overPartnerPet;
    }

    void PetTradeWindow::paintPet(
        gfx::Canvas& canvas,
        core::Point at,
        const formats::NpcInfo& npc,
        core::Point feet,
        core::Point text,
        int level,
        std::int32_t hp,
        std::int32_t maxHp,
        int amity
    ) {
        const int slot = m_services.characters.npcSlot(npc.spriteId, npc.directionKind, kFacingSlot);
        m_services.characters.drawNpc(canvas, npc, at + feet, slot, 0);
        const gfx::TextStyle style{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const auto line = [&](core::Point where, const std::string& value) {
            canvas.drawText(at + where, value, style, static_cast<int>(value.size()) * 8, 12); // 8: pixels a digit, 12: text box height
        };
        line(text, std::to_string(level));
        // hp line is 18 pixels below the level; the amity line 35 below and 22 to the right
        line({text.x, text.y + 18}, std::to_string(hp) + "/" + std::to_string(maxHp));
        line({text.x + 22, text.y + 35}, std::to_string(amity));
    }

}
