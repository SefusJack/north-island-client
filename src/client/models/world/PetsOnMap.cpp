#include "client/models/world/PetsOnMap.h"

#include "client/rules/Walking.h"

#include <algorithm>

namespace client {

    void PetsOnMap::show(
        std::int32_t ownerId,
        const formats::NpcInfo& info,
        const std::string& name,
        int tradable,
        std::uint64_t nowMs
    ) {
        recall(ownerId, nowMs);
        m_away.erase(ownerId);
        MapPet pet;
        pet.ownerId = ownerId;
        pet.info = info;
        pet.name = name;
        pet.tradable = tradable;
        pet.visible = shown(ownerId);
        pet.body.position = m_trails.position(ownerId);
        pet.body.direction = 0;
        startWalking(pet.body, {m_trails.point(ownerId, FollowTrail::kPetPoint)}, nowMs);
        m_pets.push_back(std::move(pet));
    }

    void PetsOnMap::showMaker(
        std::int32_t ownerId,
        const formats::NpcInfo& info,
        const std::string& name,
        int tradable,
        std::uint64_t nowMs
    ) {
        hideMaker(ownerId);
        MapPet pet;
        pet.ownerId = ownerId;
        pet.info = info;
        pet.name = name;
        pet.tradable = tradable;
        pet.following = false;
        pet.maker = true;
        pet.visible = false;
        pet.body.position = m_trails.position(ownerId);
        startWalking(pet.body, {m_trails.point(ownerId, kMakerPoint)}, nowMs);
        m_pets.push_back(std::move(pet));
    }

    void PetsOnMap::hideMaker(std::int32_t ownerId) {
        m_pets.erase(std::remove_if(m_pets.begin(), m_pets.end(), [ownerId](const MapPet& pet) { return pet.ownerId == ownerId && pet.maker; }), m_pets.end());
    }

    void PetsOnMap::driveMaker(std::int32_t ownerId, bool visible, core::Point workAt) {
        for (MapPet& pet : m_pets) {
            if (pet.ownerId == ownerId && pet.maker) {
                pet.visible = visible;
                pet.workAt = workAt;
            }
        }
    }

    void PetsOnMap::setWeapon(std::int32_t ownerId, const PetWeapon& weapon) {
        for (MapPet& pet : m_pets) {
            if (pet.ownerId == ownerId && pet.following) {
                pet.weapon = weapon;
            }
        }
    }

    void PetsOnMap::recall(std::int32_t ownerId, std::uint64_t nowMs) {
        for (MapPet& pet : m_pets) {
            if (pet.ownerId == ownerId && pet.following) {
                pet.following = false;
                startWalking(pet.body, {m_trails.knows(ownerId) ? m_trails.position(ownerId) : pet.body.position}, nowMs);
            }
        }
    }

    void PetsOnMap::remove(std::int32_t ownerId) {
        m_pets.erase(std::remove_if(m_pets.begin(), m_pets.end(), [ownerId](const MapPet& pet) { return pet.ownerId == ownerId; }), m_pets.end());
        m_inTeam.erase(ownerId);
        m_hidden.erase(ownerId);
        m_away.erase(ownerId);
    }

    void PetsOnMap::rename(std::int32_t ownerId, int npcId, const std::string& name) {
        for (MapPet& pet : m_pets) {
            if (pet.ownerId == ownerId && pet.info.npcId == npcId) {
                pet.name = name;
            }
        }
    }

    void PetsOnMap::changeKind(std::int32_t ownerId, int npcId, const formats::NpcInfo& info) {
        for (MapPet& pet : m_pets) {
            if (pet.ownerId == ownerId && pet.info.npcId == npcId && !pet.walker) {
                pet.info = info;
                pet.name = info.name;
                return;
            }
        }
    }

    void PetsOnMap::ownerPlaced(std::int32_t ownerId) {
        m_pets.erase(
            std::remove_if(m_pets.begin(), m_pets.end(), [ownerId](const MapPet& pet) { return pet.ownerId == ownerId && !pet.following && !pet.maker; }),
            m_pets.end()
        );
        for (MapPet& pet : m_pets) {
            if (pet.ownerId == ownerId && !pet.maker) {
                pet.body.position = m_trails.position(ownerId);
                pet.body.walking = false;
                pet.body.path.clear();
            }
        }
    }

    void PetsOnMap::setInTeam(std::int32_t ownerId, bool inTeam) {
        const bool changed = inTeam ? m_inTeam.insert(ownerId).second : m_inTeam.erase(ownerId) != 0;
        if (!changed) {
            return;
        }
        for (MapPet& pet : m_pets) {
            if (pet.ownerId == ownerId && !pet.maker) {
                pet.visible = shown(ownerId);
            }
        }
        if (!inTeam) {
            ownerPlaced(ownerId);
        }
    }

    bool PetsOnMap::shown(std::int32_t ownerId) const {
        return !m_inHall && m_inTeam.count(ownerId) == 0 && m_hidden.count(ownerId) == 0 && m_away.count(ownerId) == 0;
    }

    void PetsOnMap::setHidden(std::int32_t ownerId, bool hidden) {
        if (hidden) {
            m_hidden.insert(ownerId);
        } else {
            m_hidden.erase(ownerId);
        }
        for (MapPet& pet : m_pets) {
            if (pet.ownerId == ownerId && !pet.maker) {
                pet.visible = shown(ownerId);
            }
        }
    }

