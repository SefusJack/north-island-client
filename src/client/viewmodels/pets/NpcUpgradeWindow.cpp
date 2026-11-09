#include "client/viewmodels/pets/NpcUpgradeWindow.h"

#include "client/rules/Vitality.h"
#include "gfx/TextStyle.h"
#include "protocol/RebirthRequests.h"
#include "ui/DrawingArea.h"
#include "ui/Tooltip.h"

#include <cstdlib>
#include <utility>

namespace client {

    namespace {

        constexpr int kBagSlots = 50; // number of bag slots searched (slots 1..50)
        constexpr int kMostPets = 4; // number of pet places searched (places 1..4)
        constexpr int kRebornItemType = 37; // item type 37: pet materials (Ambergris, Beak, ...), here the reborn item
        constexpr int kRebornAttribute = 65; // item attribute kind 65: marks a reborn item (both attribute kinds must be 65)
        constexpr int kLeastLevel = 30; // minimum pet level to reborn
        constexpr int kLeastAmity = 60; // minimum pet amity to reborn
        constexpr int kRebornAmity = 60; // amity shown for the pet after reborn
        constexpr int kMessageMs = 1200; // message display time, ms
        constexpr int kFigureSlot = 17; // NPC action slot of the pose shown for each pet

        /// @brief Loads NpcUpgradeWindow.xml into the screen and returns its window.
        /// @param services Shared game services (layout and load context).
        /// @param screen Screen the layout is loaded into.
        /// @return The NpcUpgrade window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("NpcUpgradeWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("NpcUpgrade");
        }

