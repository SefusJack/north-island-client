#include "client/viewmodels/character/PotentialWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/PetWeapon.h"
#include "client/rules/PetEquipping.h"
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

        constexpr int kLongName = 12; // names longer than this many characters are raised
        constexpr int kLongNameRise = 7; // how far a long name is raised, px
        constexpr int kCharWidth = 8; // width of one digit/character, px
        constexpr int kStandingSlot = 49; // action slot of the standing pose (player figure and NPCs)
        constexpr int kHumanKindSlot = 11; // NPC action slot used instead when directionKind != 0 (human-like sprite)
        // screen positions of the 12 potential marks round the wheel, clockwise from the top, px
        constexpr std::array<core::Point, 12> kWheelPoints = {
            {{201, 259}, {230, 277}, {245, 305}, {245, 337}, {230, 367}, {201, 385}, {167, 385}, {138, 367}, {123, 337}, {123, 305}, {138, 277}, {167, 259}}
        };
        // pill item per mode 0..2
        constexpr std::array<int, 3> kPills = {
            34269, // Potential Pill
            34289, // Potential Pill (second variant)
            34350 // Super Potential Pill
        };
        constexpr int kMostPills = 999; // pill count shown is capped at this
        constexpr int kAskMs = 5000; // confirmation dialog timeout, ms
        constexpr std::size_t kAskWrap = 32; // confirmation dialog wrap width, characters
        constexpr const char* kMaxMessage = "You have reached the character's max potential energy";
        constexpr const char* kBelowTenMessage = "Potential energy value is less then 10, cannot use";

        /// @brief Loads PotentialWindow.xml into the screen.
        /// @param services Game services (layout, load context).
        /// @param screen Screen to load into.
        /// @return The "PotentialWindow" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PotentialWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("PotentialWindow");
        }

        /// @brief Plain (no effect) text style of a colour.
        /// @param color Text colour.
        /// @return The style.
        gfx::TextStyle plain(core::Color color) { return {color, gfx::TextEffect::Plain, std::nullopt}; }

    }

    PotentialWindow::PotentialWindow(
        GameServices& services,
        ui::Screen& screen,
        const PlayerStatus& status,
        const PetTeam& pets,
        const Inventory& inventory,
        const BagMarks& marks,
        const MapCharacter& player,
        PlayerMessages& messages,
        TalkDialog& talk,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_status(status),
          m_pets(pets),
          m_inventory(inventory),
          m_marks(marks),
          m_player(player),
          m_messages(messages),
          m_talk(talk),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_targets(screen.get<ui::BlockList>("PotentialTargets")),
          m_item(screen.get<ui::IconGrid>("PotentialItem")),
          m_ok(screen.get<ui::ImageButton>("PotentialOk")),
          m_rowsSpot(screen.get<ui::Spot>("PotentialRows")),
          m_nameSpot(screen.get<ui::Spot>("PotentialRowName")),
          m_textSpot(screen.get<ui::Spot>("PotentialTextColor")),
          m_haveSpot(screen.get<ui::Spot>("PotentialHaveColor")),
          m_previewSpot(screen.get<ui::Spot>("PotentialPreviewFeet")),
          m_random(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count())) {
        for (int pet = 0; pet < PetTeam::kPlaces; ++pet) {
            // PetInfoPainter style: origin, two flags, and the {10, 2} offset of the pet figure
            m_infos.emplace_back(services, PetInfoPainter::Style{{0, 0}, true, false, {10, 2}});
        }
        screen.get<ui::DrawingArea>("PotentialFigures").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paintFigures(canvas, at, nowMs);
        });
        screen.get<ui::DrawingArea>("PotentialPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paintForm(canvas, at, nowMs);
        });
        screen.on("onPotentialTarget", [this](const ui::EventArgs& args) { selectTarget(args.index); });
        screen.on("onPotentialOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onPotentialClose", [this](const ui::EventArgs&) { close(); });
    }

    void PotentialWindow::toggle() {
        if (isOpen()) {
            close();
        } else {
            show();
        }
    }

    void PotentialWindow::open(int mode) {
        m_pendingMode = mode;
        if (m_hooks.closeItemWindow) {
            m_hooks.closeItemWindow();
        }
        show();
    }

    void PotentialWindow::show() {
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
        m_itemSlot = 0;
        m_requestSent = false;
        m_previewRise = 0;
        loadList();
        for (int mark = 1; mark <= kWheel; ++mark) {
            m_fireFrames[static_cast<std::size_t>(mark)] = static_cast<int>(m_random() % 12); // number of frames in a flame animation
        }
        m_mode = m_pendingMode;
        m_pendingMode = 0;
    }

    int PotentialWindow::itemId() const { return kPills[static_cast<std::size_t>(std::clamp(m_mode, 0, 2))]; }

    void PotentialWindow::loadList() {
        m_targets.setSelected(-1);
        m_targets.clear();
        m_rowPets.clear();
        m_targets.addRow({});
        for (int order = 1; order <= PetTeam::kPlaces; ++order) {
            const int place = m_pets.placeByOrder(order);
            if (place == 0) {
                continue;
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

    void PotentialWindow::refreshPets() {
        if (isOpen()) {
            loadList();
        }
    }

    void PotentialWindow::petChanged(int petPlace) {
        if (isOpen() && petPlace != 0 && petOf(m_targetRow) == petPlace) {
            selectTarget(m_targetRow);
        }
    }

    int PotentialWindow::petOf(int row) const {
        return row >= 1 && row <= static_cast<int>(m_rowPets.size()) ? m_rowPets[static_cast<std::size_t>(row - 1)] : 0;
    }

    void PotentialWindow::selectTarget(int row) {
        if (row < 0 || row > static_cast<int>(m_rowPets.size())) {
            return;
        }
        m_requestSent = false;
        m_itemSlot = 0;
        m_values = {};
        m_previewRise = 0;
        std::array<int, 5> stats = m_status.stats;
        if (row == 0) {
            m_potential = m_status.potential;
        } else {
            const auto& found = m_pets.at(petOf(row));
            if (!found) {
                return;
            }
            const Pet& pet = *found;
            const formats::NpcInfo& npc = pet.info;
            const int slot = m_services.characters.npcSlot(npc.spriteId, npc.directionKind, npc.directionKind != 0 ? kHumanKindSlot : kStandingSlot);
            const int height = m_services.characters.npcHeight(npc.spriteId, npc.spriteSize, slot, 0);
            if (npc.heightMode == 0) {
                const int most = npc.spriteSize == 0 ? 90 : 128; // tallest figure height (px) that fits the preview, by sprite size
                if (npc.spriteSize <= 1 && height > most) {
                    m_previewRise = height - most;
                }
            } else if (npc.heightMode == 1) {
                m_previewRise = -static_cast<int>(std::lround(height * 0.2)); // share of the figure height the preview is moved up
            }
            m_potential = pet.potential;
            stats = {pet.strength, pet.constitution, pet.intelligence, pet.wisdom, pet.agility};
        }
        for (std::size_t i = 0; i < m_values.size(); ++i) {
            m_values[i] = stats[i] - kPetStatRequirements[static_cast<std::size_t>(m_potential)];
        }
        m_targetRow = row;
        m_targets.setSelected(row);
    }

    int PotentialWindow::firstPillSlot() const {
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (item.itemId == itemId() && item.count > 0 && !m_marks.lockNotice(slot, 0) && m_marks.isAvailable(slot)) {
                return slot;
            }
        }
        return 0;
    }

    int PotentialWindow::pillCount() const {
        int count = 0;
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (item.itemId == itemId() && item.count > 0 && !m_marks.lockNotice(slot, 0) && m_marks.isAvailable(slot)) {
                count += item.count;
                if (count > kMostPills) {
                    return kMostPills;
                }
            }
        }
        return count;
    }

    bool PotentialWindow::allowed(int place) {
        if (place != 0) {
            const auto& pet = m_pets.at(place);
            if (!pet) {
                return false;
            }
            if (pet->info.mercenary != 0) {
                m_messages.show("Mercenaries cannot use this function", 2000); // message display time in ms
                return false;
            }
            if (pet->potential + 1 > kMostPotential) {
                m_messages.show(kMaxMessage, 2000); // message display time in ms
                return false;
            }
            if (itemId() == kPills[1] && pet->potential < 10) { // the middle pill needs potential 10 at least
                m_messages.show(kBelowTenMessage, 2000); // message display time in ms
                return false;
            }
            return true;
        }
        if (m_status.potential + 1 > kMostPotential) {
            m_messages.show(kMaxMessage, 2000); // message display time in ms
            return false;
        }
        if (itemId() == kPills[1] && m_status.potential < 10) { // the middle pill needs potential 10 at least
            m_messages.show(kBelowTenMessage, 2000); // message display time in ms
            return false;
        }
        return true;
    }

    void PotentialWindow::confirm() {
        const int slot = firstPillSlot();
        if (slot == 0 || m_targetRow > static_cast<int>(m_rowPets.size())) {
            return;
        }
        const int place = petOf(m_targetRow);
        bool blocked = false;
        if (place != 0) {
            const auto& pet = m_pets.at(place);
            if (!pet) {
                return;
            }
            const auto base = m_services.npcs.find(petCareerBaseId(m_services.npcs, pet->info.npcId));
            blocked = pet->info.growKey == 0 && !(base && base->growKey != 0) && pet->potential == 0;
        }
        if (!allowed(place)) {
            return;
        }
        m_itemSlot = slot;
        std::string question;
        if (place == 0 && itemId() != kPills[1] && itemId() != kPills[2]) {
            const int loss = m_status.potential > 0 ? kPetStatRequirements[static_cast<std::size_t>(m_status.potential)] -
                                                          kPetStatRequirements[static_cast<std::size_t>(m_status.potential - 1)]
                                                    : 0;
            std::array<int, 5> stats = m_status.stats;
            for (int& stat : stats) {
                stat -= loss;
            }
            for (const int skillId : lostSkills(m_services.skills, m_status, m_status.element, stats)) {
                if (const auto skill = m_services.skills.find(skillId)) {
                    question += (question.empty() ? "" : "\r") + skill->name;
                }
            }
            if (!question.empty()) {
                question = "If a failure occurs, the following skills will be forgotten Sure? \r" + question;
            }
        }
        const auto pill = m_services.items.find(itemId());
        const std::string pillName = pill ? pill->name : std::string();
        const auto cancel = [this] { m_itemSlot = 0; };
        TalkDialog::Timing timing;
        timing.wrapChars = kAskWrap;
        timing.timeoutMs = kAskMs;
        timing.onTimeout = cancel;
        timing.exclusive = false;
        const std::string warning = "After the potential energy is radiated the pet #R#Bwon't be traded/#R/#B\rUse " + pillName + "?";
        if (blocked) {
            m_talk.ask(warning, std::nullopt, [this] { send(); }, cancel, timing);
        } else if (!m_requestSent || !question.empty()) {
            m_talk.ask(question.empty() ? "Use " + pillName + "?" : question, std::nullopt, [this] { send(); }, cancel, timing);
        } else {
            send();
        }
    }

    void PotentialWindow::send() {
        if (m_itemSlot < 1 || m_itemSlot > Inventory::kBagSlots || m_inventory.bagItem(m_itemSlot).itemId != itemId() ||
            m_targetRow > static_cast<int>(m_rowPets.size())) {
            return;
        }
        const int place = petOf(m_targetRow);
        if (!allowed(place)) {
            return;
        }
        m_requestSent = true;
        m_hooks.send(protocol::potentialRequest(m_itemSlot, place));
    }

    void PotentialWindow::result(int petPlace, int before, int now) {
        const bool own = petPlace == 0 ? m_targetRow == 0 : petOf(m_targetRow) == petPlace;
        if (own) {
            m_potential = now;
        }
        if (petPlace == 0 && !own) {
            return;
        }
        if (now > before) {
            m_services.playSound("sound/SEB0300.wav");
            m_fireFrames[static_cast<std::size_t>(std::clamp(m_potential, 0, kWheel))] = 0;
        } else if (m_mode == 1) {
            m_services.playSound("sound/wav0152.wav");
        }
    }

    void PotentialWindow::paintFigures(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        for (std::size_t row = 0; row < m_rowPets.size(); ++row) {
            const auto& pet = m_pets.at(m_rowPets[row]);
            if (pet) {
                m_infos[row].paintFigure(canvas, at + m_rowsSpot.at() + core::Point{0, static_cast<int>(row + 1) * m_rowsSpot.pitch().y}, *pet, nowMs);
            }
        }
    }

    void PotentialWindow::paintRow(
        gfx::Canvas& canvas,
        core::Point at,
        int row,
        const std::string& name,
        int level,
        int potential,
        int nameRise
    ) {
        // text args after the style: 100 and 16 are the draw width and line height in px; 127, 113 and 77 are the column offsets of the level, the potential block and its width
        const core::Point origin = at + m_rowsSpot.at() + core::Point{0, row * m_rowsSpot.pitch().y};
        canvas.drawText({origin.x + m_nameSpot.at().x, origin.y + m_nameSpot.at().y + nameRise}, name, plain(m_textSpot.color()), 100, 16);
        const std::string levelText = std::to_string(level);
        canvas.drawText(
            {origin.x + m_nameSpot.at().x + 127 - static_cast<int>(levelText.size()) * kCharWidth, origin.y + m_nameSpot.at().y},
            levelText,
            plain(m_textSpot.color()),
            100,
            16
        );
        const std::string most = "/" + std::to_string(kMostPotential);
        const int mostWidth = static_cast<int>(most.size()) * kCharWidth;
        canvas.drawText({origin.x + m_nameSpot.at().x + 113 + 77 - mostWidth, origin.y + m_nameSpot.at().y}, most, plain(m_textSpot.color()), 100, 16);
        const std::string have = std::to_string(potential);
        canvas.drawText(
            {origin.x + m_nameSpot.at().x + 113 + 77 - mostWidth - static_cast<int>(have.size()) * kCharWidth, origin.y + m_nameSpot.at().y},
            have,
            plain(m_haveSpot.color()),
            100,
            16
        );
    }

    void PotentialWindow::paintForm(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        const int count = pillCount();
        m_item.clearCells();
        protocol::ItemInstance pill;
        pill.itemId = itemId();
        pill.count = count;
        if (m_services.items.find(pill.itemId)) {
            m_item.setCell(1, itemIconCell(m_services, pill, count > 0));
        }
        m_ok.setVisible(count > 0 && m_targets.selected() >= 0);
        if (!m_ok.isVisible()) {
            if (const auto confirmIcon = m_services.pictures.find("icon_Confirm_1")) {
                canvas.drawPicture(*confirmIcon, at + core::Point{m_ok.bounds().x, m_ok.bounds().y});
            }
        }
        paintRow(canvas, at, 0, m_player.name, m_status.level, m_status.potential, 0);
        m_services.characters.drawLifeFace(canvas, m_player.look, at + m_rowsSpot.at() + core::Point{16, -3}); // face icon offset in the player row, px
        for (std::size_t row = 0; row < m_rowPets.size(); ++row) {
            const auto& pet = m_pets.at(m_rowPets[row]);
            if (pet) {
                paintRow(
                    canvas,
                    at,
                    static_cast<int>(row) + 1,
                    pet->name,
                    pet->level,
                    pet->potential,
                    static_cast<int>(pet->name.size()) > kLongName ? -kLongNameRise : 0
                );
            }
        }
        if (m_targets.selected() < 0) {
            return;
        }
        const core::Point feet = at + m_previewSpot.at() + core::Point{0, m_previewRise};
        const auto& chosen = m_pets.at(petOf(m_targetRow));
        if (m_targetRow > 0 && chosen) {
            const formats::NpcInfo& npc = chosen->info;
            const int slot = m_services.characters.npcSlot(npc.spriteId, npc.directionKind, npc.directionKind != 0 ? kHumanKindSlot : kStandingSlot);
            const int frame = m_animation.frame(nowMs, m_services.characters.npcFrameCount(npc.spriteId, slot));
            const PetWeapon weapon = petWeapon(*chosen, m_services.items);
            m_services.characters.drawArmedNpc(canvas, npc, weapon.lookId, weapon.colorCode, feet, slot, frame);
        } else {
            const int frame = m_animation.frame(nowMs, m_services.characters.figureFrameCount(m_player.look, kStandingSlot));
            m_services.characters.drawFigure(canvas, m_player.look, feet, kStandingSlot, frame);
        }
        paintFires(canvas, at);
        paintBonuses(canvas, at);
    }

    void PotentialWindow::paintFires(gfx::Canvas& canvas, core::Point at) {
        const auto mark = m_services.pictures.find("Icon_PotentialMark_1");
        const auto big = m_services.pictures.find("PotentialFire1");
        const auto small = m_services.pictures.find("PotentialFire2");
        const auto step = [this, &canvas](int index) {
            if (!canvas.isAnimationFrame()) {
                return; // one step a 30 ms beat, not one a painted frame
            }
            int& frame = m_fireFrames[static_cast<std::size_t>(index)];
            ++frame;
            if (frame >= 12) { // number of frames in a flame animation
                frame = 0;
            }
        };
        for (int level = 1; level <= m_potential && level <= kWheel; ++level) {
            const core::Point where = at + kWheelPoints[static_cast<std::size_t>(level - 1)];
            if (mark) {
                canvas.drawPicture(*mark, where);
            }
            if (big) {
                const int frame = m_fireFrames[static_cast<std::size_t>(level)];
                // big flame: offset (2, -11) from the mark; 4 sub-frames (frame / 3) of a 16x26 strip
                canvas.drawPicture(*big, {where.x + 2, where.y - 11}, core::Rect{0, frame / 3 * 26, 16, 26});
                step(level);
            }
        }
        const int next = m_potential + 1;
        if (next <= kWheel && small) {
            const core::Point where = at + kWheelPoints[static_cast<std::size_t>(next - 1)];
            if (mark) {
                canvas.drawPicture(*mark, where);
            }
            const int frame = m_fireFrames[static_cast<std::size_t>(next)];
            // small flame: offset (7, 1) from the mark; 4 sub-frames (frame / 3) of an 8x14 strip
            canvas.drawPicture(*small, {where.x + 7, where.y + 1}, core::Rect{0, frame / 3 * 14, 8, 14});
            step(next);
        }
    }

    void PotentialWindow::paintBonuses(gfx::Canvas& canvas, core::Point at) {
        for (std::size_t i = 0; i < m_values.size(); ++i) {
            // ability list: first line at y 274, 16 px per line; value at x 60, bonus 26 px to its right
            const int y = at.y + 274 + static_cast<int>(i) * 16;
            canvas.drawText({at.x + 60, y}, std::to_string(m_values[i]), plain(m_textSpot.color()), 100, 16);
            canvas.drawText(
                {at.x + 60 + 26, y},
                "+" + std::to_string(kPetStatRequirements[static_cast<std::size_t>(m_potential)]),
                plain(m_haveSpot.color()),
                100,
                16
            );
        }
    }

}
