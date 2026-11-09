#include "client/viewmodels/crafting/CompoundWindow.h"

#include "client/rendering/painters/ItemCells.h"
#include "client/rules/Compounding.h"
#include "core/FileSystem.h"
#include "core/Log.h"
#include "gfx/TextStyle.h"
#include "protocol/ItemRequests.h"

#include <string>

namespace client {

    namespace {

        constexpr int kCauldronFrames = 23; // frames of the "Compounding" cauldron animation
        constexpr int kResultFrame = 14; // cauldron frame at which the result icon is released
        constexpr double kFlightSpeed = 0.12; // result icon flight speed, px per ms
        constexpr int kClosedMap = 10004; // map 10004: Prison (no compounding there)
        constexpr int kJuniorSkill = 15998; // skill 15998: Junior Alchemy
        constexpr int kHighSkill = 15999; // skill 15999: Superior Alchemy
        constexpr int kPlain = 14; // compound request kind 14: plain compound
        constexpr int kJunior = 87; // compound request kind 87: junior alchemy
        constexpr int kHigh = 101; // compound request kind 101: high (superior) alchemy
        constexpr int kBless = 115; // compound request kind 115: the Bless Pack's items
        constexpr int kBlessType = 20; // item type 20: consumables, includes Bless Pack items (Blessing Strawberry, ...)
        constexpr int kBlessSubType = 149; // item sub type 149: Bless Pack role gear (Traveller's Hat, Arez's Armor, ...)
        constexpr int kBookSubType = 141; // item sub type 141: Alchemy Book 1..4
        constexpr int kJobManteauSubType = 142; // item sub type 142: job manteau (Killer Manteau, ...)
        constexpr int kGunType = 11; // item type 11: guns (Arquebus, Bazooka, ...)
        constexpr int kCrystalType = 54; // item type 54: Bomb Diamonds (10% Bomb Diamond, ...), mountable into weapons
        constexpr int kCriticalSubType = 137; // item sub type 137: weapons with critical strike (Steel Falchion, Sky Sword, ...)

        /// @brief Loads CompoundWindow.xml and returns the window.
        /// @param services Game services providing the layout and load context.
        /// @param screen Screen the layout is loaded into.
        /// @return The CompoundWindow window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("CompoundWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("CompoundWindow");
        }

        /// @brief Parses the number that follows a name prefix, such as the 3 of "CompoundSlot3".
        /// @param name Control name.
        /// @param prefix Expected prefix.
        /// @return The number, or 0 when the name does not start with the prefix.
        int numberAfter(const std::string& name, const std::string& prefix) { return name.rfind(prefix, 0) == 0 ? std::stoi(name.substr(prefix.size())) : 0; }

        /// @brief Whether an item is a gem or crystal for mounting.
        /// @param item Item record.
        /// @return True for item type 52 or the crystal type.
        bool isGem(const formats::ItemRecord& item) { return item.type == 52 || item.type == kCrystalType; } // 52: gem item type

        /// @brief Whether an item is worn equipment.
        /// @param item Item record.
        /// @return True when its equip slot is 1 to 6.
        bool isEquipment(const formats::ItemRecord& item) { return item.equipSlot >= 1 && item.equipSlot <= 6; } // equip slots 1..6 are worn gear

        /// @brief Gets the text after "name=" on the first line with it.
        /// @details Original: TStringList.Values.
        /// @param lines Settings file lines.
        /// @param name Key to look up.
        /// @return The value, or an empty string when absent.
        std::string valueOf(const std::vector<std::string>& lines, const std::string& name) {
            for (const std::string& line : lines) {
                if (line.rfind(name + "=", 0) == 0) {
                    return line.substr(name.size() + 1);
                }
            }
            return {};
        }

