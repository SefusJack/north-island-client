#include "client/scenes/world/WorldScene.h"

#include "client/rules/FurnitureKinds.h"
#include "client/rules/CharacterActions.h"
#include "protocol/HouseRequests.h"

namespace client {

    namespace {

        constexpr int kRoundTubs[] = {39005, 39022}; // Wood Bathtub, Elm Bathtub
        constexpr int kLongTub = 39024; // Ancient Bathtub
        constexpr int kSprayTub = 39025; // Shower
        constexpr int kMessageMs = 2000; // message display time in ms

        /// @brief Where a bather sits in a tub: the pixel offset from the tub's middle and the pose slot to play.
        /// @details Original: DZ_Human.pas SitOnUtensil places.
        struct BathPlace {
            core::Point offset;
            int slot = 0;
        };

        /// @brief Finds the bather's place in a tub.
        /// @param itemId Item id of the tub.
        /// @param even Whether the tub's picture number is even (its facing).
        /// @param seat Seat number reported by the server (1 first seat, 2 to 4 the second kind).
        /// @return The offset and pose slot, or nothing when the item is not a known tub.
        /// @details Original: DZ_Human.pas SitOnUtensil.
        std::optional<BathPlace> bathPlace(int itemId, bool even, int seat) {
            BathPlace place;
            const bool second = seat >= 2 && seat <= 4; // seat numbers 2 to 4 are the second position
            if (itemId == kRoundTubs[0] || itemId == kRoundTubs[1]) {
                // round tub: pixel offsets from the middle, pose slots 71/69 by facing; the seat adjustments are pixel offsets and a pose slot step of 4
                place = even ? BathPlace{{-10, -8}, 71} : BathPlace{{10, -8}, 69};
                if (seat == 1) {
                    place.offset.x += even ? -12 : 12;
                    place.offset.y += -8;
                } else if (second) {
                    place.offset.x += even ? 24 : -24;
                    place.offset.y += 6;
                    place.slot += even ? -4 : 4;
                }
            } else if (itemId == kLongTub) {
                // long tub: pixel offsets from the middle, pose slots 69/71 by facing; the seat adjustments are pixel offsets and a pose slot step of 4
                place = even ? BathPlace{{-17, 20}, 69} : BathPlace{{17, 20}, 71};
                if (seat == 1) {
                    place.offset.x += even ? 16 : -16;
                    place.offset.y += -6;
                } else if (second) {
                    place.offset.x += even ? -20 : 20;
                    place.offset.y += 6;
                    place.slot += even ? 4 : -4;
                }
            } else if (itemId == kSprayTub) {
                // spray tub: pixel offset from the middle, pose slots 15/9 by facing; the seat adjustments are pixel offsets and a pose slot step of 4
                place = even ? BathPlace{{0, 13}, 15} : BathPlace{{0, 13}, 9};
                if (seat == 1) {
                    place.offset.x += even ? -14 : 14;
                    place.offset.y += 8;
                } else if (second) {
                    place.offset.x += even ? 12 : -12;
                    place.slot += even ? -4 : 4;
                }
            } else {
                return std::nullopt;
            }
            return place;
        }

    }

    bool WorldScene::isBathtub(int itemId) const {
        constexpr int kBathtubSubType = 112; // item sub type 112: bathtub furniture
        const auto record = m_services.items.find(itemId);
        return record && record->subType == kBathtubSubType && record->rank != 0;
    }

    std::optional<core::Point> WorldScene::bathSortCell(const MapCharacter& player) const {
        if (player.bathFurn == 0) {
            return std::nullopt;
        }
        const FurnitureRecord& piece = m_house.furniture().piece(player.bathFurn);
        const auto item = m_services.items.find(piece.itemId);
        if (!item) {
            return std::nullopt;
        }
        if (player.id == m_player.id && piece.itemId == kSprayTub) {
            return core::Point{piece.cellX, piece.cellY};
        }
        const int sizeX = piece.rotation == 1 ? item->furnSizeY : item->furnSizeX;
        const int sizeY = piece.rotation == 1 ? item->furnSizeX : item->furnSizeY;
        return core::Point{piece.cellX + sizeY - 1, piece.cellY + sizeX - 1};
    }

    int WorldScene::sprayBathPiece() const {
        const std::int32_t ownerId = m_house.furniture().ownerId();
        std::optional<std::reference_wrapper<const MapCharacter>> owner;
        if (ownerId == m_player.id) {
            owner = std::cref(m_player);
        } else if (const auto found = m_others.find(ownerId); found != m_others.end()) {
            owner = std::cref(found->second);
        }
        if (!owner || !owner->get().bathing || owner->get().bathFurn == 0 || m_house.furniture().piece(owner->get().bathFurn).itemId != kSprayTub) {
            return 0;
        }
        return owner->get().bathFurn;
    }

