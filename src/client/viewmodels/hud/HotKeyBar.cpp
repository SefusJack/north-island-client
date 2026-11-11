#include "client/viewmodels/hud/HotKeyBar.h"

#include "client/rules/FormDrag.h"
#include "client/rules/Skills.h"
#include "protocol/HotKeyRequests.h"

#include <string>

namespace client {

    namespace {

        constexpr int kItem = 1; // slot kind 1: an item
        constexpr int kSkill = 2; // slot kind 2: a skill
        constexpr int kAttack = 10001; // skill 10001: Hand Attack (the general attack)
        constexpr int kDefend = 60021; // skill 60021: Defense
        constexpr int kUpright = 1; // bar layout 1: upright (vertical)
        constexpr int kLying = 2; // bar layout 2: lying (horizontal)
        constexpr int kMinimized = 3; // bar layout 3: minimized to a small button
        constexpr int kMinimizeInMini = 10; // minimize button position inside the minimized bar, px (x and y)

        /// @brief Builds a slot's hint text: the item's name and what its two attributes give.
        /// @details Original: SetHotKeySlotImage.
        /// @param item Item record.
        /// @return Name followed by the attribute bonuses.
        std::string itemHint(const formats::ItemRecord& item) {
            std::string bonus;
            for (std::size_t k = 0; k < item.attributeKinds.size(); ++k) {
                std::string stat;
                switch (item.attributeKinds[k]) {
                    case 25: stat = "HP"; break; // attribute kind 25: HP
                    case 26: stat = "SP"; break; // attribute kind 26: SP
                    case 207: stat = "MaxHp"; break; // attribute kind 207: max HP
                    case 208: stat = "MaxSp"; break; // attribute kind 208: max SP
                    case 210: stat = "ATK"; break; // attribute kind 210: attack
                    case 211: stat = "DEF"; break; // attribute kind 211: defence
                    case 215: stat = "MATK"; break; // attribute kind 215: magic attack
                    case 216: stat = "MDEF"; break; // attribute kind 216: magic defence
                    case 214: stat = "SPD"; break; // attribute kind 214: speed
                    case 100: stat = "resistance of shoot"; break; // attribute kind 100: shoot resistance (shown with %)
                    case 250: continue; // attribute kind 250: not shown
                    default: break; // other attribute kinds: no label
                }
                if (!stat.empty()) {
                    stat += ": ";
                }
                const std::int32_t value = item.attributeValues[k];
                // attribute values are stored offset by 100 (100 = no change), 0 = unused
                if (value != 0 && value != 100) {
                    bonus += " " + stat + (value > 100 ? "+" + std::to_string(value - 100) : "-" + std::to_string(100 - value));
                    if (item.attributeKinds[k] == 100) { // shoot resistance kind: percent
                        bonus += "%";
                    }
                }
            }
            return item.name + bonus;
        }

    }