    void PetsOnMap::setHallMap(bool inHall) {
        m_inHall = inHall;
        for (MapPet& pet : m_pets) {
            if (!pet.maker) {
                pet.visible = shown(pet.ownerId);
            }
        }
    }

    void PetsOnMap::petWalk(
        std::int32_t ownerId,
        const formats::NpcInfo& info,
        int mode,
        int mapWidth,
        std::uint64_t nowMs
    ) {
        constexpr int kFromRight = 400; // px right of the owner where the walker starts
        constexpr double kPace = 0.36; // walking speed, px per ms
        m_pets.erase(std::remove_if(m_pets.begin(), m_pets.end(), [ownerId](const MapPet& pet) { return pet.ownerId == ownerId && pet.walker; }), m_pets.end());
        const core::Point owner = m_trails.position(ownerId);
        MapPet walker;
        walker.ownerId = ownerId;
        walker.info = info;
        walker.walker = true;
        walker.following = false;
        walker.walkMode = mode;
        walker.body.pixelsPerMs = kPace;
        m_mapWidth = mapWidth;
        if (mode == 1) {
            m_away.insert(ownerId);
            for (MapPet& pet : m_pets) {
                if (pet.ownerId == ownerId && !pet.maker) {
                    pet.visible = false;
                }
            }
            walker.body.position = owner;
            startWalking(walker.body, {{mapWidth, owner.y}}, nowMs);
        } else if (mode >= 2 && mode <= 4) { // walk modes 2..4: the walker comes from the right (3 back, 2 / 4 back and off again)
            walker.body.position = {owner.x + kFromRight, owner.y};
            startWalking(walker.body, {owner}, nowMs);
        } else {
            return;
        }
        m_pets.push_back(std::move(walker));
    }

    void PetsOnMap::npcWalkBy(
        std::int32_t ownerId,
        const formats::NpcInfo& info,
        int mode,
        int mapWidth,
        std::uint64_t nowMs
    ) {
        constexpr int kFromRight = 400; // px right of the owner where the walker starts
        constexpr double kPace = 0.36; // walking speed, px per ms
        m_pets.erase(std::remove_if(m_pets.begin(), m_pets.end(), [ownerId](const MapPet& pet) { return pet.ownerId == ownerId && pet.walkBy; }), m_pets.end());
        const core::Point owner = m_trails.position(ownerId);
        MapPet walker;
        walker.ownerId = ownerId;
        walker.info = info;
        walker.walker = true;
        walker.walkBy = true;
        walker.following = false;
        walker.walkMode = mode;
        walker.walkByEnd = mapWidth;
        walker.body.pixelsPerMs = kPace;
        if (mode == 1) {
            walker.body.position = owner;
            startWalking(walker.body, {{mapWidth, owner.y}}, nowMs);
        } else if (mode == 2) { // walk-by mode 2: comes from the right, then goes off
            walker.body.position = {owner.x + kFromRight, owner.y};
            startWalking(walker.body, {owner}, nowMs);
        } else {
            return;
        }
        m_pets.push_back(std::move(walker));
    }

    void PetsOnMap::update(std::uint64_t nowMs, WalkSteps steps) {
        for (MapPet& pet : m_pets) {
            if (pet.following && m_trails.knows(pet.ownerId)) {
                const core::Point goal = m_trails.point(pet.ownerId, FollowTrail::kPetPoint);
                const bool heading = pet.body.walking && !pet.body.path.empty() && pet.body.path.back() == goal;
                if (!heading && pet.body.position != goal) {
                    const std::uint64_t stepMs = pet.body.walking ? pet.body.stepMs : nowMs;
                    startWalking(pet.body, {goal}, nowMs);
                    pet.body.stepMs = stepMs;
                }
            }
            walkStep(pet.body, nowMs, steps);
            if (pet.maker && !pet.body.walking) {
                pet.body.direction = directionTowards(pet.body.position, pet.workAt, pet.body.direction);
            }
            if (pet.walkBy && !pet.body.walking && pet.walkMode != 0) {
                constexpr int kFromRight = 400; // px right of the owner where the walker starts
                const core::Point owner = m_trails.position(pet.ownerId);
                if (pet.walkMode == 1) {
                    pet.body.position = {owner.x + kFromRight, owner.y};
                    startWalking(pet.body, {owner}, nowMs);
                } else {
                    pet.body.position = owner;
                    startWalking(pet.body, {{pet.walkByEnd, owner.y}}, nowMs);
                }
                pet.walkMode = 0;
            }
            if (pet.walker && !pet.walkBy && !pet.body.walking && (pet.walkMode == 2 || pet.walkMode == 4)) { // modes 2 and 4: came back, goes off again
                const core::Point owner = m_trails.position(pet.ownerId);
                pet.walkMode = 0;
                pet.body.position = owner;
                startWalking(pet.body, {{m_mapWidth, owner.y}}, nowMs);
            }
        }
        m_pets.erase(
            std::remove_if(m_pets.begin(), m_pets.end(), [](const MapPet& pet) { return !pet.following && !pet.maker && !pet.body.walking; }),
            m_pets.end()
        );
    }

}