        /// @brief Names the picture of an element icon.
        /// @details Original FElementPics: icon_01 to icon_04 for the elements and icon_09 for the fifth.
        /// @param element Element number (1 to 5).
        /// @return The picture name, or an empty string for an unknown element.
        std::string elementPicture(int element) {
            // elements 1 to 4 use icon_01 to icon_04
            if (element >= 1 && element <= 4) {
                return "icon_0" + std::to_string(element);
            }
            // the fifth element uses icon_09
            return element == 5 ? "icon_09" : "";
        }

    }

    NpcUpgradeWindow::NpcUpgradeWindow(
        GameServices& services,
        ui::Screen& screen,
        const PetTeam& pets,
        const Inventory& inventory,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_pets(pets),
          m_inventory(inventory),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_figureSpot(screen.get<ui::Spot>("NpcUpgradeFigure")),
          m_nameSpot(screen.get<ui::Spot>("NpcUpgradeName")),
          m_elementSpot(screen.get<ui::Spot>("NpcUpgradeElement")),
          m_skillSpot(screen.get<ui::Spot>("NpcUpgradeSkill")),
          m_skillSideSpot(screen.get<ui::Spot>("NpcUpgradeSkillSide")),
          m_levelSpot(screen.get<ui::Spot>("NpcUpgradeLevel")),
          m_hpSpot(screen.get<ui::Spot>("NpcUpgradeHp")),
          m_spSpot(screen.get<ui::Spot>("NpcUpgradeSp")),
          m_statsSpot(screen.get<ui::Spot>("NpcUpgradeStats")),
          m_digitSpot(screen.get<ui::Spot>("NpcUpgradeDigit")),
          m_textColorSpot(screen.get<ui::Spot>("NpcUpgradeTextColor")) {
        screen.get<ui::DrawingArea>("NpcUpgradePaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paint(canvas, at, nowMs);
        });
        screen.on("onNpcUpgradeClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onNpcUpgradeOk", [this](const ui::EventArgs&) { request(); });
        screen.on("onNpcUpgradeNext", [this](const ui::EventArgs&) { step(1); });
        screen.on("onNpcUpgradePrevious", [this](const ui::EventArgs&) { step(-1); });
    }

    void NpcUpgradeWindow::open() {
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_index = 0;
        collect();
        preview();
    }

    void NpcUpgradeWindow::close() {
        m_window.setVisible(false);
        m_hooks.closed();
    }

    void NpcUpgradeWindow::collect() {
        m_candidates.clear();
        for (int slot = 1; slot <= kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (item.itemId == 0) {
                continue;
            }
            const auto record = m_services.items.find(item.itemId);
            if (!record || record->type != kRebornItemType || record->attributeKinds[0] != kRebornAttribute || record->attributeKinds[1] != kRebornAttribute) {
                continue;
            }
            for (int place = 1; place <= kMostPets; ++place) {
                const auto& pet = m_pets.at(place);
                if (pet && pet->info.npcId != 0 && pet->info.npcId == record->attributeValues[0] && pet->level >= kLeastLevel && pet->amity >= kLeastAmity) {
                    m_candidates.push_back({place, slot});
                }
            }
        }
    }

    void NpcUpgradeWindow::preview() {
        if (m_candidates.empty()) {
            return;
        }
        const Candidate& candidate = m_candidates[static_cast<std::size_t>(m_index)];
        if (const auto& pet = m_pets.at(candidate.petPlace); pet && pet->info.npcId != 0) {
            previewPet(*pet);
        }
        if (const protocol::ItemInstance& item = m_inventory.bagItem(candidate.bagSlot); item.itemId != 0) {
            previewReborn(item.itemId);
        }
        for (Side& side : m_sides) {
            if (side.shown) {
                loadSkills(side);
            }
        }
    }

    void NpcUpgradeWindow::previewPet(const Pet& pet) {
        Side& side = m_sides[0];
        side.shown = true;
        side.info = pet.info;
        side.name = pet.name;
        side.level = pet.level;
        side.hp = pet.hp;
        side.sp = pet.sp;
        side.stats = {pet.intelligence, pet.strength, pet.constitution, pet.constitution, pet.wisdom, pet.agility, pet.amity};
        side.weapon = petWeapon(pet, m_services.items);
    }

    void NpcUpgradeWindow::previewReborn(int itemId) {
        const auto record = m_services.items.find(itemId);
        const auto info = record ? m_services.npcs.find(record->attributeValues[1]) : std::nullopt;
        if (!info) {
            return;
        }
        Side& side = m_sides[1];
        side.shown = true;
        side.info = *info;
        side.name = info->name;
        side.level = 1;
        side.hp = maxHp(m_services.formula, totalLevel(1, 0), info->constitution, 0);
        side.sp = maxSp(m_services.formula, totalLevel(1, 0), info->wisdom, 0);
        side.stats = {info->intelligence, info->strength, info->constitution, info->constitution, info->wisdom, info->agility, kRebornAmity};
        side.weapon = {};
    }

    void NpcUpgradeWindow::loadSkills(Side& side) {
        for (std::size_t index = 0; index < side.skillNames.size(); ++index) {
            const int skill = side.info.skills[index];
            const auto record = skill != 0 ? m_services.skills.find(skill) : std::nullopt;
            side.skillNames[index] = record ? record->name : std::string();
        }
    }

    void NpcUpgradeWindow::step(int direction) {
        if (m_candidates.empty()) {
            return;
        }
        const int count = static_cast<int>(m_candidates.size());
        const int before = m_index;
        m_index = (m_index + direction + count) % count;
        if (before != m_index) {
            preview();
        }
    }

    void NpcUpgradeWindow::request() {
        if (m_candidates.empty()) {
            return;
        }
        const auto& chosen = m_pets.at(m_candidates[static_cast<std::size_t>(m_index)].petPlace);
        const int chosenKind = chosen ? chosen->info.npcId : 0;
        bool sent = false;
        for (int slot = 1; slot <= kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (item.itemId == 0) {
                continue;
            }
            const auto record = m_services.items.find(item.itemId);
            if (!record || chosenKind != record->attributeValues[0] || record->type != kRebornItemType || record->attributeKinds[0] != kRebornAttribute ||
                record->attributeKinds[1] != kRebornAttribute) {
                continue;
            }
            for (int place = 1; place <= kMostPets; ++place) {
                const auto& pet = m_pets.at(place);
                if (!pet || pet->info.npcId != record->attributeValues[0] || pet->level < kLeastLevel || pet->amity < kLeastAmity) {
                    continue;
                }
                m_hooks.send(protocol::petRebornRequest(place));
                sent = true;
                break;
            }
        }
        if (!sent) {
            m_messages.show("You have failed to reborn because you are not qualified yet.", kMessageMs);
        }
        close();
    }

    void NpcUpgradeWindow::paintNumber(gfx::Canvas& canvas, core::Point at, int value) {
        const auto digits = m_services.pictures.find("Num_White3_2");
        if (!digits) {
            return;
        }
        const gfx::Picture& picture = m_services.pictures.picture(*digits);
        const int rowHeight = picture.height() / 10; // the digit picture holds ten rows, digits 0 to 9
        const std::string text = std::to_string(std::abs(value));
        for (std::size_t index = 0; index < text.size(); ++index) {
            const int digit = text[index] - '0';
            canvas.drawPictureScaled(
                *digits,
                {at.x + static_cast<int>(index) * m_digitSpot.size().width, at.y, m_digitSpot.size().width, m_digitSpot.size().height},
                core::Rect{0, digit * rowHeight, picture.width(), rowHeight}
            );
        }
    }

    void NpcUpgradeWindow::paintSide(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs, int index) {
        const Side& side = m_sides[static_cast<std::size_t>(index)];
        if (!side.shown) {
            return;
        }
        const gfx::TextStyle style{m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        CharacterRenderer& characters = m_services.characters;
        const int slot = characters.npcSlot(side.info.spriteId, side.info.directionKind, kFigureSlot);
        const int frame = m_animations[static_cast<std::size_t>(index)].frame(nowMs, characters.npcFrameCount(side.info.spriteId, slot));
        characters.drawArmedNpc(
            canvas,
            side.info,
            side.weapon.lookId,
            side.weapon.colorCode,
            at + m_figureSpot.at() + core::Point{index * m_figureSpot.pitch().x, 0},
            slot,
            frame
        );
        canvas
            .drawText(at + m_nameSpot.at() + core::Point{index * m_nameSpot.pitch().x, 0}, side.name, style, m_nameSpot.size().width, m_nameSpot.size().height);
        if (const auto element = m_services.pictures.find(elementPicture(side.info.element))) {
            canvas.drawPicture(
                *element,
                at + m_elementSpot.at() + core::Point{index * m_nameSpot.pitch().x, 0},
                core::Rect{0, 0, m_elementSpot.size().width, m_elementSpot.size().height}
            );
        }
        for (int skill = 0; skill < 3; ++skill) { // three skills per pet kind
            const int id = side.info.skills[static_cast<std::size_t>(skill)];
            if (const auto icon = id != 0 ? m_services.pictures.find("icon_sk" + std::to_string(id)) : std::nullopt) {
                canvas.drawPicture(*icon, at + m_skillSpot.at() + core::Point{index * m_skillSideSpot.pitch().x + skill * m_skillSpot.pitch().x, 0});
            }
        }
        paintNumber(canvas, at + m_levelSpot.at() + core::Point{index * m_figureSpot.pitch().x, 0}, side.level);
        paintNumber(canvas, at + m_hpSpot.at() + core::Point{index * m_figureSpot.pitch().x, 0}, side.hp);
        paintNumber(canvas, at + m_spSpot.at() + core::Point{index * m_figureSpot.pitch().x, 0}, side.sp);
        for (std::size_t stat = 0; stat < side.stats.size(); ++stat) {
            const std::string text = std::to_string(side.stats[stat]) + (index == 1 ? "+?" : "");
            canvas.drawText(
                at + m_statsSpot.at() + core::Point{index * m_figureSpot.pitch().x, static_cast<int>(stat) * m_statsSpot.pitch().y},
                text,
                style,
                m_nameSpot.size().width,
                m_nameSpot.size().height
            );
        }
    }

    void NpcUpgradeWindow::paintTooltip(gfx::Canvas& canvas, core::Point at) {
        const core::Point mouse = m_screen.mouse();
        for (int index = 0; index < 2; ++index) { // the two sides: the pet and the pet it is reborn as
            const Side& side = m_sides[static_cast<std::size_t>(index)];
            for (int skill = 0; skill < 3; ++skill) { // three skills per pet kind
                const int id = side.info.skills[static_cast<std::size_t>(skill)];
                if (id == 0 || !m_services.pictures.find("icon_sk" + std::to_string(id))) {
                    continue;
                }
                const core::Point corner = at + m_skillSpot.at() + core::Point{index * m_skillSideSpot.pitch().y + skill * m_skillSpot.pitch().x, 0};
                if (core::Rect{corner.x, corner.y, m_skillSpot.size().width, m_skillSpot.size().height}.contains(mouse)) {
                    ui::drawTooltipAt(canvas, corner, side.skillNames[static_cast<std::size_t>(skill)]);
                    return;
                }
            }
        }
    }

    void NpcUpgradeWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        if (m_candidates.empty()) {
            return;
        }
        paintSide(canvas, at, nowMs, 0);
        paintSide(canvas, at, nowMs, 1);
        paintTooltip(canvas, at);
    }

}
