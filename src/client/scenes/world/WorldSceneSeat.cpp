#include "client/scenes/world/WorldScene.h"

#include "client/rules/FurnitureKinds.h"
#include "client/rules/PianoSeat.h"
#include "client/rules/UtensilSeat.h"
#include "client/rules/CharacterActions.h"
#include "protocol/EmotionRequests.h"
#include "protocol/HouseRequests.h"

#include <array>
#include <cstdlib>
#include <string>

namespace client {

    namespace {

        constexpr int kReach = 170; // max distance to the furniture in x and y, px
        constexpr int kNoticeMs = 1500; // message display time, ms
        constexpr int kSittingGroup = 46; // character action group 46: sitting

    }

    bool WorldScene::useUtensil(MapCharacter& character, int furnIndex, int seat) {
        if (m_player.shape) {
            return false;
        }
        const bool own = isOwn(character.id);
        if (own) {
            m_services.connection.send(protocol::stopBatheRequest(m_house.furniture().ownerId(), m_player.bathFurn));
            m_player.bathing = false;
            m_player.bathFurn = 0;
        }
        if (character.utensilHidden) {
            const int hiddenSlot = m_house.utensils().slotOfPiece(character.usedUtensil);
            if (hiddenSlot != 0) {
                m_house.utensils().utensil(hiddenSlot).shown = true;
            }
            character.utensilHidden = false;
        }
        character.usedUtensil = furnIndex;
        character.usedSeat = seat;
        if (furnIndex == 0) {
            return false;
        }
        if (m_teams.inTeam(character.id)) {
            character.usedUtensil = 0;
            character.usedSeat = 0;
            return false;
        }
        const int slot = m_house.utensils().slotOfPiece(furnIndex);
        const FurnitureRecord& piece = m_house.furniture().piece(furnIndex);
        const auto item = m_services.items.find(piece.itemId);
        if (slot == 0 || !item) {
            return false;
        }
        const core::Point world = m_house.utensils().utensil(slot).world;
        if (own && m_map && !isTrainingPiece(piece.itemId)) {
            m_camera.centerOn(world, m_map->width(), m_map->height());
        }
        const int pictureNumber = piece.rotation == 0 ? item->furnPicture0 : item->furnPicture1;
        const bool even = pictureNumber % 2 == 0;
        character.seatShift = {};
        character.seatOverlay = {};
        if (isTrainingPiece(piece.itemId)) {
            return useTrainingPiece(character, furnIndex, piece, even, world);
        }
        if (FurnitureKinds::isPiano(m_services.items, piece.itemId)) {
            const PianoSeat::Place place = PianoSeat::place(world, even);
            placeCharacter(character, place.position, false);
            character.action = 0;
            character.direction = place.direction;
            playSitting(character, kSittingGroup);
            return true;
        }
        const auto place = UtensilSeat::place(m_services.items, piece.itemId, seat, even, world);
        if (!place) {
            return false;
        }
        constexpr int kToiletRise = 13; // seat picture shift up on a toilet, px
        constexpr int kToiletAside = 4; // seat picture shift sideways on a toilet, px (left on even pictures)
        if (UtensilSeat::isToilet(m_services.items, piece.itemId)) {
            character.seatShift = {even ? -kToiletAside : kToiletAside, -kToiletRise};
        }
        if (UtensilSeat::isOctopus(m_services.items, piece.itemId)) {
            character.seatOverlay = m_services.pictures.find(std::to_string(pictureNumber)).value_or(gfx::PictureId{});
        }
        placeCharacter(character, place->position, false);
        character.action = 0;
        character.direction = place->direction;
        playSitting(character, place->group);
        character.holdFirstFrame = place->holdFirstFrame && !character.riding;
        return true;
    }

    void WorldScene::playSitting(MapCharacter& character, int group) {
        if (character.riding) {
            return;
        }
        startAction(character, group);
        if (isOwn(character.id)) {
            m_services.connection.send(protocol::actionRequest(character.actionSlot()));
        }
    }