        /// @brief Sets "name=value" on the first line with that name, or appends a new line.
        /// @param lines Settings file lines.
        /// @param name Key to set.
        /// @param value New value.
        void setValue(std::vector<std::string>& lines, const std::string& name, const std::string& value) {
            for (std::string& line : lines) {
                if (line.rfind(name + "=", 0) == 0) {
                    line = name + "=" + value;
                    return;
                }
            }
            lines.push_back(name + "=" + value);
        }

    }

    CompoundWindow::CompoundWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        BagMarks& marks,
        const PlayerStatus& status,
        PlayerMessages& messages,
        TalkDialog& talk,
        std::function<void()> bagChanged,
        std::function<int()> mapId
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_marks(marks),
          m_status(status),
          m_messages(messages),
          m_talk(talk),
          m_bagChanged(std::move(bagChanged)),
          m_mapId(std::move(mapId)),
          m_window(loadWindow(services, screen)),
          m_bag(screen.get<ui::IconGrid>("CompoundBag")),
          m_cauldronSpot(screen.get<ui::Spot>("CompoundCauldron")),
          m_flightSpot(screen.get<ui::Spot>("CompoundFlight")),
          m_countSpot(screen.get<ui::Spot>("CompoundCount")) {
        screen.get<ui::DrawingArea>("CompoundPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) { paint(canvas, at, nowMs); });
        for (int slot = 1; slot <= kSlots; ++slot) {
            screen.get<ui::DrawingArea>("CompoundSlot" + std::to_string(slot)).setPainter([this, slot](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                paintSlot(canvas, at, slot);
            });
        }
        screen.on("onCompoundClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onCompoundBagClick", [this](const ui::EventArgs& args) { addFromBag(args.index); });
        screen.on("onCompoundBagRemove", [this](const ui::EventArgs& args) { removeByBagSlot(args.index); });
        screen.on("onCompoundSlotRemove", [this](const ui::EventArgs& args) { clearSlot(numberAfter(args.source, "CompoundSlot")); });
        screen.on("onCompoundSynthesis", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onCompoundJunior", [this](const ui::EventArgs&) {
            m_junior = !m_junior;
            showJunior();
        });
        screen.on("onCompoundSkillMenu", [this](const ui::EventArgs&) { m_skillMenuOpen = !m_skillMenuOpen; });
        screen.on("onCompoundSkillChosen", [this](const ui::EventArgs& args) { chooseSkill(numberAfter(args.source, "CompoundSkill")); });
    }

    void CompoundWindow::open() {
        loadSettings();
        m_hasJunior = hasSkill(kJuniorSkill);
        m_hasHigh = hasSkill(kHighSkill);
        showJunior();
        m_resultItem = 0;
        m_litCell = 0;
        m_resultShown = false;
        clearSlots();
        refresh();
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void CompoundWindow::close() {
        saveSettings();
        clearSlots();
        m_marks.clearUseMarks(BagMarks::kCompound);
        finishFlight();
        m_cauldron.stop();
        m_window.setVisible(false);
        m_bagChanged();
    }

    void CompoundWindow::toggle() {
        if (isOpen()) {
            close();
        } else {
            open();
        }
    }

    void CompoundWindow::refresh() {
        m_bag.clearCells();
        for (int slot = 1; slot <= Inventory::kBagSlots; ++slot) {
            const protocol::ItemInstance& item = m_inventory.bagItem(slot);
            if (!item.empty() && !m_marks.isHidden(slot)) {
                m_bag.setCell(slot, itemIconCell(m_services, item, true, m_marks.isBusy(slot)));
            }
        }
        for (int slot = 1; slot <= kSlots; ++slot) {
            Slot& content = m_slots[static_cast<std::size_t>(slot)];
            if (content.bagSlot == 0) {
                continue;
            }
            const protocol::ItemInstance& item = m_inventory.bagItem(content.bagSlot);
            if (item.empty()) {
                clearSlot(slot);
            } else {
                content.itemId = item.itemId;
                content.count = item.count;
            }
        }
    }

    void CompoundWindow::addFromBag(int bagSlot) {
        const protocol::ItemInstance& item = m_inventory.bagItem(bagSlot);
        if (item.empty() || !m_services.items.find(item.itemId)) {
            return;
        }
        // the messages stay up 2000 ms
        switch (compoundRefusal(m_services.items, item.itemId)) {
            case 0: // item may be combined
                break;
            case 2: // refusal 2: big size item
                m_messages.show("Big size item, Unable to combine", 2000);
                return;
            case 3: // refusal 3: not an item
                m_messages.show("It is not a item, Unable to combine", 2000);
                return;
            case 4: // refusal 4: main material is 0
                m_messages.show("Main material is 0, Unable to combine", 2000);
                return;
            default: // any other refusal: cannot be combined
                m_messages.show("This item cannot be combined", 2000);
                return;
        }
        for (int slot = 1; slot <= kSlots; ++slot) {
            Slot& content = m_slots[static_cast<std::size_t>(slot)];
            if (content.bagSlot == 0) {
                content = {bagSlot, item.itemId, item.count};
                m_marks.setUseMark(bagSlot, BagMarks::kCompound);
                refresh();
                m_bagChanged();
                return;
            }
        }
    }

    void CompoundWindow::clearSlot(int slot) {
        if (slot < 1 || slot > kSlots) {
            return;
        }
        Slot& content = m_slots[static_cast<std::size_t>(slot)];
        if (content.bagSlot == 0) {
            return;
        }
        if (m_marks.useMark(content.bagSlot) == BagMarks::kCompound) {
            m_marks.setUseMark(content.bagSlot, 0);
        }
        content = {};
        refresh();
        m_bagChanged();
    }

    void CompoundWindow::clearSlots() {
        for (int slot = 1; slot <= kSlots; ++slot) {
            clearSlot(slot);
        }
    }

    void CompoundWindow::removeByBagSlot(int bagSlot) {
        if (m_inventory.bagItem(bagSlot).empty() || m_marks.useMark(bagSlot) != BagMarks::kCompound) {
            return;
        }
        for (int slot = 1; slot <= kSlots; ++slot) {
            if (m_slots[static_cast<std::size_t>(slot)].bagSlot == bagSlot) {
                clearSlot(slot);
            }
        }
    }

    void CompoundWindow::confirm() {
        if (m_mapId() == kClosedMap) {
            m_messages.show("Unable to compoud while closing", 1500); // message duration in ms
            return;
        }
        std::vector<int> bagSlots;
        std::vector<int> equipment;
        int gems = 0;
        int gemSlot = 0;
        int books = 0;
        int manteaus = 0;
        int blessType = 0;
        int blessSubType = 0;
        for (int slot = 1; slot <= kSlots; ++slot) {
            const int bagSlot = m_slots[static_cast<std::size_t>(slot)].bagSlot;
            if (bagSlot == 0) {
                continue;
            }
            const protocol::ItemInstance& stack = m_inventory.bagItem(bagSlot);
            const auto item = m_services.items.find(stack.itemId);
            if (!item) {
                return;
            }
            if (compoundRefusal(m_services.items, item->id) != 0) {
                m_messages.show(item->name + "Unable to combine", 2000); // message duration in ms
                return;
            }
            blessType += item->type == kBlessType ? 1 : 0;
            blessSubType += item->subType == kBlessSubType ? 1 : 0;
            manteaus += item->subType == kJobManteauSubType ? 1 : 0;
            if (item->subType == kBookSubType && item->rank != 0) {
                if (slot == 1) {
                    m_messages.show("Main material cannot be alchemy", 2000); // message duration in ms
                    return;
                }
                if (stack.count != 1) {
                    m_messages.show("Only with an alchemy book", 2000); // message duration in ms
                    return;
                }
                ++books;
            }
            if (isEquipment(*item)) {
                equipment.push_back(bagSlot);
            } else if (isGem(*item)) {
                gems += stack.count;
                if (gems < 5) { // the gem slot is remembered while fewer than 5 gems are counted
                    gemSlot = bagSlot;
                }
            }
            bagSlots.push_back(bagSlot);
        }
        if (gems != 0) {
            blessSubType = 0;
        }
        const int used = static_cast<int>(bagSlots.size());
        if (used <= 1) {
            m_messages.show("You need at least two materials.", 2000); // message duration in ms
            return;
        }
        if (books >= 2) {
            m_messages.show("Only with an alchemy book", 2000); // message duration in ms
            return;
        }
        if (used - books <= 1) {
            m_messages.show("At least 2 materials are required except alchemy book", 2000); // message duration in ms
            return;
        }
        int kind = kPlain;
        if (blessType == 0 && blessSubType == 0) {
            if (m_hasHigh) {
                kind = m_skill == 3 ? kHigh : m_skill == 2 ? kJunior : kPlain; // skill menu choice 3: high, 2: junior, else plain
            } else if (m_hasJunior) {
                kind = m_junior ? kJunior : kPlain;
            }
        } else if (blessType == used || blessSubType == used) {
            kind = kBless;
        } else {
            m_messages.show("Items in Bless Pack can't be forged with other items. ", 2000); // message duration in ms
            clearSlots();
            m_marks.clearUseMarks(BagMarks::kCompound);
            m_bagChanged();
            return;
        }
        m_request = bagSlots;
        m_requestKind = kind;
        if (gems > 0) {
            // 3000: message duration in ms
            if (!(gems <= 1 && equipment.size() == 1 && used == 2)) {
                m_messages.show("To mount magic crystal stone needs a piece of unmounted equipment and 1 magic crystal stone", 3000);
                return;
            }
            const protocol::ItemInstance& target = m_inventory.bagItem(equipment.front());
            const auto record = m_services.items.find(target.itemId);
            const std::string name = "[ " + (record ? record->name : std::string()) + " ]";
            const auto gem = m_services.items.find(m_inventory.bagItem(gemSlot).itemId);
            if (target.insertItemId != 0) {
                m_messages.show(name + "Mounted", 2000); // message duration in ms
                return;
            }
            if (record && record->equipSlot == 6) { // equip slot 6: special items (only a job manteau may be set)
                if (record->subType != kJobManteauSubType) {
                    m_messages.show(name + "Special items cannot be set", 2000); // message duration in ms
                    return;
                }
            } else if (record && record->type == kGunType) {
                m_messages.show(name + "Guns cannot be set", 2000); // message duration in ms
                return;
            } else if (record && gem && gem->type == kCrystalType) {
                if (record->equipSlot != 3) { // equip slot 3: the weapon slot
                    m_messages.show(name + "It's not weapon, can't be embed with crystal", 2000); // message duration in ms
                    return;
                }
                if (record->subType == kCriticalSubType) {
                    m_messages.show(name + "Contains critical strike function, can't be embeded.", 2000); // message duration in ms
                    return;
                }
            }
        }
        if (manteaus > 0 && !(manteaus == 1 && gems == 1)) {
            m_messages.show("The class's manteau can only combine with devil's spar", 2000); // message duration in ms
            return;
        }
        for (const int bagSlot : equipment) {
            if (m_inventory.bagItem(bagSlot).insertItemId != 0) {
                m_talk.ask("Compound material mounted with magic crystal stone, continue? ", std::nullopt, [this] { send(); }, nullptr);
                return;
            }
        }
        send();
    }

    void CompoundWindow::send() {
        m_services.connection.send(protocol::compoundRequest(m_requestKind, m_request));
        LightEffect::Setup boil;
        boil.picture = m_services.pictures.find("Compounding").value_or(gfx::PictureId{});
        boil.frameMs = 200; // cauldron frame time in ms
        boil.firstFrame = 1;
        boil.lastFrame = kCauldronFrames;
        boil.frameCount = kCauldronFrames;
        boil.motion = LightEffect::Motion::Once;
        m_cauldron.start(boil, core::Point{m_window.bounds().x, m_window.bounds().y} + m_cauldronSpot.at(), m_nowMs);
        clearSlots();
        m_marks.clearUseMarks(BagMarks::kCompound);
        m_bagChanged();
    }

    void CompoundWindow::startIdleCauldron(std::uint64_t nowMs) {
        LightEffect::Setup idle;
        idle.picture = m_services.pictures.find("Compounding").value_or(gfx::PictureId{});
        idle.frameMs = 200; // cauldron frame time in ms
        idle.firstFrame = 1;
        idle.lastFrame = 1;
        idle.frameCount = kCauldronFrames;
        m_cauldron.start(idle, core::Point{m_window.bounds().x, m_window.bounds().y} + m_cauldronSpot.at(), nowMs);
    }

    void CompoundWindow::showResult(int itemId, int bagSlot) {
        if (m_litCell != 0) {
            finishFlight();
            m_resultItem = 0;
        }
        if (isOpen() && bagSlot >= 1 && bagSlot <= BagMarks::kSlots) {
            m_resultItem = itemId;
            m_litCell = bagSlot;
            m_marks.setHidden(bagSlot, true);
        }
        clearSlots();
        refresh();
        m_bagChanged();
    }

    void CompoundWindow::finishFlight() {
        if (m_litCell != 0) {
            m_marks.setHidden(m_litCell, false);
        }
        m_flying = false;
        m_flight.stop();
        m_litCell = 0;
        m_resultShown = false;
        refresh();
        m_bagChanged();
    }

    bool CompoundWindow::hasSkill(int skillId) const {
        const auto record = m_services.skills.find(skillId);
        return record && m_status.skills.count(record->key) != 0;
    }

    void CompoundWindow::showJunior() {
        m_screen.get<ui::ImageButton>("CompoundJuniorCheck")
            .setPicture(m_services.pictures.find(m_junior ? "btn_Check_1" : "btn_UnCheck_1").value_or(gfx::PictureId{}));
    }

    void CompoundWindow::chooseSkill(int skill) {
        if (skill >= 1 && skill <= 3) { // skill menu choices are 1..3
            m_skill = skill;
            m_skillMenuOpen = false;
        }
    }

    void CompoundWindow::layoutSkills() {
        const bool menu = m_hasHigh && m_hasJunior;
        const bool junior = m_hasJunior && !m_hasHigh;
        if (!junior) {
            m_junior = false;
            showJunior();
        }
        ui::TextBox& label = m_screen.get<ui::TextBox>("CompoundJuniorLabel");
        label.setVisible(junior);
        m_screen.get<ui::Element>("CompoundJuniorCheck").setVisible(junior);
        if (junior) {
            const auto skill = m_services.skills.find(kJuniorSkill);
            label.setText(skill ? "Use " + skill->name : "Use junior alchemy skill");
        }
        if (!menu) {
            m_skill = 0;
            m_skillMenuOpen = false;
        } else if (m_skill != 2 && m_skill != 3) { // any choice other than 2 and 3 falls back to 1
            m_skill = 1;
        }
        ui::ImageButton& button = m_screen.get<ui::ImageButton>("CompoundSkill");
        button.setVisible(menu);
        if (menu) {
            button.setPicture(m_services.pictures.find("Btn_CompSk_" + std::to_string(m_skill)).value_or(gfx::PictureId{}));
        }
        for (int i = 1; i <= 3; ++i) { // the three skill menu entries
            m_screen.get<ui::Element>("CompoundSkill" + std::to_string(i)).setVisible(menu && m_skillMenuOpen);
        }
    }

    void CompoundWindow::paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        m_nowMs = nowMs;
        refresh();
        layoutSkills();
        const auto draw = [&](const char* name, core::Point where) {
            if (const auto picture = m_services.pictures.find(name)) {
                canvas.drawPicture(*picture, where);
            }
        };
        if (m_skill != 0) {
            const core::Rect button = m_screen.get<ui::Element>("CompoundSkill").bounds();
            draw("Icon_UseCompoundSkill", {at.x + button.x - 38, at.y + button.y + 1}); // 38: mark x offset left of the skill button
            if (m_skillMenuOpen) {
                const core::Rect first = m_screen.get<ui::Element>("CompoundSkill1").bounds();
                draw("Icon_RoleSelectFrame", {at.x + first.x - 3, at.y + first.y - 43}); // menu frame offset from the first entry
                const core::Rect menu{at.x + 54, at.y - 17, 47, 84}; // hover area of the open skill menu (x, y, w, h)
                if (!menu.contains(m_screen.mouse())) {
                    m_skillMenuOpen = false;
                }
            }
        }
        if (!m_cauldron.isActive()) {
            startIdleCauldron(nowMs);
        }
        m_cauldron.setPosition(at + m_cauldronSpot.at());
        m_cauldron.update(nowMs);
        m_cauldron.draw(canvas, nowMs);
        if (!m_cauldron.isActive()) {
            startIdleCauldron(nowMs);
        }
        if (m_cauldron.frameIndex() == kResultFrame) {
            if (m_resultItem != 0) {
                const auto item = m_services.items.find(m_resultItem);
                LightEffect::Setup flight;
                // picture "1000": fallback icon when the item or its icon is unknown
                flight.picture = m_services.pictures.find(item ? std::to_string(item->iconId) : "1000")
                                     .value_or(m_services.pictures.find("1000").value_or(gfx::PictureId{}));
                flight.frameMs = static_cast<std::uint64_t>(m_litCell);
                flight.motion = LightEffect::Motion::ToTarget;
                flight.loop = false;
                flight.target =
                    at + m_bag.bounds().position() + m_bag.cellPosition(m_litCell) + core::Point{16, 32}; // centre of the bag cell's icon (32 x 32 cell)
                flight.speed = kFlightSpeed;
                m_flight.start(flight, at + m_flightSpot.at(), nowMs);
                m_flying = true;
                m_resultItem = 0;
            }
            m_resultShown = true;
        }
        if (m_flying) {
            m_flight.update(nowMs);
            m_flight.draw(canvas, nowMs);
            if (m_flight.atTarget()) {
                finishFlight();
            }
        }
        if (m_resultShown && m_cauldron.frameIndex() == 0 && !m_flying && m_resultItem != 0) {
            m_resultItem = 0;
            finishFlight();
        }
    }

    void CompoundWindow::paintSlot(gfx::Canvas& canvas, core::Point at, int slot) {
        const Slot& content = m_slots[static_cast<std::size_t>(slot)];
        ui::DrawingArea& area = m_screen.get<ui::DrawingArea>("CompoundSlot" + std::to_string(slot));
        if (content.bagSlot == 0) {
            area.setHint("");
            return;
        }
        const protocol::ItemInstance& item = m_inventory.bagItem(content.bagSlot);
        const ui::IconCell cell = itemIconCell(m_services, item, true);
        area.setHint(cell.hint);
        const int down = slot == 1 ? 4 : 0; // the main slot's icon sits 4 pixels down
        if (cell.picture.isValid()) {
            canvas.drawPicture(cell.picture, {at.x, at.y + down}, core::Rect{0, 0, 32, 32}); // icon cell size in pixels
        }
        // count at x 44; y 17 for the main slot, 11 for the others; 66: text width
        canvas.drawText(
            {at.x + 44, at.y + (slot == 1 ? 17 : 11)},
            std::to_string(content.count),
            {m_countSpot.color(), gfx::TextEffect::Plain, std::nullopt},
            66
        );
    }

    void CompoundWindow::loadSettings() {
        const auto lines = core::readLines(m_settingsFile);
        if (!lines || valueOf(*lines, "User") != std::to_string(m_playerId)) {
            return;
        }
        const std::string middle = valueOf(*lines, "MiddleCompoundUse");
        const std::string high = valueOf(*lines, "HighCompoundUse");
        if (!middle.empty()) {
            m_junior = std::stoi(middle) > 0;
            if ((high.empty() || high == "0") && m_junior) {
                m_skill = 2; // skill menu choice 2: junior
            }
        }
        if (!high.empty() && high != "0") {
            m_skill = std::stoi(high);
        }
    }

    void CompoundWindow::saveSettings() const {
        if (m_settingsFile.empty()) {
            return;
        }
        std::vector<std::string> lines = core::readLines(m_settingsFile).value_or(std::vector<std::string>{});
        if (std::filesystem::exists(m_settingsFile)) {
            setValue(lines, "MiddleCompoundUse", m_junior ? "1" : "0");
            setValue(lines, "HighCompoundUse", std::to_string(m_skill));
        }
        if (!core::writeLines(m_settingsFile, lines)) {
            core::logWarning("cannot write ", m_settingsFile.string());
        }
    }

}
