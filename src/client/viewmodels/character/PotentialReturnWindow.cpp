#include "client/viewmodels/character/PotentialReturnWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/PetWeapon.h"
#include "client/rules/PotentialRules.h"
#include "gfx/TextStyle.h"
#include "protocol/PotentialRequests.h"
#include "ui/DrawingArea.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>

namespace client {

    namespace {

        constexpr int kCharWidth = 8; // width of one digit/character, px
        constexpr int kStandingSlot = 49; // NPC action slot used for the standing pose
        constexpr int kHumanKindSlot = 11; // NPC action slot used instead when directionKind != 0 (human-like sprite)
        // screen positions of the 12 potential marks round the wheel, clockwise from the top, px
        constexpr std::array<core::Point, 12> kWheelPoints = {
            {{207, 221}, {236, 239}, {251, 267}, {251, 299}, {236, 329}, {207, 347}, {173, 347}, {144, 329}, {129, 299}, {129, 267}, {144, 239}, {173, 221}}
        };
        constexpr int kAntiPotentialPill = 34351; // item 34351: Anti-Potential Pill
        constexpr int kPotentialPill = 34269; // item 34269: Potential Pill (drawn to show the potential count)
        constexpr int kAskMs = 5000; // confirmation dialog timeout, ms
        constexpr std::size_t kAskWrap = 32; // confirmation dialog wrap width, characters

        /// @brief Loads PotentialReturnWindow.xml and returns its root window.
        /// @param services Game services (layout and load context).
        /// @param screen Screen to load into.
        /// @return The "PotentialReturn" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PotentialReturnWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("PotentialReturn");
        }

