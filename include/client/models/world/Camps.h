#ifndef NORTH_ISLAND_CLIENT_MODELS_WORLD_CAMPS_H
#define NORTH_ISLAND_CLIENT_MODELS_WORLD_CAMPS_H

#include "client/models/world/Camp.h"
#include "core/Bytes.h"
#include "formats/ItemTable.h"

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>

namespace client {

    /// @brief The tents of the map and the player's own, driven by the server's action 65.
    /// @details Original: Re_Camp.pas TRe_CampManage, 300 places. Action 65 puts tents up (65/1, list 65/3), takes them down (65/4)
    /// and changes their style (65/14).
    class Camps {
        public:
            /// @brief Callbacks into the owning scene.
            struct Hooks {
                std::function<bool(std::int32_t playerId)> isOwn;
                std::function<int()> mapId;
                std::function<void(const std::string&, int)> message;
                std::function<int(int tentItemId)> frameHeight;
                std::function<void()> ownTentPitched;
            };

            /// @brief Creates the camp list.
            /// @param items Item table used to look up tent names.
            /// @param hooks Scene callbacks.
            Camps(
                const formats::ItemTable& items,
                Hooks hooks
            );

            /// @brief Handles a tent being put up.
            /// @details Original: server 65/1 (RecvCampAdded): i32 owner, u16 tent item, i32 x, i32 y, u8 kind 1, u8 kind 2.
            /// @param payload Packet payload.
            void receiveAdded(const core::Bytes& payload);

            /// @brief Handles the map's list of tents.
            /// @details Original: server 65/3 (RecvCampList): records of 16 bytes.
            /// @param payload Packet payload.
            void receiveList(const core::Bytes& payload);

            /// @brief Handles a tent being taken down.
            /// @details Original: server 65/4 (RecvCampTakenBack): i32 owner.
            /// @param payload Packet payload.
            void receiveTakenBack(const core::Bytes& payload);

            /// @brief Handles a tent's style changing.
            /// @details Original: server 65/14 (RecvTentStyleChanged): i32 owner, u8 kind 1, u8 kind 2.
            /// @param payload Packet payload.
            void receiveStyle(const core::Bytes& payload);

            /// @brief Removes all tents (a map change).
            /// @details Original: FreeCampObjects.
            void clear();

            /// @brief The tents by place.
            /// @return Read-only map of place to tent.
            const std::map<int, Camp>& camps() const { return m_camps; }

            /// @brief The tents by place.
            /// @return Mutable map of place to tent.
            std::map<int, Camp>& camps() { return m_camps; }

            /// @brief The map where the own tent stands.
            /// @details Original: Player.FCampMapId.
            /// @return Map id, 0 for none.
            int ownMap() const { return m_ownMap; }

            /// @brief The own tent's item.
            /// @details Original: Player.FCampItemId.
            /// @return Item id, 0 for none.
            int ownItem() const { return m_ownItem; }

            /// @brief Finds the place of an owner's tent.
            /// @details Original: FindCampByOwner.
            /// @param ownerId Owner player id.
            /// @return The place, or nothing.
            std::optional<int> slotOfOwner(std::int32_t ownerId) const;

            /// @brief Gives the sprite id of a tent style.
            /// @details Original: GetCampKindId; style from the Tent Palette's set (1) or the Shape Clay's (2).
            /// @param index Style number.
            /// @param set 1 for the Tent Palette, 2 for the Shape Clay.
            /// @return The sprite id, 0 for none.
            static int kindId(int index, int set);

        private:
            /// @brief Adds a tent in the first free place.
            /// @details Original: AddCamp; the own tent also records its map and item and closes windows in the way.
            /// @param ownerId Owner player id.
            /// @param tentItemId Tent item id.
            /// @param position Map position.
            /// @param kind1 Tent Palette style.
            /// @param kind2 Shape Clay style.
            /// @return The place, 0 when full.
            int add(
                std::int32_t ownerId,
                int tentItemId,
                core::Point position,
                int kind1,
                int kind2
            );

            const formats::ItemTable& m_items;
            Hooks m_hooks;
            std::map<int, Camp> m_camps;
            int m_ownMap = 0;
            int m_ownItem = 0;
    };

}

#endif
