#ifndef NORTH_ISLAND_CLIENT_MODELS_CHARACTER_CHARACTER_LOOK_H
#define NORTH_ISLAND_CLIENT_MODELS_CHARACTER_CHARACTER_LOOK_H

#include "formats/ColorPattern.h"
#include "formats/ItemTable.h"
#include "protocol/CharacterSummary.h"
#include "protocol/PlayerAppearance.h"

#include <array>
#include <cstdint>

namespace client {

    /// @brief How a player character looks: body type 1..4, head, body sprite, colours and the sprites worn per equipment slot 1..6.
    /// @details Also holds what the layer order reads (DrawHumanLayers).
    struct CharacterLook {
        int bodyType = 1;
        int head = 0;
        int bodySprite = 0;
        formats::ColorPattern basePattern;
        formats::ColorPattern pattern;
        // 7 entries: slots 1..6, index 0 unused
        std::array<int, 7> equipmentLooks{};
        int weaponKind = 0;
        int drawLook = 0;
        int specialType = 0;
        int job = 0;

        /// @brief Builds the look from the character list: base colours from the two pattern codes, equipment from Item.dat.
        /// @param character The character list entry.
        /// @param items Item.dat table.
        /// @return The look.
        static CharacterLook fromSummary(const protocol::CharacterSummary& character, const formats::ItemTable& items);

        /// @brief Builds the look from a player record of the world (server 3 / 4).
        /// @param appearance The player's appearance record.
        /// @param items Item.dat table.
        /// @param job The player's class (0..6).
        /// @return The look.
        static CharacterLook fromAppearance(const protocol::PlayerAppearance& appearance, const formats::ItemTable& items, int job = 0);

        /// @brief Builds a face as friend and guild records give it: body type, head and the two colour codes, nothing worn.
        /// @details The codes are parts 1 and 5, as SeparatePatt puts them.
        /// @param bodyType Body type.
        /// @param head Head number.
        /// @param code1 Colour code of part 1.
        /// @param code5 Colour code of part 5.
        /// @param items Item.dat table.
        /// @return The look.
        static CharacterLook fromColorCodes(
            int bodyType,
            int head,
            std::int32_t code1,
            std::int32_t code5,
            const formats::ItemTable& items
        );

        /// @brief Builds a body type and head in its own colours wearing items (Item.dat gives their slots, sprites and colours).
        /// @param bodyType Body type.
        /// @param head Head number.
        /// @param colors The character's colours.
        /// @param equipment Six item ids, 0 = none.
        /// @param items Item.dat table.
        /// @return The look.
        static CharacterLook dressed(
            int bodyType,
            int head,
            const formats::ColorPattern& colors,
            const std::array<std::uint16_t, 6>& equipment,
            const formats::ItemTable& items
        );
    };

}

#endif