        /// @brief Makes a plain text style of one colour.
        /// @param color Text colour.
        /// @return The style.
        gfx::TextStyle plain(core::Color color) { return {color, gfx::TextEffect::Plain, std::nullopt}; }

    }

    PotentialReturnWindow::PotentialReturnWindow(
        GameServices& services,
        ui::Screen& screen,
        const PetTeam& pets,
        const Inventory& inventory,
        PlayerMessages& messages,
        TalkDialog& talk,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_pets(pets),
          m_inventory(inventory),
          m_messages(messages),
          m_talk(talk),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_targets(screen.get<ui::BlockList>("PotentialReturnTargets")),
          m_item(screen.get<ui::IconGrid>("PotentialReturnItem")),
          m_rowsSpot(screen.get<ui::Spot>("PotentialReturnRows")),
          m_nameSpot(screen.get<ui::Spot>("PotentialReturnRowName")),
          m_textSpot(screen.get<ui::Spot>("PotentialReturnTextColor")),
          m_haveSpot(screen.get<ui::Spot>("PotentialReturnHaveColor")),
          m_takenSpot(screen.get<ui::Spot>("PotentialReturnTakenColor")),
          m_previewSpot(screen.get<ui::Spot>("PotentialReturnPreviewFeet")),
          m_random(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count())) {
        for (int pet = 0; pet < PetTeam::kPlaces; ++pet) {
            m_infos.emplace_back(services, PetInfoPainter::Style{{0, 0}, true, false, {10, 2}}); // style: last field {10,2} is a pixel offset
        }
        screen.get<ui::DrawingArea>("PotentialReturnFigures").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paintFigures(canvas, at, nowMs);
        });
        screen.get<ui::DrawingArea>("PotentialReturnPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paintForm(canvas, at, nowMs);
        });
        screen.on("onPotentialReturnTarget", [this](const ui::EventArgs& args) { selectTarget(args.index); });
        screen.on("onPotentialReturnOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onPotentialReturnClose", [this](const ui::EventArgs&) { close(); });
    }

    void PotentialReturnWindow::open(int bagSlot) {
        m_pendingSlot = bagSlot;
        if (m_hooks.closeItemWindow) {
            m_hooks.closeItemWindow();
        }
        if (m_hooks.fighting()) {
            m_messages.show("Can't be used during the battle", 2000); // message display time in ms
            close();
            return;
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_fireFrames = {};
        m_values = {};
        m_potential = 0;
        m_targetRow = 0;
        m_previewRise = 0;
        loadList();
        for (int mark = 1; mark <= kWheel; ++mark) {
            m_fireFrames[static_cast<std::size_t>(mark)] = static_cast<int>(m_random() % 12); // a flame has 12 animation frames
        }
        m_itemSlot = m_pendingSlot;
    }

    void PotentialReturnWindow::loadList() {
        m_targets.setSelected(-1);
        m_targets.clear();
        m_rowPets.clear();
        for (int order = 1; order <= PetTeam::kPlaces; ++order) {
            const int place = m_pets.placeByOrder(order);
            if (place == 0) {
                break;
            }
            const auto& pet = m_pets.at(place);
            if (!pet) {
                break;
            }
            if (pet->info.mercenary == 0) {
                m_rowPets.push_back(place);
                m_targets.addRow({});
            }
        }
    }

    void PotentialReturnWindow::refreshPets() {
        if (isOpen()) {
            loadList();
        }
    }

    int PotentialReturnWindow::petOf(int row) const {
        return row >= 0 && row < static_cast<int>(m_rowPets.size()) ? m_rowPets[static_cast<std::size_t>(row)] : 0;
    }

    void PotentialReturnWindow::petChanged(int petPlace) {
        if (isOpen() && petPlace != 0 && petOf(m_targetRow) == petPlace) {
            selectTarget(m_targetRow);
        }
    }

    void PotentialReturnWindow::selectTarget(int row) {
        if (row < 0 || row >= static_cast<int>(m_rowPets.size())) {
            return;
        }
        m_potential = 0;
        const auto& found = m_pets.at(petOf(row));
        if (!found) {
            return;
        }
        const Pet& pet = *found;
        const formats::NpcInfo& npc = pet.info;
        const int slot = m_services.characters.npcSlot(npc.spriteId, npc.directionKind, npc.directionKind != 0 ? kHumanKindSlot : kStandingSlot);
        const int height = m_services.characters.npcHeight(npc.spriteId, npc.spriteSize, slot, 0);
        m_previewRise = 0;
        if (npc.heightMode == 0) {
            const int most = npc.spriteSize == 0 ? 90 : 128; // tallest preview height in pixels: 90 for size 0, else 128
            if (npc.spriteSize <= 1 && height > most) {
                m_previewRise = height - most;
            }
        } else if (npc.heightMode == 1) {
            m_previewRise = -static_cast<int>(std::lround(height * 0.2)); // height mode 1 moves the preview down by a fifth of its height
        }
        m_values = {pet.strength, pet.constitution, pet.intelligence, pet.wisdom, pet.agility};
        m_potential = pet.potential;
        m_targetRow = row;
        m_targets.setSelected(row);
    }

    bool PotentialReturnWindow::allowed(int place) {
        if (m_itemSlot < 1 || m_itemSlot > Inventory::kBagSlots || m_inventory.bagItem(m_itemSlot).itemId != kAntiPotentialPill) {
            return false;
        }
        const auto& pet = m_pets.at(place);
        if (m_targetRow > 3 || place == 0 || !pet) { // the form lists at most 4 rows (0..3)
            return false;
        }
        if (pet->info.mercenary != 0) {
            m_messages.show("Mercenaries cannot use this function", 2000); // message display time in ms
            return false;
        }
        if (pet->potential == 0) {
            m_messages.show("Potential power is 0, unable to use", 2000); // message display time in ms
            return false;
        }
        bool space = false;
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            space = space || m_inventory.bagItem(slot).empty();
        }
        if (!space) {
            m_messages.show("You should unfill at least 2 items space", 2000); // message display time in ms
            return false;
        }
        return true;
    }

    void PotentialReturnWindow::confirm() {
        if (!allowed(petOf(m_targetRow))) {
            return;
        }
        const auto pill = m_services.items.find(kAntiPotentialPill);
        TalkDialog::Timing timing;
        timing.wrapChars = kAskWrap;
        timing.timeoutMs = kAskMs;
        timing.exclusive = false;
        m_talk.ask("Character's potential power will be 0\rUse " + (pill ? pill->name : std::string()) + "?", std::nullopt, [this] { send(); }, [] {}, timing);
    }

    void PotentialReturnWindow::send() {
        const int place = petOf(m_targetRow);
        if (!allowed(place)) {
            return;
        }
        m_hooks.send(protocol::potentialReturnRequest(m_itemSlot, place));
    }

    void PotentialReturnWindow::paintFigures(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        for (std::size_t row = 0; row < m_rowPets.size(); ++row) {
            const auto& pet = m_pets.at(m_rowPets[row]);
            if (pet) {
                m_infos[row].paintFigure(canvas, at + m_rowsSpot.at() + core::Point{0, static_cast<int>(row) * m_rowsSpot.pitch().y}, *pet, nowMs);
            }
        }
    }

    void PotentialReturnWindow::paintRows(gfx::Canvas& canvas, core::Point at) {
        for (std::size_t row = 0; row < m_rowPets.size(); ++row) {
            const auto& pet = m_pets.at(m_rowPets[row]);
            if (!pet) {
                continue;
            }
            const core::Point origin = at + m_rowsSpot.at() + core::Point{0, static_cast<int>(row) * m_rowsSpot.pitch().y};
            // every text box is 100x16; the level ends at x 127 of the name offset, the potential ends at x 113+77
            canvas.drawText({origin.x + m_nameSpot.at().x, origin.y + m_nameSpot.at().y}, pet->name, plain(m_textSpot.color()), 100, 16);
            const std::string level = std::to_string(pet->level);
            // level text is right-aligned to x 127 of the name offset
            canvas.drawText(
                {origin.x + m_nameSpot.at().x + 127 - static_cast<int>(level.size()) * kCharWidth, origin.y + m_nameSpot.at().y},
                level,
                plain(m_textSpot.color()),
                100,
                16
            );
            const std::string most = "/" + std::to_string(kMostPotential);
            const int mostWidth = static_cast<int>(most.size()) * kCharWidth;
            canvas.drawText({origin.x + m_nameSpot.at().x + 113 + 77 - mostWidth, origin.y + m_nameSpot.at().y}, most, plain(m_textSpot.color()), 100, 16);
            const std::string have = std::to_string(pet->potential);
            canvas.drawText(
                {origin.x + m_nameSpot.at().x + 113 + 77 - mostWidth - static_cast<int>(have.size()) * kCharWidth, origin.y + m_nameSpot.at().y},
                have,
                plain(m_haveSpot.color()),
                100,
                16
            );
        }
    }

    void PotentialReturnWindow::paintForm(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        m_item.clearCells();
        protocol::ItemInstance pill;
        pill.itemId = kPotentialPill;
        pill.count = std::max(m_potential, 0);
        if (m_services.items.find(pill.itemId)) {
            ui::IconCell cell = itemIconCell(m_services, pill, true);
            m_item.setCell(1, cell);
        }
        paintRows(canvas, at);
        if (m_targets.selected() < 0) {
            return;
        }
        const auto& chosen = m_pets.at(petOf(m_targetRow));
        if (chosen) {
            const formats::NpcInfo& npc = chosen->info;
            const int slot = m_services.characters.npcSlot(npc.spriteId, npc.directionKind, npc.directionKind != 0 ? kHumanKindSlot : kStandingSlot);
            const int frame = m_animation.frame(nowMs, m_services.characters.npcFrameCount(npc.spriteId, slot));
            const PetWeapon weapon = petWeapon(*chosen, m_services.items);
            m_services.characters
                .drawArmedNpc(canvas, npc, weapon.lookId, weapon.colorCode, at + m_previewSpot.at() + core::Point{0, m_previewRise}, slot, frame);
        }
        paintFires(canvas, at);
        paintBonuses(canvas, at);
    }

    void PotentialReturnWindow::paintFires(gfx::Canvas& canvas, core::Point at) {
        const auto mark = m_services.pictures.find("Icon_PotentialMark_1");
        const auto big = m_services.pictures.find("PotentialFire1");
        for (int level = 1; level <= m_potential && level <= kWheel; ++level) {
            const core::Point where = at + kWheelPoints[static_cast<std::size_t>(level - 1)];
            if (mark) {
                canvas.drawPicture(*mark, where);
            }
            if (big) {
                int& frame = m_fireFrames[static_cast<std::size_t>(level)];
                // flame drawn 2 right and 11 up of the mark; 16x26 frames, 3 ticks per frame, 4 frames in the sheet
                canvas.drawPicture(*big, {where.x + 2, where.y - 11}, core::Rect{0, frame / 3 * 26, 16, 26});
                if (++frame >= 12) { // 12 ticks, then the flame starts over
                    frame = 0;
                }
            }
        }
    }

    void PotentialReturnWindow::paintBonuses(gfx::Canvas& canvas, core::Point at) {
        for (std::size_t i = 0; i < m_values.size(); ++i) {
            // ability rows start at y 236, 16 px apart; value at x 66, the taken amount at x 60+26+6; text box 100x16
            const int y = at.y + 236 + static_cast<int>(i) * 16;
            canvas.drawText({at.x + 66, y}, std::to_string(m_values[i]), plain(m_textSpot.color()), 100, 16);
            if (m_potential != 0) {
                canvas.drawText(
                    {at.x + 60 + 26 + 6, y},
                    "-" + std::to_string(kPetStatRequirements[static_cast<std::size_t>(m_potential)]),
                    plain(m_takenSpot.color()),
                    100,
                    16
                );
            }
        }
    }

}
