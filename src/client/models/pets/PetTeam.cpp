#include "client/models/pets/PetTeam.h"

#include "client/rules/Vitality.h"

#include <algorithm>

namespace client {

    namespace {

        // attribute ids of the 8/2 attribute update packet
        constexpr int kHp = 25; // current HP
        constexpr int kSp = 26; // current SP
        constexpr int kIntelligence = 27; // INT
        constexpr int kStrength = 28; // STR
        constexpr int kConstitution = 29; // CON
        constexpr int kAgility = 30; // AGI
        constexpr int kWisdom = 33; // WIS
        constexpr int kFirstCombat = 41; // ATK; 42 DEF, 43 MATK, 44 MDEF, 45 SPD follow
        constexpr int kLastCombat = 45; // SPD
        constexpr int kLevel = 35; // level
        constexpr int kExp = 36; // experience
        constexpr int kAmity = 64; // amity
        constexpr int kSkillLevel = 110; // level of the skill whose id is extra
        constexpr int kSkillExp = 111; // experience of the skill whose id is extra
        constexpr int kMaxHp = 205; // most HP
        constexpr int kMaxSp = 206; // most SP
        constexpr int kMaxHpBonus = 207; // equipment bonus to the most HP
        constexpr int kMaxSpBonus = 208; // equipment bonus to the most SP
        constexpr int kAtkBonus = 210; // ATK bonus
        constexpr int kDefBonus = 211; // DEF bonus
        constexpr int kSpdBonus = 214; // SPD bonus
        constexpr int kMatkBonus = 215; // MATK bonus
        constexpr int kMdefBonus = 216; // MDEF bonus
        constexpr int kEquipmentSlots = 6; // 1 head, 2 body, 3 weapon, 4 wrist, 5 feet, 6 special
        constexpr int kMostSp = 65535; // largest SP value (16-bit field)

        /// @brief Tells whether a place number is 1..kPlaces.
        /// @param place Place number.
        /// @return True when valid.
        bool validPlace(int place) { return place >= 1 && place <= PetTeam::kPlaces; }

        /// @brief Maps a place to its array index (0 for an invalid place).
        /// @param place Place number.
        /// @return The index into the places array.
        std::size_t index(int place) { return validPlace(place) ? static_cast<std::size_t>(place) : 0; }

    }

    std::optional<Pet> PetTeam::fromRecord(const protocol::PetRecord& record) const {
        const auto info = m_npcs.find(record.npcId);
        if (!info) {
            return std::nullopt;
        }
        Pet pet;
        pet.info = *info;
        pet.name = record.name.empty() ? info->name : record.name;
        pet.level = record.level;
        pet.reborn = record.reborn;
        pet.exp = record.exp;
        pet.hp = record.hp;
        pet.sp = record.sp;
        pet.strength = record.strength;
        pet.constitution = record.constitution;
        pet.intelligence = record.intelligence;
        pet.wisdom = record.wisdom;
        pet.agility = record.agility;
        pet.amity = record.amity;
        pet.tradable = record.tradable;
        pet.potential = record.potential;
        pet.training = record.training;
        pet.skillLevels = record.skillLevels;
        pet.skillExp = record.skillExp;
        std::vector<protocol::ItemInstance> worn = record.worn;
        if (worn.empty()) {
            for (const int itemId : record.equipment) {
                protocol::ItemInstance item;
                item.itemId = itemId;
                item.count = 1;
                worn.push_back(item);
            }
        }
        for (const protocol::ItemInstance& item : worn) {
            wear(pet, item);
        }
        recalc(pet);
        return pet;
    }

    void PetTeam::recalc(Pet& pet) const {
        const int level = totalLevel(pet.level, pet.reborn);
        pet.maxHp = maxHp(m_formula, level, pet.constitution, pet.maxHpBonus);
        pet.maxSp = maxSp(m_formula, level, pet.wisdom, pet.maxSpBonus);
        pet.recalcStats = {pet.strength, pet.constitution, pet.intelligence, pet.wisdom, pet.agility};
        pet.recalcBonus = pet.combatBonus;
    }

    void PetTeam::recalcMaxima(Pet& pet) const {
        const int level = totalLevel(pet.level, pet.reborn);
        pet.maxHp = maxHp(m_formula, level, pet.constitution, pet.maxHpBonus);
        pet.maxSp = maxSp(m_formula, level, pet.wisdom, pet.maxSpBonus);
    }