    HotKeyBar::HotKeyBar(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_panel(screen.load(services.game.layout("HotKeyBar.xml"), services.loadContext())),
          m_pageNumber(screen.get<ui::ImageView>("HotKeyPage")),
          m_grid(screen.get<ui::IconGrid>("HotKeySlots")),
          m_background(screen.get<ui::ImageView>("HotKeyBackground")),
          m_frame(screen.get<ui::Element>("HotKeyFrame")),
          m_previous(screen.get<ui::ImageButton>("HotKeyPagePrevious")),
          m_next(screen.get<ui::ImageButton>("HotKeyPageNext")),
          m_minimize(screen.get<ui::ImageButton>("HotKeyMinimize")) {
        screen.on("onHotKeyFramePress", [this](const ui::EventArgs& args) {
            m_grab = args.position;
            m_dragging = true;
        });
        screen.on("onHotKeyFrameDrag", [this](const ui::EventArgs& args) {
            if (!m_dragging) {
                return;
            }
            const core::Rect frame = m_frame.bounds();
            const FormDrag::HotKeyMove move =
                FormDrag::dragHotKeyBar(m_layout, m_previousLayout, m_grab, {frame.x + args.position.x, frame.y + args.position.y}, windowArea());
            m_layout = move.layout;
            m_previousLayout = move.previousLayout;
            m_grab = move.grab;
            m_origin = move.area.position();
            place();
        });
        screen.on("onHotKeyFrameRelease", [this](const ui::EventArgs&) { m_dragging = false; });
        place();
        screen.on("onHotKeyMinimize", [this](const ui::EventArgs&) { toggleMinimized(); });
        screen.on("onHotKeyPagePrevious", [this](const ui::EventArgs&) { turn(-1); });
        screen.on("onHotKeyPageNext", [this](const ui::EventArgs&) { turn(1); });
        screen.on("onHotKeyClear", [this](const ui::EventArgs& args) {
            constexpr int kNoBarMap = 10701; // map 10701: the jail (name not in the tables; the original says "in the jail")
            if (m_hooks.mapId() == kNoBarMap) {
                m_messages.show("Unable to use the function here.", 1200); // message duration in ms
                return;
            }
            clear(args.index, m_page);
            send(args.index, m_page);
        });
        screen.on("onHotKeyDrop", [this](const ui::EventArgs& args) { dropped(args); });
        screen.on("onHotKeyUse", [this](const ui::EventArgs& args) { use(args.index); });
    }

    void HotKeyBar::place() {
        const bool lying = m_layout == kLying;
        const bool minimized = m_layout == kMinimized;
        const core::Size size = FormDrag::hotKeySize(m_layout);
        const auto at = [this](int x, int y) { return core::Point{m_origin.x + x, m_origin.y + y}; };
        const auto picture = [this](const char* name, gfx::PictureId current) { return m_services.pictures.find(name).value_or(current); };
        const auto button = [&](ui::ImageButton& control, const char* name, core::Point spot) {
            control.setPicture(picture(name, control.picture()), m_services.pictures);
            control.setPosition(spot);
        };
        m_background.setPicture(picture(minimized ? "Form_HotKey_Mini" : lying ? "Form_HotKey_H" : "Form_HotKey_V", gfx::PictureId{}));
        m_background.setBounds({m_origin.x, m_origin.y, size.width, size.height});
        m_frame.setBounds({m_origin.x, m_origin.y, size.width, size.height});
        m_grid.setVisible(!minimized);
        m_previous.setVisible(!minimized);
        m_next.setVisible(!minimized);
        m_pageNumber.setVisible(!minimized);
        if (minimized) {
            button(m_minimize, "Btn_Window_1", at(kMinimizeInMini, kMinimizeInMini));
        } else if (lying) {
            m_grid.setColumns(kSlots, 1);
            // lying layout: offsets from the bar origin of the slot grid, minimize button, arrows and page number
            m_grid.setPosition(at(32, 23));
            button(m_minimize, "Btn_Minimize_1", at(255, 38));
            button(m_previous, "Btn_ArrowUp_2", at(17, 12));
            button(m_next, "Btn_ArrowDn_2", at(17, 36));
            m_pageNumber.setPosition(at(15, 22));
        } else {
            m_grid.setColumns(1, kSlots);
            // upright layout: offsets from the bar origin of the slot grid, minimize button, arrows and page number
            m_grid.setPosition(at(23, 29));
            button(m_minimize, "Btn_Minimize_1", at(37, 0));
            button(m_previous, "Btn_ArrowL_3", at(12, 248));
            button(m_next, "Btn_ArrowR_3", at(36, 248));
            m_pageNumber.setPosition(at(23, 245));
        }
    }

    void HotKeyBar::toggleMinimized() {
        const core::Rect button = m_minimize.bounds();
        m_dragging = false;
        if (m_layout != kMinimized) {
            const core::Size mini = FormDrag::hotKeySize(kMinimized);
            m_origin = {button.x - (mini.width - button.width) / 2, button.y - (mini.height - button.height) / 2};
            m_previousLayout = m_layout;
            m_layout = kMinimized;
        } else {
            // minimize button offsets: (255, 38) lying, (37, 0) upright
            const core::Point spot = m_previousLayout == kLying ? core::Point{255, 38} : core::Point{37, 0};
            m_origin = {button.x - spot.x, button.y - spot.y};
            m_layout = m_previousLayout == kLying ? kLying : kUpright;
            m_previousLayout = kMinimized;
        }
        const core::Size size = FormDrag::hotKeySize(m_layout);
        m_origin = FormDrag::keepHotKeyBar({m_origin.x, m_origin.y, size.width, size.height}, windowArea()).position();
        place();
    }

    void HotKeyBar::load(const std::vector<protocol::HotKeyRecord>& records) {
        for (const protocol::HotKeyRecord& record : records) {
            if (record.kind == kItem) {
                setItem(record.slot, record.id, false, record.page);
            } else if (record.kind == kSkill) {
                setSkill(record.slot, record.id, false, record.page);
            }
        }
    }

    void HotKeyBar::putItem(int slot, int itemId) { setItem(slot, itemId, true, std::nullopt); }

    void HotKeyBar::putSkill(int slot, int skillId) { setSkill(slot, skillId, true, std::nullopt); }

    void HotKeyBar::setItem(int slot, int itemId, bool sendIt, std::optional<int> page) {
        const int onPage = page.value_or(m_page);
        if (slot < 1 || slot > kSlots || onPage < 1 || onPage > kPages || m_hooks.bagSlotOf(itemId) == 0) {
            return;
        }
        const auto item = m_services.items.find(itemId);
        if (!item) {
            return;
        }
        constexpr int kEscapeButton = 34164; // item 34164: Escape Button
        // item ids of the reset scrolls: 34089 Forgotten Scroll, 34122 Lethe Scroll, 34137 Pet Lethe Scroll, 34139 Pet Return Scroll
        const bool resetScroll = itemId == 34089 || itemId == 34122 || itemId == 34137 || itemId == 34139;
        // item types 17 (with an alias item) and 26 are gift boxes
        const bool giftBox = (item->type == 17 && item->aliasItemId != 0) || item->type == 26;
        // item ids of the bamboo dragonflies: 34169 Bamboo Dragonfly, 34332 Mini Dragonfly
        const bool bambooDragonfly = itemId == 34169 || itemId == 34332;
        // item sub type 139 cannot be placed on the bar
        const bool notPlaceable = !m_hooks.isStackable(itemId) || resetScroll || item->subType == 139 || giftBox || bambooDragonfly;
        if (notPlaceable && itemId != kEscapeButton) {
            m_messages.show("Unable to lay the item", 1000); // message duration in ms
            return;
        }
        m_slots[static_cast<std::size_t>(onPage)][static_cast<std::size_t>(slot)] = {kItem, itemId};
        if (onPage == m_page) {
            showSlot(slot, false);
        }
        if (sendIt) {
            send(slot, onPage);
        }
    }

    void HotKeyBar::setSkill(int slot, int skillId, bool sendIt, std::optional<int> page) {
        const int onPage = page.value_or(m_page);
        if (slot < 1 || slot > kSlots || onPage < 1 || onPage > kPages) {
            return;
        }
        if (skillId != kAttack && skillId != kDefend) {
            if (isRoleSkill(skillId)) {
                skillId = kRoleSkillPlaceholder;
            }
            const bool playerMisses = !m_hooks.playerKnows(skillId);
            if (skillId == kRoleSkillPlaceholder) {
                skillId = m_hooks.roleSkillId();
            }
            if (playerMisses && !m_hooks.petKnows(skillId)) {
                return;
            }
        }
        m_slots[static_cast<std::size_t>(onPage)][static_cast<std::size_t>(slot)] = {kSkill, skillId};
        if (onPage == m_page) {
            showSlot(slot, false);
        }
        if (sendIt) {
            send(slot, onPage);
        }
    }

    void HotKeyBar::clear(int slot, int page) {
        if (slot < 1 || slot > kSlots || page < 1 || page > kPages) {
            return;
        }
        m_slots[static_cast<std::size_t>(page)][static_cast<std::size_t>(slot)] = {};
        if (page == m_page) {
            m_grid.clearCell(slot);
        }
    }

    void HotKeyBar::send(int slot, int page) {
        if (slot < 1 || slot > kSlots || page < 1 || page > kPages) {
            return;
        }
        const HotKeySlot& held = m_slots[static_cast<std::size_t>(page)][static_cast<std::size_t>(slot)];
        m_services.connection.send(protocol::hotKeyRequest({held.kind, held.id, page, slot}));
    }

    void HotKeyBar::dropped(const ui::EventArgs& args) {
        const int from = args.index;
        if (from < 1 || from > kSlots) {
            return;
        }
        auto& page = m_slots[static_cast<std::size_t>(m_page)];
        if (args.target == "HotKeySlots") {
            const int to = args.targetIndex;
            if (to < 1 || to > kSlots || to == from) {
                return;
            }
            std::swap(page[static_cast<std::size_t>(from)], page[static_cast<std::size_t>(to)]);
            showSlot(from, false);
            showSlot(to, false);
            send(from, m_page);
            send(to, m_page);
        } else if (args.target.rfind("PlusKey", 0) == 0) {
            const HotKeySlot& held = page[static_cast<std::size_t>(from)];
            if (held.kind == kSkill && m_putOnPlus) {
                m_putOnPlus(std::stoi(args.target.substr(7)), held.id); // 7 = length of the "PlusKey" prefix
            }
        } else if (args.target.empty()) {
            clear(from, m_page);
            send(from, m_page);
        }
    }

    void HotKeyBar::turn(int step) {
        m_page = (m_page - 1 + step + kPages) % kPages + 1;
        m_pageNumber.setPicture(m_services.pictures.find("HotKeyPage_" + std::to_string(m_page)).value_or(gfx::PictureId{}));
        for (int slot = 1; slot <= kSlots; ++slot) {
            showSlot(slot, false);
        }
    }

    void HotKeyBar::showSlot(int slot, bool grey) {
        const HotKeySlot& held = m_slots[static_cast<std::size_t>(m_page)][static_cast<std::size_t>(slot)];
        std::optional<gfx::PictureId> picture;
        std::string hint;
        if (held.kind == kItem) {
            if (const auto item = m_services.items.find(held.id)) {
                picture = m_services.pictures.find(std::to_string(item->iconId));
                hint = itemHint(*item);
            }
        } else if (held.kind == kSkill) {
            if (const auto skill = m_services.skills.find(held.id)) {
                picture = m_services.pictures.find("icon_sk" + std::to_string(skill->iconId) + (grey ? "s_g" : "s"));
                hint = held.id == kAttack ? "General attack" : held.id == kDefend ? skill->name : skill->name + " SP:" + std::to_string(skill->spCost);
            }
        }
        if (held.id == 0 || !picture) {
            m_grid.clearCell(slot);
            return;
        }
        constexpr int kIconSize = 24; // icon cell size, px (square)
        const core::Point from = held.kind == kItem ? core::Point{4, 3} : core::Point{0, 0}; // item icon source offset (4, 3)
        m_grid.setCell(slot, ui::IconCell{*picture, {from.x, from.y, kIconSize, kIconSize}, {}, std::nullopt, hint});
    }

    void HotKeyBar::update() {
        for (int slot = 1; slot <= kSlots; ++slot) {
            const HotKeySlot held = m_slots[static_cast<std::size_t>(m_page)][static_cast<std::size_t>(slot)];
            if (held.kind == kItem) {
                if (m_hooks.bagSlotOf(held.id) == 0) {
                    clear(slot, m_page);
                }
            } else if (held.kind == kSkill) {
                const std::optional<bool> usable = m_hooks.usableInBattle(held.id);
                showSlot(slot, usable.has_value() && !*usable);
            }
        }
    }

    void HotKeyBar::use(int slot) {
        if (slot < 1 || slot > kSlots) {
            return;
        }
        const HotKeySlot held = m_slots[static_cast<std::size_t>(m_page)][static_cast<std::size_t>(slot)];
        if (held.id == 0) {
            return;
        }
        constexpr int kNoBarMap = 10701; // map 10701: the jail (name not in the tables)
        if (m_hooks.mapId() == kNoBarMap) {
            m_messages.show("Unable to use the function here.", 1200); // message duration in ms
            return;
        }
        if (held.kind == kItem) {
            useItem(held.id);
        } else if (held.kind == kSkill) {
            useSkill(held.id);
        }
    }

    void HotKeyBar::useItem(int itemId) {
        const int bagSlot = m_hooks.bagSlotOf(itemId);
        const auto item = m_services.items.find(itemId);
        if (bagSlot == 0 || !item) {
            return;
        }
        const bool fighting = m_hooks.fighting();
        constexpr int kBattleScroll = 24; // item type 24: battle-only throwables (Ammonia, Bomb, Bowling, ...)
        switch (item->subType) {
            case 0: // item sub type 0: usable item, battle command / target cursor after the checks
            case 25: // item sub type 25: same checks as sub type 0
            case 54: // item sub type 54: same checks as sub type 0
            case 123: // item sub type 123: same checks as sub type 0
            case 124: // item sub type 124: same checks as sub type 0
            case 125: { // item sub type 125: same checks as sub type 0
                constexpr int kEscapeButton = 34164; // item 34164: Escape Button
                if ((item->type == kBattleScroll || itemId == kEscapeButton) && !fighting) {
                    m_messages.show("The item only can be used in battle.", 1500); // message duration in ms
                    return;
                }
                // item types 17 (with an alias item) and 26 are gift boxes
                if ((item->type == 17 && item->aliasItemId != 0) || item->type == 26) {
                    m_messages.show("Cannot use gift box in shortcut bar", 1500); // message duration in ms
                    return;
                }
                if (item->attributeKinds[0] == 0 && item->attributeKinds[1] == 0) {
                    m_messages.show("Don't need it now", 1500); // message duration in ms
                    return;
                }
                break;
            }
            case 58: // item sub type 58: fireworks, only a message
                m_messages.show("Fireworks effect", 1500); // message duration in ms
                return;
            case 113: // item sub type 113: battle command / target cursor
            case 114: // item sub type 114: battle command / target cursor
            case 118: // item sub type 118: battle command / target cursor
                break;
            case 120: // item sub type 120: refused in combat, else used from the bag
            case 121: // item sub type 121: refused in combat, else used from the bag
            case 127: // item sub type 127: refused in combat, else used from the bag
            case 128: // item sub type 128: refused in combat, else used from the bag
                if (fighting) {
                    m_messages.show(item->subType == 120 ? "Unable to use" : "Unable to use in combat", 1000); // message duration in ms
                    return;
                }
                m_hooks.useItem(bagSlot);
                return;
            case 130: // item sub type 130: nothing happens
                return;
            default: // other sub types: used at once from the bag
                m_hooks.useItem(bagSlot);
                return;
        }
        if (!fighting) {
            if (item->type == kBattleScroll) {
                m_messages.show("[ " + item->name + " ]Can only be used in combat", 1500); // message duration in ms
                return;
            }
            m_hooks.aim(kItem, itemId);
        } else {
            m_hooks.battleItem(itemId, bagSlot);
        }
    }

    void HotKeyBar::useSkill(int skillId) {
        if (m_hooks.fighting()) {
            m_hooks.battleSkill(skillId);
            return;
        }
        const auto skill = m_services.skills.find(skillId);
        if (!skill) {
            return;
        }
        const int asked = isRoleSkill(skillId) ? kRoleSkillPlaceholder : skillId;
        constexpr int kProfessional = 9; // skill category 9: professional (job) skills
        constexpr int kWaterGlaze = 17011; // skill 17011: Najas's Memory
        // skill types 6, 7 and 14 (aid skills) are the ones usable in the world
        if (!(skill->skillType == 6 || skill->skillType == 7 || skill->skillType == 14) || skillId == kWaterGlaze) {
            // last argument (1200) is the message duration in ms
            m_messages.show(
                skill->category == kProfessional && skillId != kWaterGlaze ? "It is a professional skill." : "The skill can only be used in battle.",
                1200
            );
            return;
        }
        const auto known = m_services.skills.find(asked);
        if (!known || known->key == 0 || !m_hooks.playerKnows(asked)) {
            m_messages.show("Unable to use this skill", 1500); // message duration in ms
            return;
        }
        if (m_hooks.sp() < skill->spCost) {
            m_messages.show("not enough SP", 1200); // message duration in ms
            return;
        }
        if (asked == kRoleSkillPlaceholder) {
            if (const auto clothes = m_hooks.roleClothesNeeded()) {
                const auto needed = m_services.items.find(*clothes);
                m_messages.show("Use [" + skill->name + "]need equipment[" + (needed ? needed->name : std::string()) + "]", 1500); // message duration in ms
                return;
            }
        }
        m_hooks.aim(kSkill, asked);
    }

}