    void WorldScene::drawSprayExtras(gfx::Canvas& canvas, const MapCharacter& player, bool afterFigure) {
        constexpr int kWaterFrames = 3; // frames stacked vertically in the spray_N water picture
        constexpr core::Point kWaterAt{135, 55}; // water picture position inside the shower picture, px
        constexpr core::Rect kMiddle{120, 0, 60, 300}; // middle strip of the shower overlay redrawn in front of the bather: x, y, width, height px
        const FurnitureRecord& piece = m_house.furniture().piece(player.bathFurn);
        const int slot = m_house.utensils().slotOfPiece(player.bathFurn);
        if (piece.itemId != kSprayTub || slot == 0) {
            return;
        }
        const core::Rect area = m_house.utensils().pictureArea(slot, m_camera.origin());
        if (afterFigure) {
            const gfx::PictureId overlay = m_house.utensils().utensil(slot).overlayPicture;
            if (overlay.isValid() && sprayBathPiece() == player.bathFurn) {
                canvas.drawPicture(overlay, {area.x + kMiddle.x, area.y}, kMiddle);
            }
            return;
        }
        const auto item = m_services.items.find(piece.itemId);
        if (!item) {
            return;
        }
        const int number = piece.rotation == 0 ? item->furnPicture0 : item->furnPicture1;
        const auto water = m_services.pictures.find("spray_" + std::to_string((number + 1) % 2 + 1));
        if (!water) {
            return;
        }
        if (canvas.isAnimationFrame()) {
            m_sprayFrame = m_sprayFrame % kWaterFrames + 1;
        }
        const gfx::Picture& data = m_services.pictures.picture(*water);
        const int height = data.height() / kWaterFrames;
        canvas.drawPicture(*water, {area.x + kWaterAt.x, area.y + kWaterAt.y}, core::Rect{0, (m_sprayFrame - 1) * height, data.width(), height});
    }

    void WorldScene::useBathtub(int furnIndex) {
        if (m_player.stall || m_itemStall.isOpen() || m_petStall.isOpen()) {
            m_messages.show("Unable to bathe while setting a stall", kMessageMs);
        } else if (m_player.vehicle) {
            m_messages.show("Unable to bathe while using a vehicle", kMessageMs);
        } else if (m_makeJobs.playerSlot() != 0 && m_makeJobs.petSlot() == 0) {
            m_messages.show("Unable to bathe while making a item", kMessageMs);
        } else if (m_player.shape) {
            m_messages.show("Can't use when you are shifting body", kMessageMs);
        } else {
            m_services.connection.send(protocol::batheRequest(m_house.furniture().ownerId(), furnIndex));
        }
    }

    void WorldScene::onBathing(std::int32_t playerId, int furnIndex, int seat, std::int32_t sitterId) {
        const auto character = characterOf(playerId);
        const int slot = m_house.utensils().slotOfPiece(furnIndex);
        if (!character || slot == 0) {
            return;
        }
        const FurnitureRecord& piece = m_house.furniture().piece(furnIndex);
        const auto item = m_services.items.find(piece.itemId);
        if (!item) {
            return;
        }
        const bool even = (piece.rotation == 0 ? item->furnPicture0 : item->furnPicture1) % 2 == 0;
        const auto place = bathPlace(piece.itemId, even, seat);
        if (!place) {
            return;
        }
        const core::Point world = m_house.utensils().utensil(slot).world;
        MapCharacter& bather = character->get();
        bather.bathing = true;
        bather.bathFurn = furnIndex;
        if (isOwn(playerId) && m_map) {
            m_camera.centerOn(world, m_map->width(), m_map->height());
        }
        placeCharacter(bather, {world.x + place->offset.x, world.y + place->offset.y}, false);
        bather.action = 0;
        startActionSlot(bather, place->slot);
        if (!isOwn(playerId)) {
            const std::optional<std::int32_t> leader = m_teams.leaderOf(m_player.id);
            if (leader && *leader != sitterId && *leader == playerId && !m_player.riding) {
                m_services.connection.send(protocol::batheRequest(m_house.furniture().ownerId(), furnIndex));
            }
        }
    }

    void WorldScene::onBathEnd(std::int32_t playerId) {
        if (const auto character = characterOf(playerId); character && character->get().bathing) {
            character->get().bathing = false;
            character->get().bathFurn = 0;
            stopAction(character->get());
        }
    }

    void WorldScene::leaveBath() {
        if (m_player.bathing) {
            m_services.connection.send(protocol::stopBatheRequest(m_house.furniture().ownerId(), m_player.bathFurn));
            m_player.bathing = false;
            m_player.bathFurn = 0;
        }
    }

}
