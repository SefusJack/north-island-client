#include "client/viewmodels/trade/StallPetsWindow.h"

#include "client/rules/PriceColors.h"
#include "client/rendering/painters/PetGear.h"
#include "gfx/TextStyle.h"
#include "protocol/StallRequests.h"
#include "ui/DigitText.h"
#include "ui/DrawingArea.h"
#include "ui/IconGrid.h"
#include "ui/TextBox.h"

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    namespace {

        constexpr int kPitch = 203; // horizontal distance between neighbouring pet places, px
        constexpr int kFacingSlot = 13; // NPC sprite action slot (facing direction) the pets are drawn with
        constexpr int kOverlayAlpha = 80; // alpha of the coloured overlays (0..255)
        constexpr int kHighLevel = 86; // pet NPC level from which the level-gap rule applies
        constexpr int kLevelGap = 50; // a pet this many levels above the player cannot be bought
        const core::Color kMarkedColor{0, 255, 0, kOverlayAlpha}; // r, g, b, alpha: green, marked place
        const core::Color kHoverColor{255, 108, 1, kOverlayAlpha}; // r, g, b, alpha: orange, place under the mouse

        /// @brief Loads StallPetsWindow.xml and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load the layout into.
        /// @return The "StallPets" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("StallPetsWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("StallPets");
        }

        /// @brief Extracts the pet place number from an element name such as "StallPets2".
        /// @param name Element name.
        /// @return The place number, or 0 when the name has another prefix.
        int placeOf(const std::string& name) {
            const std::string prefix = "StallPets";
            return name.rfind(prefix, 0) == 0 ? std::stoi(name.substr(prefix.size())) : 0;
        }

        /// @brief Gives the screen rectangle of one pet's panel.
        /// @param at Window origin on the canvas.
        /// @param place Pet place (1-based).
        /// @return Rectangle covering the pet's panel.
        core::Rect placeArea(core::Point at, int place) {
            return {at.x + 25 + (place - 1) * kPitch, at.y + 68, 186, 190};
        } // panel offset 25,68 from the window, size 186x190

    }

    StallPetsWindow::StallPetsWindow(
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
          m_textSpot(screen.get<ui::Spot>("StallPetsText")) {
        screen.get<ui::DrawingArea>("StallPetsPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paint(canvas, at, m_screen.mouse(), nowMs);
        });
        screen.on("onStallPetsClick", [this](const ui::EventArgs& args) { toggle(placeOf(args.source)); });
        screen.on("onStallPetsBuy", [this](const ui::EventArgs&) { buy(); });
        screen.on("onStallPetsClose", [this](const ui::EventArgs&) { close(); });
    }

    void StallPetsWindow::show(const protocol::PetStallGoods& goods) {
        m_goods = goods;
        if (m_goods.pets.size() > static_cast<std::size_t>(kPlaces)) {
            m_goods.pets.resize(kPlaces);
        }
        m_marked.fill(false);
        const int count = static_cast<int>(m_goods.pets.size());
        const char* picture = count <= 1 ? "Form_BuyPet1Stall_1" : count == 2 ? "Form_BuyPet2Stall_1" : "Form_BuyPet3Stall_1";
        m_window.setPicture(picture, m_services.pictures);
        // first pet's gear x (narrow form 34, else 32)
        const int firstGear = count <= 1 ? 34 : 32;
        // gear grid y 308; the six-slot grid is 140 px right and at y 265
        m_screen.get<ui::Element>("StallPets1Gear").setPosition({firstGear, 308});
        m_screen.get<ui::Element>("StallPets1Gear6").setPosition({firstGear + 140, 265});
        // button shift per form width: 0, 100 or 201 px
        const int shift = count <= 1 ? 0 : count == 2 ? 100 : 201;
        // Close at x 129, Buy at x 54 (plus shift), both at y 405
        m_screen.get<ui::Element>("StallPetsClose").setPosition({129 + shift, 405});
        m_screen.get<ui::Element>("StallPetsBuy").setPosition({54 + shift, 405});
        // small close button x per form width (193, 392, 594), y 26
        m_screen.get<ui::Element>("StallPetsCloseSmall").setPosition({count <= 1 ? 193 : count == 2 ? 392 : 594, 26});
        // gold and total digits x per form width
        const int digits = count <= 1 ? 118 : count == 2 ? 219 : 319;
        // gold digits y 384, total digits y 364
        m_screen.get<ui::Element>("StallPetsGold").setPosition({digits, 384});
        m_screen.get<ui::Element>("StallPetsTotal").setPosition({digits, 364});
        for (int place = 1; place <= kPlaces; ++place) {
            const std::string name = "StallPets" + std::to_string(place);
            const bool shown = place <= count;
            std::optional<std::reference_wrapper<const protocol::PetStallGoods::Pet>> pet;
            if (shown) {
                pet = std::cref(m_goods.pets[static_cast<std::size_t>(place - 1)]);
            }
            showPetGear(
                m_services,
                m_screen.get<ui::IconGrid>(name + "Gear"),
                m_screen.get<ui::IconGrid>(name + "Gear6"),
                pet ? pet->get().equipment : std::vector<int>{}
            );
            const auto npc = pet ? m_services.npcs.find(pet->get().npcId) : std::nullopt;
            m_screen.get<ui::TextBox>("StallPetsName" + std::to_string(place)).setText(npc ? npc->name : "");
            ui::TextBox& price = m_screen.get<ui::TextBox>("StallPetsPrice" + std::to_string(place));
            const std::string amount = pet ? std::to_string(pet->get().price) : "";
            price.setText(amount);
            price.setFontColor(priceColor(amount.size()));
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void StallPetsWindow::close() {
        m_goods = {};
        m_marked.fill(false);
        m_window.setVisible(false);
        m_info.hide();
        m_infoShown = false;
    }

    void StallPetsWindow::toggle(int place) {
        if (place < 1 || place > static_cast<int>(m_goods.pets.size())) {
            return;
        }
        const protocol::PetStallGoods::Pet& pet = m_goods.pets[static_cast<std::size_t>(place - 1)];
        const auto npc = m_services.npcs.find(pet.info.npcId);
        const int level = npc ? npc->level : 0;
        if (level >= kHighLevel && level - m_status.level >= kLevelGap) {
            m_messages.show("You are not up to the level required", 2000); // message display time in ms
            return;
        }
        if (pet.info.amity == 0) {
            m_messages.show("Disqualified from buying with 0 degree of amity", 2000); // message display time in ms
            return;
        }
        m_marked[static_cast<std::size_t>(place)] = !m_marked[static_cast<std::size_t>(place)];
    }

    std::int32_t StallPetsWindow::total() const {
        std::int32_t sum = 0;
        for (int place = 1; place <= static_cast<int>(m_goods.pets.size()); ++place) {
            if (m_marked[static_cast<std::size_t>(place)]) {
                sum += m_goods.pets[static_cast<std::size_t>(place - 1)].price;
            }
        }
        return sum;
    }

    void StallPetsWindow::buy() {
        if (m_status.gold < total()) {
            m_messages.show("Not enough money", 2000); // message display time in ms
            return;
        }
        int buying = 0;
        std::vector<int> places;
        for (int place = 1; place <= static_cast<int>(m_goods.pets.size()); ++place) {
            if (!m_marked[static_cast<std::size_t>(place)]) {
                continue;
            }
            const protocol::PetStallGoods::Pet& pet = m_goods.pets[static_cast<std::size_t>(place - 1)];
            if (m_pets.placeOf(pet.npcId) != 0) {
                m_messages.show("Can't buy a pet with the same attribute", 2000); // message display time in ms
                return;
            }
            ++buying;
            places.push_back(pet.stallSlot);
        }
        if (PetTeam::kPlaces - m_pets.count() < buying) {
            m_messages.show("No enough space available for pet", 2000); // message display time in ms
            return;
        }
        if (buying <= 0) {
            m_messages.show("Please select the pet you want to buy first", 2000); // message display time in ms
            return;
        }
        m_services.connection.send(protocol::buyStallPetsRequest(m_goods.ownerId, places));
        m_window.setVisible(false);
    }

    void StallPetsWindow::paint(gfx::Canvas& canvas, core::Point at, core::Point mouse, std::uint64_t nowMs) {
        const gfx::TextStyle style{m_textSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        const auto number = [&](core::Point where, const std::string& value) {
            // 8 px per digit, text height 12
            canvas.drawText(at + where, value, style, static_cast<int>(value.size()) * 8, 12);
        };
        const auto shadow = m_services.pictures.find("PetShadow");
        const auto heart = m_services.pictures.find("Bmp_Friend_1");
        bool overPet = false;
        for (int place = 1; place <= static_cast<int>(m_goods.pets.size()); ++place) {
            const protocol::PetStallGoods::Pet& pet = m_goods.pets[static_cast<std::size_t>(place - 1)];
            const auto npc = m_services.npcs.find(pet.npcId);
            if (!npc) {
                continue;
            }
            const int dx = (place - 1) * kPitch;
            if (shadow) {
                canvas.drawPicture(*shadow, {at.x + 85 + dx, at.y + 194}); // pet shadow offset in the panel
            }
            const int slot = m_services.characters.npcSlot(npc->spriteId, npc->directionKind, kFacingSlot);
            const int frame = m_animations[static_cast<std::size_t>(place)].frame(nowMs, m_services.characters.npcFrameCount(npc->spriteId, slot));
            // pet sprite anchor (107,194) in the panel
            m_services.characters.drawNpc(canvas, *npc, at + core::Point{107 + dx, 194}, slot, frame);
            if (m_services.characters.npcContains(*npc, at + core::Point{107 + dx, 194}, slot, frame, mouse)) {
                m_info.show(pet.info, pet.maxHp, pet.maxSp, mouse);
                overPet = true;
            }
            if (m_marked[static_cast<std::size_t>(place)]) {
                canvas.fillRectAlpha(placeArea(at, place), kMarkedColor);
            }
            // stat text positions: level (50,232), hp (50,249), amity (74,267)
            number({50 + dx, 232}, std::to_string(pet.level));
            number({50 + dx, 249}, std::to_string(pet.hp) + "/" + std::to_string(pet.maxHp));
            number({74 + dx, 267}, std::to_string(pet.info.amity));
            if (heart) {
                canvas.drawPicture(*heart, {at.x + 27 + dx, at.y + 268}); // amity heart icon position
            }
        }
        if (m_infoShown && !overPet) {
            m_info.hide();
        }
        m_infoShown = overPet;
        m_screen.get<ui::DigitText>("StallPetsGold").setText(std::to_string(m_status.gold));
        m_screen.get<ui::DigitText>("StallPetsTotal").setText(std::to_string(total()));
        for (int place = 1; place <= static_cast<int>(m_goods.pets.size()); ++place) {
            const core::Rect area = placeArea(at, place);
            if (area.contains(mouse) && !m_marked[static_cast<std::size_t>(place)]) {
                canvas.fillRectAlpha(area, kHoverColor);
            }
        }
    }

}