    void PetTeam::setList(const std::vector<protocol::PetRecord>& records) {
        for (const protocol::PetRecord& record : records) {
            if (!validPlace(record.slot) || placeOf(record.npcId) != 0) {
                continue;
            }
            auto pet = fromRecord(record);
            if (!pet) {
                continue;
            }
            pet->order = std::min(count() + 1, kPlaces);
            m_places[index(record.slot)] = std::move(pet);
        }
    }

    int PetTeam::join(const protocol::PetRecord& record) {
        if (placeOf(record.npcId) != 0) {
            return 0;
        }
        for (int place = 1; place <= kPlaces; ++place) {
            if (m_places[index(place)]) {
                continue;
            }
            auto pet = fromRecord(record);
            if (!pet) {
                return 0;
            }
            pet->order = count() + 1;
            pet->hp = pet->maxHp;
            pet->sp = pet->maxSp;
            m_places[index(place)] = std::move(pet);
            return place;
        }
        return 0;
    }

    bool PetTeam::put(int place, const protocol::PetRecord& record) {
        if (!validPlace(place)) {
            return false;
        }
        auto pet = fromRecord(record);
        if (!pet) {
            return false;
        }
        pet->order = m_places[index(place)] ? m_places[index(place)]->order : 0;
        m_places[index(place)] = std::move(pet);
        return true;
    }

    void PetTeam::remove(int place) {
        if (!validPlace(place) || !m_places[index(place)]) {
            return;
        }
        if (m_activePlace == place) {
            m_activePlace = 0;
        }
        const int order = m_places[index(place)]->order;
        m_places[index(place)].reset();
        for (auto& pet : m_places) {
            if (pet && pet->order > order) {
                --pet->order;
            }
        }
    }

    void PetTeam::clear() {
        for (auto& pet : m_places) {
            pet.reset();
        }
        m_activePlace = 0;
        m_cityBattleDead = {};
    }

    void PetTeam::replaceWornIds(int first, int last, int to) {
        for (auto& pet : m_places) {
            if (!pet) {
                continue;
            }
            for (auto& item : pet->worn) {
                if (item.itemId >= first && item.itemId <= last) {
                    item.itemId = to;
                }
            }
        }
    }

    void PetTeam::setCityBattleDead(int place, bool dead) {
        if (validPlace(place)) {
            m_cityBattleDead[static_cast<std::size_t>(place)] = dead;
        }
    }

    void PetTeam::setActive(int npcId) { m_activePlace = npcId == 0 ? 0 : placeOf(npcId); }

