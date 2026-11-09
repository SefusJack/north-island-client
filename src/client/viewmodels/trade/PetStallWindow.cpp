#include "client/viewmodels/trade/PetStallWindow.h"

#include "client/rendering/painters/PetGear.h"
#include "client/rules/TextFilter.h"
#include "gfx/TextStyle.h"
#include "protocol/StallRequests.h"
#include "ui/DigitText.h"
#include "ui/DrawingArea.h"
#include "ui/IconGrid.h"

namespace client {

    namespace {

        constexpr int kPitch = 182; // horizontal distance between neighbouring stall places, px
        constexpr int kFacingSlot = 13; // NPC sprite action slot (facing direction) the pets are drawn with
        constexpr int kTeamPkMap1 = 12064; // map 12064: 2F, the Capitol (team PK, no stalls)
        constexpr int kTeamPkMap2 = 11073; // map 11073: 2F, Activity Centre (team PK, no stalls)
        constexpr int kNoTradeFlag = 2; // item flag bit 2: the item cannot be traded
        constexpr int kOverlayAlpha = 80; // alpha of the coloured overlays (0..255)
        const core::Color kInStallColor{108, 108, 1, kOverlayAlpha}; // r, g, b, alpha: dark yellow, pet already in the stall
        const core::Color kMarkedColor{0, 255, 0, kOverlayAlpha}; // r, g, b, alpha: green, marked place
        const core::Color kHoverColor{255, 108, 1, kOverlayAlpha}; // r, g, b, alpha: orange, place under the mouse
        const char* const kDefaultName = "Personal shop";
        const char* const kRevokeFirst = "Revoking stall before moving pet";

        /// @brief Loads PetStallWindow.xml into the screen and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The loaded PetStall window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PetStallWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("PetStall");
        }

        /// @brief Reads the place number from a "PetStallSlotN" element name.
        /// @param name Element name.
        /// @return The place number, or 0 when the name does not match.
        int placeOf(const std::string& name) {
            const std::string prefix = "PetStallSlot";
            return name.rfind(prefix, 0) == 0 ? std::stoi(name.substr(prefix.size())) : 0;
        }

