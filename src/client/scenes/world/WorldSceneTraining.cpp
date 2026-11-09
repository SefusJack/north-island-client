#include "client/scenes/world/WorldScene.h"

#include "client/rendering/characters/FigureShadow.h"
#include "client/models/house/TrainingPet.h"
#include "client/rules/CharacterActions.h"
#include "client/rules/Walking.h"
#include "protocol/EmotionRequests.h"
#include "protocol/HouseRequests.h"
#include "protocol/PetRequests.h"

#include "gfx/TextStyle.h"

#include <cstdlib>

namespace client {

    namespace {

        constexpr int kTreadmill = 39105; // item 39105: Treadmill
        constexpr int kBicepExerciser = 39100; // item 39100: Bicep Exerciser
        constexpr int kReach = 170; // max distance to the equipment, px
        constexpr int kNoticeMs = 1000; // notice message duration, ms
        constexpr int kTrainingPose = 46; // character pose while using the treadmill
        constexpr int kRidingPose = 18; // character pose while using the bicep exerciser
        constexpr int kFullTraining = 100; // training percentage at which no more training is possible
        constexpr core::Point kPetEven{-13, 8}; // pet offset from the treadmill on an even place, px (x, y)
        constexpr core::Point kPetOdd{16, 8}; // pet offset from the treadmill on an odd place, px (x, y)
        constexpr int kPetFacingEven = 7; // pet facing direction on an even place
        constexpr int kPetFacingOdd = 1; // pet facing direction on an odd place
        constexpr core::Point kBicepEven{-20, 30}; // character offset from the bicep exerciser on an even place, px (x, y)
        constexpr core::Point kBicepOdd{23, 27}; // character offset from the bicep exerciser on an odd place, px (x, y)
        constexpr int kBicepFacingEven = 7; // character facing direction on an even place
        constexpr int kBicepFacingOdd = 1; // character facing direction on an odd place
        constexpr int kNameGap = 10; // gap between the pet's head and its name, px
        constexpr int kStandingSlots = 8; // direction slot offset of the standing animation
        constexpr int kShortNpcHeight = 50; // pets shorter than this (px) use the larger name gap
        constexpr int kShortNpcNameGap = 25; // name gap for short pets, px
        constexpr int kCharWidth = 8; // width of one name character, px

    }

    bool WorldScene::isTrainingPiece(int itemId) const {
        return (itemId == kTreadmill || itemId == kBicepExerciser) && m_services.items.find(itemId).has_value();
    }

    bool WorldScene::useTrainingPiece(
        MapCharacter& character,
        int furnIndex,
        const FurnitureRecord& piece,
        bool even,
        core::Point world
    ) {
        const bool own = isOwn(character.id);
        const auto refuse = [&](const std::string& text, bool stopRequest) {
            if (own) {
                if (stopRequest) {
                    m_services.connection.send(protocol::stopUsingUtensilRequest());
                }
                m_messages.show(text, kNoticeMs);
            }
            character.usedUtensil = 0;
            character.usedSeat = 0;
            return false;
        };
        const bool owner = m_house.furniture().ownerId() == m_player.id;
        const auto riderOff = [&] {
            if (own && character.riding) {
                const int place = m_pets.ridingPlace();
                if (const auto& ridden = m_pets.at(place)) {
                    m_services.connection.send(protocol::dismountPetRequest(place, ridden->info.npcId));
                }
                character.riding.reset();
                m_pets.setRiding(0);
            }
        };
        m_trainingPet.reset();
        if (piece.itemId == kTreadmill) {
            if (own) {
                const int place = m_pets.activePlace();
                if (place == 0) {
                    return refuse("No pets can be trained", true);
                }
                const auto& pet = m_pets.at(place);
                if (!pet || pet->info.bodyType == 0 || pet->info.mercenary != 0) {
                    return refuse("No object to be trained, can't use sports equipment", true);
                }
                if (owner && pet->training >= kFullTraining) {
                    return refuse("Reached the train upper limit, need to be upgraded.", false);
                }
                TrainingPet runner;
                runner.npc = pet->info;
                runner.place = place;
                runner.furnIndex = furnIndex;
                const core::Point shift = even ? kPetEven : kPetOdd;
                runner.world = {world.x + shift.x, world.y + shift.y};
                runner.direction = even ? kPetFacingEven : kPetFacingOdd;
                m_trainingPet = runner;
            }
            riderOff();
            stopAction(character);
            character.direction = directionTowards(character.position, world, character.direction);
            character.action = 0;
            playSitting(character, kTrainingPose);
            return true;
        }
        if (owner && own && m_trainPct >= kFullTraining) {
            return refuse("Reached the train upper limit , need to be upgraded.", false);
        }
        const core::Point shift = even ? kBicepEven : kBicepOdd;
        placeCharacter(character, {world.x + shift.x, world.y + shift.y}, false);
        character.direction = even ? kBicepFacingEven : kBicepFacingOdd;
        riderOff();
        character.action = 0;
        playSitting(character, kRidingPose);
        return true;
    }