    void PetTeam::setAttribute(int place, int attribute, std::int32_t value, std::int32_t extra) {
        if (!validPlace(place) || !m_places[index(place)]) {
            return;
        }
        Pet& pet = *m_places[index(place)];
        if (attribute >= kFirstCombat && attribute <= kLastCombat) { // 8/2 attributes 41-45: the pet's shown values as the server works them out
            pet.serverCombat[static_cast<std::size_t>(attribute - kFirstCombat)] = value;
            pet.serverCombatKnown = true;
            return;
        }
        switch (attribute) {
            case kHp: // 8/2 attribute 25: current HP
                pet.hp = std::clamp(value, 0, std::max(pet.maxHp, 0));
                break;
            case kSp: // 8/2 attribute 26: current SP
                pet.sp = std::clamp(value, 0, std::clamp(pet.maxSp, 0, kMostSp));
                break;
            case kStrength: // 8/2 attribute 28: STR
                pet.strength = value;
                break;
            case kIntelligence: // 8/2 attribute 27: INT
                pet.intelligence = value;
                break;
            case kAgility: // 8/2 attribute 30: AGI
                pet.agility = value;
                break;
            case kConstitution: // 8/2 attribute 29: CON
                pet.constitution = value;
                break;
            case kWisdom: // 8/2 attribute 33: WIS (the most SP follows)
                pet.wisdom = value;
                pet.maxSp = maxSp(m_formula, totalLevel(pet.level, pet.reborn), pet.wisdom, pet.maxSpBonus);
                break;
            case kLevel: // 8/2 attribute 35: level (recalculates)
                pet.level = value;
                recalc(pet);
                break;
            case kExp: // 8/2 attribute 36: experience
                pet.exp = value;
                break;
            case kAmity: // 8/2 attribute 64: amity
                pet.amity = value;
                break;
            case kSkillLevel: // 8/2 attribute 110: level of the skill whose id is extra
                for (std::size_t i = 0; i < pet.skillLevels.size(); ++i) {
                    if (pet.info.skills[i] == extra) {
                        pet.skillLevels[i] = value;
                    }
                }
                break;
            case kSkillExp: // 8/2 attribute 111: experience of the skill whose id is extra
                for (std::size_t i = 0; i < pet.skillExp.size(); ++i) {
                    if (pet.info.skills[i] == extra) {
                        pet.skillExp[i] = value;
                    }
                }
                break;
            case kMaxHp: // 8/2 attribute 205: most HP
                pet.maxHp = value;
                pet.hp = std::min(pet.hp, pet.maxHp);
                break;
            case kMaxSp: // 8/2 attribute 206: most SP
                pet.maxSp = value;
                pet.sp = std::min(pet.sp, pet.maxSp);
                break;
            case kMaxHpBonus: // 8/2 attribute 207: equipment bonus to the most HP
                pet.maxHpBonus = value;
                recalcMaxima(pet);
                break;
            case kMaxSpBonus: // 8/2 attribute 208: equipment bonus to the most SP
                pet.maxSpBonus = value;
                recalcMaxima(pet);
                break;
            case kAtkBonus: // 8/2 attribute 210: ATK bonus
                pet.combatBonus[0] = value;
                break;
            case kDefBonus: // 8/2 attribute 211: DEF bonus
                pet.combatBonus[1] = value;
                break;
            case kMatkBonus: // 8/2 attribute 215: MATK bonus
                pet.combatBonus[2] = value; // index 2 = MATK
                break;
            case kMdefBonus: // 8/2 attribute 216: MDEF bonus
                pet.combatBonus[3] = value; // index 3 = MDEF
                break;
            case kSpdBonus: // 8/2 attribute 214: SPD bonus
                pet.combatBonus[4] = value; // index 4 = SPD
                break;
            default: // other attributes are ignored
                return;
        }
        switch (attribute) {
            case kStrength:
            case kIntelligence:
            case kAgility:
            case kConstitution:
            case kWisdom:
            case kAtkBonus:
            case kDefBonus:
            case kMatkBonus:
            case kMdefBonus:
            case kSpdBonus: // a stat or bonus changed: the shown values follow it at once; the server's values are stale until it sends new ones
                pet.recalcStats = {pet.strength, pet.constitution, pet.intelligence, pet.wisdom, pet.agility};
                pet.recalcBonus = pet.combatBonus;
                pet.serverCombatKnown = false;
                break;
            case kLevel:
                pet.serverCombatKnown = false;
                break;
            default:
                break;
        }
    }

    bool PetTeam::wear(Pet& pet, const protocol::ItemInstance& item) const {
        const auto record = m_items.find(item.itemId);
        if (!record || record->equipSlot < 1 || record->equipSlot > kEquipmentSlots) {
            return false;
        }
        pet.worn[static_cast<std::size_t>(record->equipSlot)] = item;
        return true;
    }

    bool PetTeam::equip(int place, const protocol::ItemInstance& item) {
        return validPlace(place) && m_places[index(place)] && wear(*m_places[index(place)], item);
    }

    void PetTeam::clearEquipment(int place, int equipSlot) {
        if (validPlace(place) && m_places[index(place)] && equipSlot >= 1 && equipSlot <= kEquipmentSlots) {
            m_places[index(place)]->worn[static_cast<std::size_t>(equipSlot)] = {};
        }
    }

    protocol::ItemInstance PetTeam::swapEquipment(int place, const protocol::ItemInstance& item) {
        const auto record = m_items.find(item.itemId);
        if (!validPlace(place) || !m_places[index(place)] || !record || record->equipSlot < 1 || record->equipSlot > kEquipmentSlots) {
            return {};
        }
        protocol::ItemInstance& slot = m_places[index(place)]->worn[static_cast<std::size_t>(record->equipSlot)];
        const protocol::ItemInstance before = slot;
        slot = item;
        return before;
    }

    const std::optional<Pet>& PetTeam::at(int place) const { return m_places[index(place)]; }

    std::optional<Pet>& PetTeam::at(int place) { return m_places[index(place)]; }

    int PetTeam::placeByOrder(int order) const {
        for (int place = 1; place <= kPlaces; ++place) {
            const auto& pet = m_places[index(place)];
            if (pet && pet->order == order) {
                return place;
            }
        }
        return 0;
    }

    int PetTeam::placeOf(int npcId) const {
        for (int place = 1; place <= kPlaces; ++place) {
            const auto& pet = m_places[index(place)];
            if (pet && pet->info.npcId == npcId) {
                return place;
            }
        }
        return 0;
    }

    int PetTeam::count() const {
        return static_cast<int>(std::count_if(m_places.begin() + 1, m_places.end(), [](const auto& pet) { return pet.has_value(); }));
    }

}
