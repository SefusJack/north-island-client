#include "client/rendering/characters/RoleSpriteLibrary.h"

#include "core/FileSystem.h"
#include "core/Log.h"
#include "core/StringUtil.h"

namespace client {

    std::optional<std::reference_wrapper<formats::RoleSpriteFile>> RoleSpriteLibrary::openBank(const std::string& bank) {
        auto found = m_banks.find(bank);
        if (found == m_banks.end()) {
            auto file = formats::RoleSpriteFile::open(m_game.data("jma/" + bank + ".jma"), m_game.data("jma/" + bank + ".Jxa"));
            if (!file) {
                core::logWarning("sprite bank ", bank, " cannot be opened");
            }
            found = m_banks.emplace(bank, std::move(file)).first;
        }
        if (!found->second) {
            return std::nullopt;
        }
        return std::ref(*found->second);
    }

    bool RoleSpriteLibrary::contains(const std::string& bank, int spriteId) {
        const auto file = openBank(bank);
        return file && file->get().contains(spriteId);
    }

    std::optional<std::reference_wrapper<const formats::RoleSprite>> RoleSpriteLibrary::find(const std::string& bank, int spriteId) {
        const auto key = std::make_pair(bank, spriteId);
        auto found = m_sprites.find(key);
        if (found == m_sprites.end()) {
            std::optional<formats::RoleSprite> sprite;
            if (auto file = openBank(bank)) {
                sprite = file->get().read(spriteId);
            }
            found = m_sprites.emplace(key, std::move(sprite)).first;
        }
        if (!found->second) {
            return std::nullopt;
        }
        return std::cref(*found->second);
    }

    void RoleSpriteLibrary::addHdFrame(const std::string& bank, int spriteId, int frame, const std::filesystem::path& file) {
        const FrameId id{core::toLower(bank), spriteId, static_cast<std::size_t>(frame)};
        m_hdFiles[id] = file;
        m_hdFrames.erase(id);
    }

    std::optional<std::reference_wrapper<const formats::IndexedImage>> RoleSpriteLibrary::hdFrame(
        const std::string& bank,
        int spriteId,
        std::size_t frame,
        const formats::RoleSpriteFrame& original
    ) {
        if (m_hdFiles.empty()) {
            return std::nullopt;
        }
        const FrameId id{core::toLower(bank), spriteId, frame};
        auto found = m_hdFrames.find(id);
        if (found == m_hdFrames.end()) {
            std::optional<formats::IndexedImage> image;
            if (const auto file = m_hdFiles.find(id); file != m_hdFiles.end()) {
                const auto data = core::readFile(file->second);
                image = data ? formats::decodeIndexedPng(*data) : std::nullopt;
                if (!image) {
                    core::logWarning("the HD sprite frame ", file->second.string(), " is not an 8-bit paletted PNG: the original is used");
                } else if (image->width != original.width * 2 || image->height != original.height * 2) {
                    core::logWarning(
                        "the HD sprite frame ",
                        file->second.string(),
                        " is ",
                        image->width,
                        "x",
                        image->height,
                        ", not ",
                        original.width * 2,
                        "x",
                        original.height * 2,
                        ": the original is used"
                    );
                    image.reset();
                }
            }
            found = m_hdFrames.emplace(id, std::move(image)).first;
        }
        if (!found->second) {
            return std::nullopt;
        }
        return std::cref(*found->second);
    }

}
