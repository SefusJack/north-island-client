#include "client/viewmodels/pets/ForageWindow.h"

#include "core/Log.h"
#include "gfx/TextStyle.h"
#include "protocol/ForageRequests.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kShortMs = 1000; // short message display time, ms
        constexpr int kLongMs = 2000; // long message display time, ms
        constexpr int kPetType = 1; // pet info type value required for a pet to be sent foraging
        constexpr int kStandingSlot = 11; // NPC action slot of the standing pose
        constexpr int kForagingSlot = 3; // NPC action slot of the foraging pose
        constexpr int kStripTop = 120; // top of the target slot strip inside the window, px
        constexpr int kStripBottom = 150; // bottom of the target slot strip inside the window, px
        // left and right x edges (px, inside the window) of target slots 1..3; index 0 unused
        constexpr int kTargetLeft[] = {0, 40, 80, 120};
        constexpr int kTargetRight[] = {0, 70, 110, 150};
        constexpr int kStackFlag = 2; // item record flag bit value 2: cannot be transferred, so it cannot be a target

        /// @brief Loads ForageWindow.xml into the screen and returns its window.
        /// @param services Shared game services.
        /// @param screen Screen to load into.
        /// @return The "Forage" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("ForageWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Forage");
        }

    }

    ForageWindow::ForageWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        BagMarks& marks,
        const PetTeam& pets,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_marks(marks),
          m_pets(pets),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_targetAreas{
              screen.get<ui::DrawingArea>("ForageTarget1"),
              screen.get<ui::DrawingArea>("ForageTarget2"),
              screen.get<ui::DrawingArea>("ForageTarget3")
          },
          m_petSpot(screen.get<ui::Spot>("ForagePet")),
          m_nameSpot(screen.get<ui::Spot>("ForageName")),
          m_textColorSpot(screen.get<ui::Spot>("ForageTextColor")) {
        screen.get<ui::DrawingArea>("ForagePaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paintPet(canvas, at, nowMs);
        });
        for (int target = 1; target <= kTargets; ++target) {
            m_targetAreas[static_cast<std::size_t>(target - 1)].get().setPainter([this, target](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                paintTarget(canvas, at, target);
            });
        }
        screen.on("onForageStart", [this](const ui::EventArgs&) { start(); });
        screen.on("onForageCancel", [this](const ui::EventArgs&) { cancel(); });
        screen.on("onForageClose", [this](const ui::EventArgs&) { closeForm(); });
        screen.on("onForageTargetClear", [this](const ui::EventArgs&) { clickTarget(); });
    }

    void ForageWindow::open(int bagSlot) {
        if (m_foraging) {
            m_messages.show("The pet has been dispatched to look for items.", kLongMs);
            return;
        }
        if (m_boneUsed) {
            m_messages.show("The dog bone has been selected", kLongMs);
            return;
        }
        m_marks.setUseMark(bagSlot, kBoneMark);
        const core::Rect items = m_hooks.itemWindowBounds();
        const core::Rect area = m_window.bounds();
        const int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
        m_window.setPosition({left, items.y});
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        m_boneSlot = bagSlot;
        m_boneUsed = true;
        m_petKind = 0;
        findPet();
        if (m_petKind == 0) {
            hide();
            reset();
            m_messages.show("No suitable pet", kLongMs);
        }
        m_targetCount = 0;
        m_ordinal = 0;
        loadTargets();
        m_hooks.marksChanged();
    }

    void ForageWindow::findPet() {
        m_petKind = 0;
        for (int order = 1; order <= PetTeam::kPlaces; ++order) {
            const int place = m_pets.placeByOrder(order);
            const std::optional<Pet>& pet = place != 0 ? m_pets.at(place) : std::nullopt;
            if (!pet || pet->info.type != kPetType || isForaging(place) || !m_hooks.petFree(place)) {
                continue;
            }
            m_petPlace = place;
            m_petKind = pet->info.npcId;
            break;
        }
    }

    void ForageWindow::loadTargets() {
        if (const auto file = ForageFile::read(ForageFile::pathFor(m_services.game.userRoot(), m_hooks.playerId()))) {
            m_saved = file->items;
        }
        for (int target = 1; target <= kTargets; ++target) {
            m_targets[static_cast<std::size_t>(target)] = m_saved[static_cast<std::size_t>(target)];
            setTargetHint(target);
        }
    }

    void ForageWindow::setTargetHint(int target) {
        const int itemId = m_targets[static_cast<std::size_t>(target)];
        const auto record = itemId != 0 ? m_services.items.find(itemId) : std::nullopt;
        m_targetAreas[static_cast<std::size_t>(target - 1)].get().setHint(record ? record->name : std::string());
    }

    int ForageWindow::targetUnderMouse() const {
        const core::Point local = m_screen.mouse() - m_window.bounds().position();
        if (local.y <= kStripTop || local.y >= kStripBottom) {
            return 0;
        }
        for (int target = 1; target <= kTargets; ++target) {
            if (local.x > kTargetLeft[target] && local.x < kTargetRight[target]) {
                return target;
            }
        }
        return 0;
    }

    void ForageWindow::addTarget(int bagSlot) {
        constexpr const char* kNoLay = "Unable to lay the item";
        if (m_foraging) {
            m_messages.show("The action is not available", kLongMs);
            return;
        }
        const int itemId = m_inventory.bagItem(bagSlot).itemId;
        if (itemId == 0) {
            return;
        }
        if (!m_inventory.isStackable(itemId)) {
            m_messages.show("The items can't be taken", kLongMs);
            return;
        }
        const auto record = m_services.items.find(itemId);
        if (!record) {
            m_messages.show(kNoLay, kShortMs);
            return;
        }
        if (const auto notice = m_marks.lockNotice(bagSlot, 0)) {
            m_messages.show(notice->text, notice->ms);
            return;
        }
        if (!m_marks.isAvailable(bagSlot) || (record->flags & kStackFlag) != 0) {
            m_messages.show(kNoLay, kShortMs);
            return;
        }
        if (const int target = targetUnderMouse()) {
            m_targets[static_cast<std::size_t>(target)] = itemId;
            setTargetHint(target);
        }
    }

    void ForageWindow::clearTarget(int target) {
        m_targets[static_cast<std::size_t>(target)] = 0;
        setTargetHint(target);
    }

    void ForageWindow::clickTarget() {
        if (m_foraging) {
            return;
        }
        if (const int target = targetUnderMouse()) {
            clearTarget(target);
        }
    }

    bool ForageWindow::hasTargets() const {
        for (const int item : m_targets) {
            if (item != 0) {
                return true;
            }
        }
        return false;
    }

    void ForageWindow::start() {
        if (m_foraging) {
            hide();
            return;
        }
        if (m_boneSlot == 0 || m_petPlace == 0) {
            return;
        }
        if (!m_marks.isAvailable(m_boneSlot)) {
            closeForm();
            return;
        }
        const std::optional<Pet>& pet = m_pets.at(m_petPlace);
        if (!pet || pet->info.npcId != m_petKind) {
            hide();
            reset();
            m_messages.show("No suitable pet", kShortMs);
            return;
        }
        findPet();
        if (m_petKind == 0) {
            hide();
            reset();
            m_messages.show("No suitable pet", kLongMs);
            return;
        }
        ForageFile file;
        file.items = m_targets;
        if (!file.write(ForageFile::pathFor(m_services.game.userRoot(), m_hooks.playerId()))) {
            core::logWarning("cannot write the Dog Bone's settings");
        }
        sendNext();
        hide();
    }

    void ForageWindow::cancel() {
        if (m_foraging) {
            m_services.connection.send(protocol::forageCancelRequest());
        }
        reset();
        hide();
    }

    void ForageWindow::closeForm() {
        if (m_foraging) {
            hide();
        } else {
            reset();
        }
    }

    void ForageWindow::reset() {
        m_targets.fill(0);
        m_foraging = false;
        m_boneUsed = false;
        for (int target = 1; target <= kTargets; ++target) {
            setTargetHint(target);
        }
        if (m_marks.useMark(m_boneSlot) == kBoneMark) {
            m_marks.setUseMark(m_boneSlot, 0);
        }
        m_petForaging[static_cast<std::size_t>(m_petPlace)] = false;
        m_boneSlot = 0;
        m_petKind = 0;
        m_petPlace = 0;
        m_current = 0;
        hide();
        m_hooks.marksChanged();
    }

    void ForageWindow::sendNext() {
        m_current = 0;
        bool found = false;
        for (int order = 1; order <= PetTeam::kPlaces; ++order) {
            const int place = m_pets.placeByOrder(order);
            const std::optional<Pet>& pet = place != 0 ? m_pets.at(place) : std::nullopt;
            if (!pet || pet->info.type != kPetType || pet->info.npcId != m_petKind || !m_hooks.petFree(place)) {
                continue;
            }
            m_petPlace = place;
            found = true;
            break;
        }
        if (!found) {
            std::string name;
            if (const auto info = m_services.npcs.find(m_petKind)) {
                name = info->name;
            }
            m_messages.show(name + "Not in your team", kShortMs);
            cancel();
            reset();
            hide();
            return;
        }
        if (!hasTargets()) {
            reset();
        }
        for (int target = 1; target <= kTargets; ++target) {
            const int itemId = m_targets[static_cast<std::size_t>(target)];
            if (itemId == 0 || !m_inventory.isStackable(itemId)) {
                continue;
            }
            const auto record = m_services.items.find(itemId);
            if (!record || (record->flags & kStackFlag) != 0) {
                continue;
            }
            m_marks.setUseMark(m_boneSlot, kBoneMark);
            m_petForaging[static_cast<std::size_t>(m_petPlace)] = true;
            m_current = itemId;
            m_targetCount = 0;
            for (int other = 1; other <= kTargets; ++other) {
                if (m_targets[static_cast<std::size_t>(other)] != 0) {
                    ++m_targetCount;
                }
            }
            ++m_ordinal;
            m_services.connection.send(protocol::forageStartRequest(m_boneSlot, m_petPlace, itemId, m_targetCount, m_ordinal));
            m_foraging = true;
            m_services.playSound("sound/wav0152.wav");
            m_messages.show("Pet goes to find " + record->name, kShortMs);
            m_targetIndex = target;
            m_targets[static_cast<std::size_t>(target)] = 0;
            break;
        }
        m_hooks.marksChanged();
    }

    void ForageWindow::found(int itemId, int count) {
        std::string pet;
        if (const auto info = m_services.npcs.find(m_petKind)) {
            pet = info->name;
        }
        const auto record = m_services.items.find(itemId);
        m_messages.show(pet + " find " + std::to_string(count) + "," + (record ? record->name : std::string()), kLongMs);
        clearTarget(m_targetIndex);
        sendNext();
        if (m_current == 0) {
            reset();
        }
    }

    void ForageWindow::markBone(int boneSlot) {
        if (boneSlot >= 1 && boneSlot <= BagMarks::kSlots && m_marks.useMark(boneSlot) == 0) {
            m_marks.setUseMark(boneSlot, kBoneMark);
            m_hooks.marksChanged();
        }
    }

    void ForageWindow::ended(int boneSlot, int petPlace, int code) {
        if (m_marks.useMark(boneSlot) == kBoneMark) {
            m_marks.setUseMark(boneSlot, 0);
        }
        if (petPlace >= 1 && petPlace <= PetTeam::kPlaces) {
            m_petForaging[static_cast<std::size_t>(petPlace)] = false;
        }
        reset();
        hide();
        m_messages.show(code == 1 ? "Searching failed" : "Cancel searching", kLongMs); // 23/147 code 1: search failed, otherwise cancelled
    }

    void ForageWindow::paintPet(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        if (m_petKind == 0 || m_petPlace == 0) {
            return;
        }
        const std::optional<Pet>& pet = m_pets.at(m_petPlace);
        if (!pet) {
            return;
        }
        const int direction = m_foraging ? kForagingSlot : kStandingSlot;
        const int slot = m_services.characters.npcSlot(pet->info.spriteId, pet->info.directionKind, direction);
        const core::Size size = m_services.characters.npcFrameSize(pet->info.spriteId, slot);
        const int rise = pet->info.spriteSize == 0 ? 16 : 50; // pixels the pet is lifted: small sprite 16, large sprite 50
        const int frame = m_animation.frame(nowMs, m_services.characters.npcFrameCount(pet->info.spriteId, slot));
        m_services.characters.drawNpc(canvas, pet->info, {at.x + m_petSpot.at().x, at.y + m_petSpot.at().y + size.height / 2 - rise}, slot, frame);
        const gfx::TextStyle style{m_textColorSpot.color(), gfx::TextEffect::Plain, std::nullopt};
        canvas.drawText({at.x + m_nameSpot.at().x, at.y + m_nameSpot.at().y}, pet->info.name, style, m_window.bounds().width, 15); // line height in pixels
    }

    void ForageWindow::paintTarget(gfx::Canvas& canvas, core::Point at, int target) {
        const int itemId = m_targets[static_cast<std::size_t>(target)];
        const auto record = itemId != 0 ? m_services.items.find(itemId) : std::nullopt;
        if (!record) {
            return;
        }
        if (const auto icon = m_services.pictures.find(std::to_string(record->iconId))) {
            canvas.drawPicture(*icon, at);
        }
    }

}