    void WorldScene::clickSeat(int piece) {
        constexpr int kSecondSeat = 2; // seat number 2 of a two-seat piece
        const int slot = m_house.utensils().slotOfPiece(piece);
        if (slot == 0) {
            return;
        }
        const HouseUtensil& utensil = m_house.utensils().utensil(slot);
        const int itemId = utensil.itemId;
        if (!UtensilSeat::isSeat(m_services.items, itemId)) {
            return;
        }
        if (m_player.id == m_house.furniture().ownerId() && m_player.bathing && m_player.bathFurn == piece) {
            return;
        }
        if (std::abs(utensil.world.x - m_player.position.x) > kReach || std::abs(utensil.world.y - m_player.position.y) > kReach) {
            return;
        }
        const int seats = UtensilSeat::seatCount(m_services.items, itemId);
        if (UtensilSeat::isOctopus(m_services.items, itemId)) {
            if (m_player.shape) {
                m_messages.show("Shapeshifting, use failed", kNoticeMs);
                return;
            }
            if (m_player.riding) {
                m_messages.show("Riding, use failed", kNoticeMs);
                return;
            }
            if (m_teams.inTeam(m_player.id)) {
                m_messages.show("In a team, use failed", kNoticeMs);
                return;
            }
            if (pieceInUse(piece)) {
                return;
            }
            if (useUtensil(m_player, piece, 1)) {
                m_house.utensils().utensil(slot).shown = false;
                m_player.utensilHidden = true;
                m_services.connection.send(protocol::sitUtensilRequest(m_player.usedUtensil, m_player.usedSeat));
            }
            return;
        }
        if ((seats == 3 || UtensilSeat::isToilet(m_services.items, itemId)) && m_player.riding) { // 3 seats: the Great Sofa-like piece
            m_messages.show("Can't use when you are riding pets", kNoticeMs);
            return;
        }
        int seat = 1;
        if (seats == 2) { // a piece with 2 seats
            std::array<bool, 3> taken{}; // indexed by seat number 1 and 2 (index 0 unused)
            const auto note = [&](const MapCharacter& other) {
                if (other.usedUtensil == piece && other.usedSeat >= 1 && other.usedSeat <= kSecondSeat) {
                    taken[static_cast<std::size_t>(other.usedSeat)] = true;
                }
            };
            note(m_player);
            for (const auto& [id, other] : m_others) {
                note(other);
            }
            if (taken[1] && taken[kSecondSeat]) {
                return;
            }
            seat = !taken[1] ? 1 : kSecondSeat;
            if (m_player.usedSeat == 1 && !taken[kSecondSeat]) {
                seat = kSecondSeat;
            }
            if (m_player.usedSeat == kSecondSeat && !taken[1]) {
                seat = 1;
            }
        } else if (pieceInUse(piece)) {
            return;
        }
        if (useUtensil(m_player, piece, seat)) {
            m_services.connection.send(protocol::sitUtensilRequest(m_player.usedUtensil, m_player.usedSeat));
        }
    }

    void WorldScene::leaveSeat() {
        if (m_pianoSeated) {
            m_pianoSeated = false;
            m_piano.close();
        }
        if (m_player.usedUtensil != 0) {
            m_trainingPet.reset();
            if (m_player.utensilHidden) {
                m_player.utensilHidden = false;
                if (const int slot = m_house.utensils().slotOfPiece(m_player.usedUtensil); slot != 0) {
                    m_house.utensils().utensil(slot).shown = true;
                }
            }
            m_player.usedUtensil = 0;
            m_player.usedSeat = 0;
            m_services.connection.send(protocol::stopUsingUtensilRequest());
        }
    }

    void WorldScene::onUtensilUsed(std::int32_t playerId, int furnIndex, int seat) {
        const auto character = characterOf(playerId);
        const int slot = m_house.utensils().slotOfPiece(furnIndex);
        if (!character || !m_house.inFloorRoom() || slot == 0) {
            return;
        }
        const int itemId = m_house.utensils().utensil(slot).itemId;
        const bool piano = FurnitureKinds::isPiano(m_services.items, itemId);
        if (!useUtensil(character->get(), furnIndex, piano ? 1 : seat)) {
            return;
        }
        if (piano) {
            if (isOwn(playerId)) {
                m_pianoSeated = true;
            }
        } else if (!isOwn(playerId) && UtensilSeat::isOctopus(m_services.items, itemId)) {
            m_house.utensils().utensil(slot).shown = false;
            character->get().utensilHidden = true;
        }
    }

    void WorldScene::onUtensilLeft(std::int32_t playerId) {
        const auto character = characterOf(playerId);
        if (!character) {
            return;
        }
        MapCharacter& left = character->get();
        if (!isOwn(playerId) && left.utensilHidden) {
            if (const int slot = m_house.utensils().slotOfPiece(left.usedUtensil); slot != 0) {
                m_house.utensils().utensil(slot).shown = true;
            }
            left.utensilHidden = false;
        }
        left.usedUtensil = 0;
        left.usedSeat = 0;
    }

}