    void WorldScene::clickTraining(int piece) {
        const int slot = m_house.utensils().slotOfPiece(piece);
        if (slot == 0) {
            return;
        }
        const HouseUtensil& utensil = m_house.utensils().utensil(slot);
        if (!isTrainingPiece(utensil.itemId)) {
            return;
        }
        if (std::abs(utensil.world.x - m_player.position.x) > kReach || std::abs(utensil.world.y - m_player.position.y) > kReach) {
            return;
        }
        for (const auto& [id, other] : m_others) {
            if (other.usedUtensil == piece) {
                return;
            }
        }
        if (useUtensil(m_player, piece, 1)) {
            m_services.connection.send(protocol::sitUtensilRequest(m_player.usedUtensil, m_player.usedSeat));
        }
    }

    void WorldScene::onTrainingPercent(bool pet, int place, int percent) {
        if (!pet) {
            m_trainPct = percent;
        } else if (auto& found = m_pets.at(place)) {
            found->training = percent;
        }
    }

    void WorldScene::onTrainingReset(bool pet, int place) { onTrainingPercent(pet, place, 0); }

    void WorldScene::onTrainingBusy() { m_messages.show("Pet is being trained, the function can't be used", 2000); } // message duration in ms

    void WorldScene::endTraining(MapCharacter& character) {
        character.usedUtensil = 0;
        character.usedSeat = 0;
        stopAction(character);
        if (isOwn(character.id)) {
            m_trainingPet.reset();
            m_services.connection.send(protocol::actionRequest(character.actionSlot()));
        }
    }

    void WorldScene::updateTrainingDisplay(std::uint64_t nowMs) {
        HouseTraining& training = m_house.training();
        const std::int32_t ownerId = m_house.furniture().ownerId();
        int used = 0;
        if (m_player.id == ownerId) {
            used = m_player.usedUtensil;
        } else if (const auto owner = characterOf(ownerId)) {
            used = owner->get().usedUtensil;
        }
        training.setOwnerUsed(used);
        int bar = -1;
        bool bicep = false;
        if (used != 0) {
            if (const int slot = m_house.utensils().slotOfPiece(used); slot != 0) {
                bicep = m_house.utensils().utensil(slot).itemId == kBicepExerciser;
                if (m_player.id == ownerId) {
                    const auto& pet = m_pets.at(m_pets.activePlace());
                    bar = bicep ? m_trainPct : (pet ? pet->training : 0);
                }
            }
        }
        training.setBar(bar);
        training.step(nowMs, bicep);
    }

    void WorldScene::paintTrainingPet(gfx::Canvas& canvas, const Camera& camera, int furnIndex) {
        if (!m_trainingPet || m_trainingPet->furnIndex != furnIndex) {
            return;
        }
        TrainingPet& pet = *m_trainingPet;
        const int slot = m_services.characters.npcSlot(pet.npc.spriteId, pet.npc.directionKind, pet.direction);
        const int frame = pet.anim.next(m_nowMs, m_services.characters.npcFrameCount(pet.npc.spriteId, slot), false, HumanFrame::kMovingIntervalMs);
        const core::Point origin = camera.origin();
        const core::Point feet{pet.world.x - origin.x, pet.world.y - origin.y};
        drawFigureShadow(canvas, feet, pet.npc.shadow);
        m_services.characters.drawNpc(canvas, pet.npc, feet, slot, frame);
        if (const auto& named = m_pets.at(pet.place)) {
            const int standing = m_services.characters.npcSlot(pet.npc.spriteId, pet.npc.directionKind, kStandingSlots + pet.direction);
            const int height = m_services.characters.npcHeight(pet.npc.spriteId, pet.npc.spriteSize, standing, 0) * (pet.npc.heightMode == 1 ? 2 : 1);
            const int gap = height < kShortNpcHeight ? kShortNpcNameGap : kNameGap;
            const int width = static_cast<int>(named->name.size()) * kCharWidth;
            canvas.drawText(
                {feet.x - width / 2, feet.y - height - gap},
                named->name,
                {core::Color::fromRgb565(named->tradable == 1 ? 0x07FF : 0xFC00), gfx::TextEffect::Shadow, std::nullopt}
            ); // RGB565 colours: 0x07FF cyan (renamable pet), 0xFC00 orange
        }
    }

}
