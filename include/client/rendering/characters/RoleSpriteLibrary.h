#ifndef NORTH_ISLAND_CLIENT_RENDERING_CHARACTERS_ROLE_SPRITE_LIBRARY_H
#define NORTH_ISLAND_CLIENT_RENDERING_CHARACTERS_ROLE_SPRITE_LIBRARY_H

#include "client/services/GameDirectory.h"
#include "formats/PngCodec.h"
#include "formats/RoleSprite.h"
#include "formats/RoleSpriteFile.h"

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <utility>

namespace client {

    /// @brief The sprite banks of the jma folder ("002" bodies, "002h" heads, "002e" clothes, ...), opened when first needed.
    /// @details Decoded sprites are kept.
    class RoleSpriteLibrary {
        public:
            /// @brief Creates the library.
            /// @param game The game directory holding the jma folder.
            explicit RoleSpriteLibrary(
                const GameDirectory& game
            )
                : m_game(game) {}

            RoleSpriteLibrary(const RoleSpriteLibrary&) = delete;
            RoleSpriteLibrary& operator=(const RoleSpriteLibrary&) = delete;

            /// @brief Tells whether a bank holds a sprite.
            /// @param bank Bank name.
            /// @param spriteId Sprite id.
            /// @return True when it does.
            bool contains(const std::string& bank, int spriteId);

            /// @brief Finds a decoded sprite, reading it on first use.
            /// @param bank Bank name.
            /// @param spriteId Sprite id.
            /// @return The sprite, nothing when missing.
            std::optional<std::reference_wrapper<const formats::RoleSprite>> find(const std::string& bank, int spriteId);

            /// @brief Registers a mod pack's HD frame (sprites\@2x\<bank>\<sprite>\<frame>.png).
            /// @details The frame's palette indices at twice its size each way, as an 8-bit paletted PNG; the last one added for a frame wins.
            /// @param bank Bank name.
            /// @param spriteId Sprite id.
            /// @param frame Frame index.
            /// @param file Path of the PNG.
            void addHdFrame(const std::string& bank, int spriteId, int frame, const std::filesystem::path& file);

            /// @brief Gives a frame's HD palette indices when a pack has them and they fit the frame.
            /// @param bank Bank name.
            /// @param spriteId Sprite id.
            /// @param frame Frame index.
            /// @param original The original frame, whose size the HD image must double.
            /// @return The HD image, else nothing (and a warning logged once).
            std::optional<std::reference_wrapper<const formats::IndexedImage>> hdFrame(
                const std::string& bank,
                int spriteId,
                std::size_t frame,
                const formats::RoleSpriteFrame& original
            );

        private:
            using FrameId = std::tuple<std::string, int, std::size_t>;

            /// @brief Opens a sprite bank on first use.
            /// @param bank Bank name.
            /// @return The bank file, nothing when it cannot be opened.
            std::optional<std::reference_wrapper<formats::RoleSpriteFile>> openBank(const std::string& bank);

            const GameDirectory& m_game;
            std::map<std::string, std::optional<formats::RoleSpriteFile>> m_banks;
            std::map<std::pair<std::string, int>, std::optional<formats::RoleSprite>> m_sprites;
            std::map<FrameId, std::filesystem::path> m_hdFiles;
            std::map<FrameId, std::optional<formats::IndexedImage>> m_hdFrames;
    };

}

#endif