        /// @brief The area of a stall place in the window.
        /// @param at Top-left of the drawing area.
        /// @param place Stall place (1..3).
        /// @return The place's rectangle.
        core::Rect placeArea(core::Point at, int place) {
            return {at.x + 222 + (place - 1) * kPitch, at.y + 71, 167, 203};
        } // first place's rectangle: x 222, y 71, 167 x 203

    }

    PetStallWindow::PetStallWindow(
        GameServices& services,
        ui::Screen& screen,
        const PetTeam& pets,
        const PlayerStatus& status,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_pets(pets),
          m_status(status),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_shopName(screen.get<ui::TextBox>("PetStallShopName")),
          m_petName(screen.get<ui::TextBox>("PetStallPetName")),
          m_textSpot(screen.get<ui::Spot>("PetStallText")),
          m_fixedNameSpot(screen.get<ui::Spot>("PetStallFixedName")) {
        screen.get<ui::DrawingArea>("PetStallPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paint(canvas, at, m_screen.mouse(), nowMs);
        });
        screen.on("onPetStallNext", [this](const ui::EventArgs&) { selectNext(); });
        screen.on("onPetStallPrevious", [this](const ui::EventArgs&) { selectPrevious(); });
        screen.on("onPetStallAdd", [this](const ui::EventArgs&) { add(); });
        screen.on("onPetStallRemove", [this](const ui::EventArgs&) { removeMarked(); });
        screen.on("onPetStallClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onPetStallKeep", [this](const ui::EventArgs&) { keep(); });
        screen.on("onPetStallRevoke", [this](const ui::EventArgs&) { revoke(); });
        screen.on("onPetStallSlotClick", [this](const ui::EventArgs& args) {
            Place& place = m_places[static_cast<std::size_t>(placeOf(args.source))];
            if (place.npcId != 0) {
                place.marked = !place.marked;
            }
        });
        m_shopName.setText(kDefaultName);
        for (int place = 1; place <= kPlaces; ++place) {
            price(place).setText("0");
        }
    }

    ui::TextBox& PetStallWindow::price(int place) { return m_screen.get<ui::TextBox>("PetStallPrice" + std::to_string(place)); }

    std::int32_t PetStallWindow::priceOf(int place) {
        const std::string& text = price(place).text();
        return text.empty() ? 0 : static_cast<std::int32_t>(std::stol(text));
    }

    void PetStallWindow::toggle() {
        if (isOpen()) {
            hide();
        } else {
            open();
        }
    }

    void PetStallWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        if (!m_open) {
            if (m_pets.count() <= 0) {
                m_messages.show("You have no pet and unable to keep stall\xA1I", 2000); // message display time in ms
                hide();
                return;
            }
            m_count = 0;
            m_shopName.setText(m_lastName.empty() ? kDefaultName : m_lastName);
        }
        if (m_pets.count() > 0) {
            m_order = 0;
            selectNext();
        }
    }

    void PetStallWindow::selectNext() {
        const int count = m_pets.count();
        if (count <= 0) {
            return;
        }
        ++m_order;
        if (count < m_order) {
            m_order = 0;
            m_selected = 0;
            m_petName.setText("");
            selectNext();
            return;
        }
        m_selected = m_pets.placeByOrder(m_order);
        if (m_selected == 0) {
            selectNext();
            return;
        }
        showSelected();
    }

    void PetStallWindow::selectPrevious() {
        const int count = m_pets.count();
        if (count <= 0) {
            return;
        }
        m_order = m_order == 0 ? count : m_order - 1;
        if (m_order == 0) {
            selectPrevious();
            return;
        }
        m_selected = m_pets.placeByOrder(m_order);
        if (m_selected == 0) {
            selectPrevious();
            return;
        }
        showSelected();
    }

    void PetStallWindow::showSelected() {
        const auto& pet = m_pets.at(m_selected);
        if (pet) {
            m_petName.setFontColor(pet->tradable == 0 ? m_fixedNameSpot.color() : m_textSpot.color());
            m_petName.setText(pet->name.empty() ? pet->info.name : pet->name);
        }
        refresh();
    }

    void PetStallWindow::refresh() {
        const auto& pet = m_pets.at(m_selected);
        showPetGear(
            m_services,
            m_screen.get<ui::IconGrid>("PetStallGear"),
            m_screen.get<ui::IconGrid>("PetStallGear6"),
            pet ? pet->equipmentIds() : std::vector<int>{}
        );
        for (int place = 1; place <= kPlaces; ++place) {
            const Place& entry = m_places[static_cast<std::size_t>(place)];
            const auto& sold = m_pets.at(entry.petPlace);
            const std::string name = "PetStallSlot" + std::to_string(place);
            showPetGear(
                m_services,
                m_screen.get<ui::IconGrid>(name + "Gear"),
                m_screen.get<ui::IconGrid>(name + "Gear6"),
                entry.npcId != 0 && sold ? sold->equipmentIds() : std::vector<int>{}
            );
            m_screen.get<ui::TextBox>("PetStallName" + std::to_string(place)).setText(entry.npcId != 0 && sold ? sold->info.name : "");
        }
    }

    void PetStallWindow::add() {
        const auto& pet = m_pets.at(m_selected);
        if (!pet) {
            return;
        }
        if (pet->info.growKey != 0) {
            m_messages.show("This pet cannot be sold", 1200); // message display time in ms
            return;
        }
        if (pet->amity == 0) {
            m_messages.show("Sell pet failed with 0 degree of amity", 2000); // message display time in ms
            return;
        }
        if (m_pets.ridingPlace() != 0 && m_pets.ridingPlace() == m_order) {
            m_messages.show("Ride pet unavailable for sale", 2000); // message display time in ms
            return;
        }
        if (m_pets.activePlace() == m_selected) {
            m_messages.show("Fighting Pet unavailable for sale", 2000); // message display time in ms
            return;
        }
        if (m_open) {
            m_messages.show(kRevokeFirst, 2000); // message display time in ms
            return;
        }
        if (m_order == 0) {
            m_messages.show("Select pets to sell first", 2000); // message display time in ms
            return;
        }
        if (m_count == kPlaces) {
            m_messages.show("No enough space for you to sell\xA1I", 2000); // message display time in ms
            return;
        }
        if (pet->info.mercenary != 0) {
            m_messages.show("The trade of soliders can't be transferred. ", 2000); // message display time in ms
            return;
        }
        for (const int itemId : pet->equipmentIds()) {
            const auto record = m_services.items.find(itemId);
            if (record && (record->flags & kNoTradeFlag) != 0) {
                m_messages.show("Gear Column[ " + record->name + " ]Unable to trade the untranferred item", 2000); // message display time in ms
                return;
            }
        }
        for (int place = 1; place <= kPlaces; ++place) {
            Place& entry = m_places[static_cast<std::size_t>(place)];
            if (entry.npcId != 0 || m_inStall[static_cast<std::size_t>(m_selected)]) {
                continue;
            }
            ++m_count;
            m_inStall[static_cast<std::size_t>(m_selected)] = true;
            entry = {m_selected, pet->info.npcId, false};
            price(place).setText("0");
            refresh();
            return;
        }
    }

    void PetStallWindow::emptyPlace(int place) {
        Place& entry = m_places[static_cast<std::size_t>(place)];
        m_inStall[static_cast<std::size_t>(entry.petPlace)] = false;
        entry = {};
        --m_count;
        price(place).setText("0");
    }

    void PetStallWindow::removeMarked() {
        if (m_open) {
            m_messages.show(kRevokeFirst, 2000); // message display time in ms
            return;
        }
        for (int place = 1; place <= kPlaces; ++place) {
            if (m_places[static_cast<std::size_t>(place)].marked) {
                emptyPlace(place);
            }
        }
        refresh();
    }

    void PetStallWindow::clearPlaces() {
        for (int place = 1; place <= kPlaces; ++place) {
            emptyPlace(place);
            price(place).setReadOnly(false);
        }
        m_open = false;
        refresh();
    }

    void PetStallWindow::close() {
        if (!m_open) {
            clearPlaces();
        }
        hide();
    }

    void PetStallWindow::keep() {
        const int map = m_hooks.mapId();
        if (map == kTeamPkMap1 || map == kTeamPkMap2) {
            m_messages.show("Team PK, cannot set stall", 2000); // message display time in ms
            return;
        }
        if (m_hooks.remoteOn()) {
            m_messages.show("Operation unavailable while auto remote controller still in use", 2000); // message display time in ms
            return;
        }
        if (m_hooks.fishing()) {
            m_messages.show("Unable to set a stall while fishing", 2000); // message display time in ms
            return;
        }
        if (m_hooks.inTeam()) {
            m_messages.show("Unable to keep stall while in group", 2000); // message display time in ms
            return;
        }
        if (m_hooks.keepsAnyStall()) {
            m_messages.show("One kind of stall every time", 2000); // message display time in ms
            return;
        }
        if (m_pets.ridingPlace() != 0) {
            m_messages.show("Riding, the action has been cancelled", 2000); // message display time in ms
            return;
        }
        if (m_hooks.usesVehicle()) {
            m_messages.show("Put away transport vehicle before opening stall\xA1I", 2000); // message display time in ms
            return;
        }
        if (m_shopName.text().empty()) {
            m_messages.show("Please enter the name of the shop", 1200); // message display time in ms
            return;
        }
        if (!isNameClean(m_shopName.text(), false)) {
            m_messages.show(kIllegalWords, 2000); // message display time in ms
            return;
        }
        if (m_count <= 0) {
            m_messages.show("Sell at least 1 pet before keeping stall", 2000); // message display time in ms
            return;
        }
        std::vector<protocol::OwnStall::Price> prices;
        for (int place = 1; place <= kPlaces; ++place) {
            const Place& entry = m_places[static_cast<std::size_t>(place)];
            if (entry.npcId != 0) {
                if (priceOf(place) == 0) {
                    m_messages.show("The amount should not be zero", 2000); // message display time in ms
                    return;
                }
                const auto npc = m_services.npcs.find(entry.npcId);
                if (npc && npc->mercenary != 0) {
                    m_messages.show("The trade of soliders can't be transferred. ", 2000); // message display time in ms
                    return;
                }
                prices.push_back({entry.petPlace, priceOf(place)});
            } else if (priceOf(place) != 0) {
                price(place).setText("0");
                return;
            }
        }
        m_services.connection.send(protocol::openPetStallRequest(m_shopName.text(), prices));
        m_open = true;
        hide();
        m_lastName = m_shopName.text();
    }

    void PetStallWindow::revoke() {
        if (!m_hooks.keepsPetStall()) {
            return;
        }
        m_services.connection.send(protocol::closePetStallRequest());
        hide();
        m_open = false;
        clearPlaces();
        m_lastName = m_shopName.text();
    }

    void PetStallWindow::stallOpened() {
        for (int place = 1; place <= kPlaces; ++place) {
            price(place).setReadOnly(true);
        }
        hide();
    }

    void PetStallWindow::showSale(const protocol::StallSale& sale) {
        for (int place = 1; place <= kPlaces; ++place) {
            if (m_places[static_cast<std::size_t>(place)].npcId == sale.itemId) {
                emptyPlace(place);
                --m_count;
            }
        }
        const auto npc = m_services.npcs.find(sale.itemId);
        // sale kind 2: a recharge card was sold
        m_hooks.chatLine(sale.kind == 2 ? "Sell Recharge Card of Wonderland Online" : "Sell " + (npc ? npc->name : std::string()));
        m_income += sale.money;
        refresh();
    }

    void PetStallWindow::paint(gfx::Canvas& canvas, core::Point at, core::Point mouse, std::uint64_t nowMs) {
        if (const auto heart = m_services.pictures.find("Bmp_Friend_1")) {
            // positions of the amity hearts: chosen pet, then the three stall places
            canvas.drawPicture(*heart, {at.x + 22, at.y + 278});
            canvas.drawPicture(*heart, {at.x + 222, at.y + 277});
            canvas.drawPicture(*heart, {at.x + 406, at.y + 278});
            canvas.drawPicture(*heart, {at.x + 586, at.y + 278});
        }
        m_screen.get<ui::Element>("PetStallKeep").setVisible(!m_open);
        m_screen.get<ui::Element>("PetStallRevoke").setVisible(m_open);
        const bool choosing = !m_open && m_order != 0;
        for (const char* name : {"PetStallNext", "PetStallPrevious", "PetStallGear", "PetStallGear6", "PetStallPetName"}) {
            m_screen.get<ui::Element>(name).setVisible(!m_open);
        }
        const gfx::TextStyle style{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const auto number = [&](core::Point where, const std::string& value) {
            canvas.drawText(at + where, value, style, static_cast<int>(value.size()) * 8, 12); // 8: pixels a digit, 12: text box height
        };
        const auto drawPet = [&](const formats::NpcInfo& npc, core::Point feet, int index) {
            const int slot = m_services.characters.npcSlot(npc.spriteId, npc.directionKind, kFacingSlot);
            const int frame = m_animations[static_cast<std::size_t>(index)].frame(nowMs, m_services.characters.npcFrameCount(npc.spriteId, slot));
            m_services.characters.drawNpc(canvas, npc, at + feet, slot, frame);
        };
        const auto shadow = m_services.pictures.find("PetShadow");
        if (choosing) {
            if (const auto& pet = m_pets.at(m_selected)) {
                // chosen pet: shadow at (75, 200), in-stall overlay rectangle (22, 71, 167 x 203), feet at (110, 205), level at (50, 242), hp at (50, 259), amity at (72, 278)
                if (shadow) {
                    canvas.drawPicture(*shadow, {at.x + 75, at.y + 200});
                }
                if (m_inStall[static_cast<std::size_t>(m_selected)]) {
                    canvas.fillRectAlpha({at.x + 22, at.y + 71, 167, 203}, kInStallColor);
                }
                drawPet(pet->info, {110, 205}, 0);
                number({50, 242}, std::to_string(pet->level));
                number({50, 259}, std::to_string(pet->hp) + "/" + std::to_string(pet->maxHp));
                number({72, 278}, std::to_string(pet->amity));
            }
        }
        for (int place = 1; place <= kPlaces; ++place) {
            const Place& entry = m_places[static_cast<std::size_t>(place)];
            const auto& pet = m_pets.at(entry.petPlace);
            if (entry.npcId == 0 || !pet) {
                continue;
            }
            const int dx = (place - 1) * kPitch;
            // stall place: shadow at (285 + dx, 200), feet at (308 + dx, 205), level at (250 + dx, 242), hp at (250 + dx, 259), amity at (272 + dx, 276)
            if (shadow) {
                canvas.drawPicture(*shadow, {at.x + 285 + dx, at.y + 200});
            }
            drawPet(pet->info, {308 + dx, 205}, place);
            if (entry.marked) {
                canvas.fillRectAlpha(placeArea(at, place), kMarkedColor);
            }
            number({250 + dx, 242}, std::to_string(pet->level));
            number({250 + dx, 259}, std::to_string(pet->hp) + "/" + std::to_string(pet->maxHp));
            number({272 + dx, 276}, std::to_string(pet->amity));
        }
        for (int place = 1; place <= kPlaces; ++place) {
            const Place& entry = m_places[static_cast<std::size_t>(place)];
            const core::Rect area = placeArea(at, place);
            if (area.contains(mouse) && !entry.marked && entry.npcId != 0) {
                canvas.fillRectAlpha(area, kHoverColor);
            }
        }
        m_screen.get<ui::DigitText>("PetStallGold").setText(std::to_string(m_status.gold));
        m_screen.get<ui::DigitText>("PetStallIncome").setText(std::to_string(m_income));
    }

}
