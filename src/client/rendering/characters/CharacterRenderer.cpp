#include "client/rendering/characters/CharacterRenderer.h"

#include "client/data/EquipDrawOrder.h"

#include "formats/GamePixels.h"
#include "formats/Image.h"
#include "formats/PixelArtScale.h"
#include "gfx/RenderDevice.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <vector>

namespace client {

    namespace {

        constexpr int kWeaponExtraY = 68; // weapon sprites are drawn 68 px lower (Layer extraY)

        const std::set<int> kKeepEntry254 = {
            1159,
            1161,
            1162,
            1163,
            1164,
            1185,
            1186,
            1187,
            1189
        }; // sprite ids whose palette entry 254 is a real colour: it is not turned into the transparency key

        /// @brief Gives the sprite bank name prefix of a body type ("001" for body type 0).
        /// @param bodyType Body type 0..3.
        /// @return The prefix.
        std::string bankPrefix(int bodyType) { return "00" + std::to_string(bodyType + 1); }

        /// @brief Gives the first sprite id of a body type's range.
        /// @param bodyType Body type 0..3.
        /// @return The base id.
        int idBase(int bodyType) { return (bodyType + 1) * 1000; } // each body type owns a block of 1000 sprite ids

    }

    std::optional<CharacterRenderer::Layer> CharacterRenderer::equipmentLayer(const CharacterLook& look, int slot) {
        const int lookId = look.equipmentLooks[static_cast<std::size_t>(slot)];
        if (lookId <= 0) {
            return std::nullopt;
        }
        const std::string prefix = bankPrefix(look.bodyType);
        const auto firstWith = [&](std::initializer_list<std::string> banks, int sprite) -> std::optional<Layer> {
            for (const std::string& bank : banks) {
                if (m_sprites.contains(bank, sprite)) {
                    return Layer{bank, sprite, 0};
                }
            }
            return std::nullopt;
        };
        switch (slot) {
            case 1: // equipment slot 1: c bank
                return firstWith({prefix + "c", prefix + "c_c01"}, lookId);
            case 2: // equipment slot 2: clothes, e bank
                return firstWith({prefix + "e", prefix + "e_c01"}, lookId);
            case 3: { // equipment slot 3: weapon
                // weapons from xx700 up are in the w1 bank, the rest in w
                auto layer = lookId >= idBase(look.bodyType) + 700 ? firstWith({prefix + "w1", prefix + "w1_c01"}, lookId) : firstWith({prefix + "w"}, lookId);
                if (layer) {
                    layer->extraY = kWeaponExtraY;
                }
                return layer;
            }
            case 4: // equipment slot 4: a bank
                return firstWith({prefix + "a"}, lookId);
            case 5: // equipment slot 5: s bank
                return firstWith({prefix + "s", prefix + "s_c01"}, lookId);
            case 6: // equipment slot 6: the special slot
                if (look.specialType == 13) { // item type 13: cape
                    return firstWith({prefix + "c", prefix + "c_c01"}, lookId);
                }
                if (look.specialType == 12 || look.specialType == 16) { // item types 12 and 16: drawn from the e bank
                    return firstWith({prefix + "e", prefix + "e_c01"}, lookId);
                }
                return std::nullopt;
            default: // other slots show nothing
                return std::nullopt;
        }
    }

    std::optional<CharacterRenderer::Layer> CharacterRenderer::manteauPiece(const CharacterLook& look, int job) {
        // the special slot (6) holds a manteau when its look is 2000..5999 and ends in 347; jobs are 0..6
        const int worn = look.equipmentLooks[6];
        if (worn < 2000 || worn >= 6000 || worn % 1000 != 347 || job < 0 || job > 6) {
            return std::nullopt;
        }
        const std::string prefix = bankPrefix(look.bodyType);
        const int sprite = idBase(look.bodyType) + 348 + job; // manteau pieces start at sprite 348 of the body type's range
        for (const std::string& bank : {prefix + "e", prefix + "e_c01"}) {
            if (m_sprites.contains(bank, sprite)) {
                return Layer{bank, sprite, 0};
            }
        }
        return std::nullopt;
    }

    void CharacterRenderer::applyStatusTint(std::array<core::Color, 256>& palette, int kind) {
        // per status kind 0..8: red, green, blue shift added to the palette entries
        static constexpr std::array<std::array<int, 3>, 9> kShifts{
            {{0, 0, 0}, {125, -25, -25}, {-75, 50, -75}, {-100, 0, 75}, {50, 50, -100}, {-25, -25, 75}, {-100, 25, 50}, {75, 75, 75}, {-100, -100, -100}}
        };
        if (kind < 1 || kind > 8) { // status tint kinds are 1..8
            return;
        }
        constexpr std::size_t kFirst = 16; // palette entries 16..223 take the status shift (the rest are not shifted)
        constexpr std::size_t kLast = 223; // last shifted palette entry
        const auto& shift = kShifts[static_cast<std::size_t>(kind)];
        const auto channel = [](std::uint8_t value, int delta) {
            return static_cast<std::uint8_t>(std::clamp(value + delta, 5, 250));
        }; // each channel stays within 5..250
        for (std::size_t i = kFirst; i <= kLast; ++i) {
            palette[i].r = channel(palette[i].r, shift[0]);
            palette[i].g = channel(palette[i].g, shift[1]);
            palette[i].b = channel(palette[i].b, shift[2]);
        }
    }

    std::optional<std::reference_wrapper<const gfx::Texture>> CharacterRenderer::frameTexture(
        gfx::Canvas& canvas,
        const FrameKey& key,
        const formats::RoleSprite& sprite
    ) {
        auto found = m_textures.find(key);
        if (found == m_textures.end()) {
            const formats::RoleSpriteFrame& frame = sprite.frames[key.frame];
            std::optional<gfx::Texture> texture;
            if (!frame.pixels.empty()) {
                std::array<core::Color, 256> palette =
                    key.tint == Tint::None ? sprite.palette : formats::ColorPattern(key.digits).tint(sprite.palette, key.tint == Tint::Face);
                applyStatusTint(palette, key.statusTint);
                constexpr core::Color kMarked{255, 255, 10, 255}; // yellow (r, g, b, alpha) put into palette entry 255 of a marked sprite
                // palette entry 255 is yellow when marked, else the green transparency key (0,255,0); entry 254 is the key too unless the sprite keeps it
                palette[255] = key.marked ? kMarked : core::Color{0, 255, 0, 255};
                if (kKeepEntry254.count(key.sprite) == 0) {
                    palette[254] = {0, 255, 0, 255};
                }
                const auto hd = m_sprites.hdFrame(key.bank, key.sprite, key.frame, frame);
                const int width = hd ? hd->get().width : frame.width;
                const int height = hd ? hd->get().height : frame.height;
                const std::vector<std::uint8_t>& indices = hd ? hd->get().pixels : frame.pixels;
                formats::Image image(width, height);
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        const std::uint8_t index = indices[static_cast<std::size_t>(y * width + x)];
                        const core::Color color = palette[index];
                        const bool transparent = index == 0 || (color.r == 0 && color.g == 255 && color.b == 0); // index 0 or the pure green key is transparent
                        image.at(x, y) = transparent ? core::Color{0, 0, 0, 0} : color;
                    }
                }
                formats::quantizeTo16Bit(image);
                const int scaledDetail = hd ? 1 : canvas.device().pixelArtDetail(gfx::PixelArtPart::Sprites);
                if (scaledDetail > 1) {
                    texture =
                        gfx::Texture::create(canvas.device(), formats::scalePixelArt(image, canvas.device().pixelArtFilter(), scaledDetail), scaledDetail);
                } else {
                    texture = gfx::Texture::create(canvas.device(), image, hd ? 2 : 1); // an HD frame has twice the pixels across
                }
            }
            found = m_textures.emplace(key, std::move(texture)).first;
        }
        if (!found->second) {
            return std::nullopt;
        }
        return std::cref(*found->second);
    }

    void CharacterRenderer::drawLayer(
        gfx::Canvas& canvas,
        const Layer& layer,
        const formats::ColorPattern& pattern,
        Tint tint,
        core::Point feet,
        int slot,
        int frame,
        SpriteHighlight highlight
    ) {
        const auto sprite = m_sprites.find(layer.bank, layer.sprite);
        if (!sprite) {
            return;
        }
        const int count = sprite->get().frameCount(slot);
        if (count <= 0) {
            return;
        }
        const auto index = sprite->get().frameIndex(slot, frame % count);
        if (!index) {
            return;
        }
        const FrameKey key{layer.bank, layer.sprite, *index, pattern.digits(), tint, highlight.marked, highlight.statusTint};
        const auto texture = frameTexture(canvas, key, sprite->get());
        if (!texture) {
            return;
        }
        const formats::RoleSpriteFrame& data = sprite->get().frames[*index];
        if (highlight.scale != highlight.scaleDivisor) {
            const core::Rect area = scaledArea(data, layer, feet, highlight);
            if (!highlight.ghost) {
                if (highlight.opacity != 255) { // 255 is full opacity
                    canvas.blendTextureScaled(texture->get(), area, highlight.opacity);
                } else {
                    canvas.drawTextureScaled(texture->get(), area);
                }
            }
            if (highlight.light != 0) {
                canvas.drawTextureScaled(texture->get(), area, highlight.light);
            }
            return;
        }
        // the frame's canvas ends 32 pixels below the feet
        core::Point at{
            feet.x - data.canvasWidth / 2 + data.offsetX,
            feet.y - data.canvasHeight + data.offsetY + 32 + layer.extraY + (layer.extraY != 0 ? highlight.lowDrop : 0)
        };
        if (highlight.mirrored) {
            at.x = feet.x + data.canvasWidth / 2 - data.offsetX - data.width;
            canvas.drawTextureMirrored(texture->get(), at);
            return;
        }
        if (!highlight.ghost) {
            if (highlight.opacity != 255) { // 255 is full opacity
                canvas.blendTexture(texture->get(), at, highlight.opacity);
            } else {
                canvas.drawTexture(texture->get(), at);
            }
        }
        if (highlight.light != 0) {
            canvas.addTexture(texture->get(), at, highlight.light);
        }
    }

    core::Rect CharacterRenderer::scaledArea(const formats::RoleSpriteFrame& data, const Layer& layer, core::Point feet, SpriteHighlight highlight) {
        const int s = highlight.scale;
        const int d = highlight.scaleDivisor;
        const int drop = layer.extraY + (layer.extraY != 0 ? highlight.lowDrop : 0);
        const int x = feet.x - ((data.canvasWidth / 2 - data.offsetX) * s) / d;
        const int y = feet.y + drop - ((data.canvasHeight - data.offsetY - 32) * s) / d; // canvas ends 32 pixels below the feet
        return {x, y, data.width * s / d, data.height * s / d};
    }

    int CharacterRenderer::figureFrameCount(const CharacterLook& look, int slot) {
        const auto body = m_sprites.find(bankPrefix(look.bodyType), idBase(look.bodyType) + look.bodySprite);
        return body ? body->get().frameCount(slot) : 0;
    }

    core::Size CharacterRenderer::figureFrameSize(const CharacterLook& look, int slot) {
        const auto body = m_sprites.find(bankPrefix(look.bodyType), idBase(look.bodyType) + look.bodySprite);
        if (!body || body->get().frameCount(slot) <= 0) {
            return {1, 1};
        }
        const auto index = body->get().frameIndex(slot, 0);
        if (!index) {
            return {1, 1};
        }
        const formats::RoleSpriteFrame& frame = body->get().frames[*index];
        return {frame.width, frame.height};
    }

    core::Rect CharacterRenderer::figureFrameArea(const CharacterLook& look, core::Point feet, int slot) {
        const auto body = m_sprites.find(bankPrefix(look.bodyType), idBase(look.bodyType) + look.bodySprite);
        const auto index = body && body->get().frameCount(slot) > 0 ? body->get().frameIndex(slot, 0) : std::nullopt;
        if (!index) {
            return {feet.x, feet.y, 0, 0};
        }
        const formats::RoleSpriteFrame& frame = body->get().frames[*index];
        return {
            feet.x - frame.canvasWidth / 2 + frame.offsetX,
            feet.y - frame.canvasHeight + frame.offsetY + 32,
            frame.width,
            frame.height
        }; // canvas ends 32 pixels below the feet
    }

    void CharacterRenderer::drawFigure(
        gfx::Canvas& canvas,
        const CharacterLook& look,
        core::Point feet,
        int slot,
        int frame,
        SpriteHighlight highlight
    ) {
        const std::string prefix = bankPrefix(look.bodyType);
        const int base = idBase(look.bodyType);
        const Layer body{prefix, base + look.bodySprite, 0};
        const Layer head{prefix + "h", base + 100 + look.head, 0}; // heads start at sprite 100 of the body type's range

        drawLayer(canvas, body, look.pattern, Tint::Figure, feet, slot, frame, highlight);
        for (const int equipmentSlot : equipDrawOrder(look, slot, frame)) {
            if (equipmentSlot == 0) { // slot 0 in the draw order is the head
                drawLayer(canvas, head, look.pattern, Tint::Figure, feet, slot, frame, highlight);
            } else if (canDrawEquipSlot(look, equipmentSlot)) {
                if (const auto layer = equipmentLayer(look, equipmentSlot)) {
                    drawLayer(canvas, *layer, look.pattern, Tint::Figure, feet, slot, frame, highlight);
                }
                // base piece after slot 2; class part after slot 6 when its item type is 16 and the job is 1..6
                const bool base = equipmentSlot == 2 && look.equipmentLooks[2] > 0;
                const bool part = equipmentSlot == 6 && look.specialType == 16 && look.job >= 1 && look.job <= 6;
                if (base || part) {
                    if (const auto piece = manteauPiece(look, base ? 0 : look.job)) {
                        drawLayer(canvas, *piece, look.pattern, Tint::Figure, feet, slot, frame, highlight);
                    }
                }
            }
        }
    }

    void CharacterRenderer::drawSwimmer(
        gfx::Canvas& canvas,
        const CharacterLook& look,
        core::Point feet,
        int slot,
        int frame
    ) {
        const std::string prefix = bankPrefix(look.bodyType);
        const int base = idBase(look.bodyType);
        drawLayer(canvas, Layer{prefix, base + look.bodySprite, 0}, look.pattern, Tint::Figure, feet, slot, frame);
        drawLayer(
            canvas,
            Layer{prefix + "h", base + 100 + look.head, 0},
            look.pattern,
            Tint::Figure,
            feet,
            slot,
            frame
        ); // heads start at sprite 100 of the body type's range
    }

    std::optional<CharacterRenderer::Layer> CharacterRenderer::npcLayer(int spriteId) {
        /// @brief A later sprite bank and the first sprite id it holds.
        struct LaterBank {
            const char* name;
            int firstSprite;
        };

        // first sprite id held by each later 001 bank
        constexpr LaterBank kLaterBanks[] = {{"001_c02", 1761}, {"001_c01", 1704}, {"001_b01", 1593}, {"001_a01", 1486}, {"001_01", 1351}, {"001_1", 1247}};
        for (const LaterBank& bank : kLaterBanks) {
            if (bank.firstSprite <= spriteId && m_sprites.contains(bank.name, spriteId)) {
                return Layer{bank.name, spriteId, 0};
            }
        }
        if (m_sprites.contains("001", spriteId)) {
            return Layer{"001", spriteId, 0};
        }
        return std::nullopt;
    }

    std::optional<CharacterRenderer::Layer> CharacterRenderer::petLayer(int roleSpriteId) {
        /// @brief A later sprite bank and the first sprite id it holds.
        struct LaterBank {
            const char* name;
            int firstSprite;
        };

        // first sprite id held by each later 007 bank
        constexpr LaterBank kLaterBanks[] = {{"007_c01", 7287}, {"007_b01", 7258}, {"007_1", 7154}};
        for (const LaterBank& bank : kLaterBanks) {
            if (bank.firstSprite <= roleSpriteId && m_sprites.contains(bank.name, roleSpriteId)) {
                return Layer{bank.name, roleSpriteId, 0};
            }
        }
        if (m_sprites.contains("007", roleSpriteId)) {
            return Layer{"007", roleSpriteId, 0};
        }
        return std::nullopt;
    }

    int CharacterRenderer::petFrameCount(const formats::NpcInfo& npc, int slot) {
        const auto layer = petLayer(npc.roleSpriteId);
        const auto sprite = layer ? m_sprites.find(layer->bank, layer->sprite) : std::nullopt;
        return sprite ? sprite->get().frameCount(slot) : 0;
    }

    formats::ColorPattern CharacterRenderer::npcPattern(const formats::NpcInfo& npc) {
        formats::ColorPattern pattern;
        for (std::size_t part = 0; part < npc.colors.size(); ++part) {
            pattern.apply(npc.colors[part], static_cast<int>(part) + 1);
        }
        return pattern;
    }

    void CharacterRenderer::drawPet(
        gfx::Canvas& canvas,
        const formats::NpcInfo& npc,
        core::Point feet,
        int slot,
        int frame
    ) {
        if (const auto layer = petLayer(npc.roleSpriteId)) {
            drawLayer(canvas, *layer, npcPattern(npc), Tint::Figure, feet, slot, frame);
        }
    }

    int CharacterRenderer::npcFrameCount(int spriteId, int slot) {
        const auto layer = npcLayer(spriteId);
        const auto sprite = layer ? m_sprites.find(layer->bank, spriteId) : std::nullopt;
        return sprite ? sprite->get().frameCount(slot) : 0;
    }

    int CharacterRenderer::npcSlot(int spriteId, int directionKind, int slot) {
        int result = slot;
        switch (directionKind) {
            case 3: // direction kind 3: two standing facings, slots 8..11 map to 8 and 12..15 to 12
                // standing slots are 8..15; the first four share slot 8, the last four slot 12
                if (slot >= 8 && slot <= 11) {
                    result = 8;
                } else if (slot >= 12 && slot <= 15) {
                    result = 12;
                }
                break;
            case 5: // direction kind 5: standing slots 8..15 pair up (slot | 1)
                if (slot >= 8 && slot <= 15) {
                    result = slot | 1;
                }
                break;
            case 6: // direction kind 6: one facing (slot 12), or the wanted standing slot when the sprite has it
                result = 12;
                if (slot >= 8 && slot <= 15 && npcFrameCount(spriteId, slot) > 0) {
                    result = slot;
                }
                break;
            default: // other direction kinds keep the wanted slot
                break;
        }
        if (npcFrameCount(spriteId, result) > 0) {
            return result;
        }
        const int action = slot < 8 ? 0 : 8; // walking slots are 0..7, standing 8..15
        std::vector<int> present;
        for (int k = 0; k < 8; ++k) { // eight directions per action
            if (npcFrameCount(spriteId, action + k) > 0) {
                present.push_back(action + k);
            }
        }
        const int turn = slot - action;
        switch (present.size()) {
            case 1: // the sprite has one direction: use it
                return present.front();
            case 2: // two directions: turns 0..3 use direction 4 (south), the rest the next action's first
                return turn <= 3 ? action + 4 : action + 8;
            case 4: // four directions: pair the turns, taking the odd direction of each pair
                return action + (turn / 2) * 2 + 1;
            default: // other counts keep the earlier result
                return result;
        }
    }

    std::optional<CharacterRenderer::Layer> CharacterRenderer::npcLayerOf(const formats::NpcInfo& npc) {
        constexpr int kLargeDrop = 68; // large NPC sprites (spriteSize 1) are drawn 68 px lower
        constexpr int kTallDrop = 32; // and 32 px more for a tall kind (heightMode 1)
        auto layer = npcLayer(npc.spriteId);
        if (layer && npc.spriteSize == 1) {
            layer->extraY += kLargeDrop + (npc.heightMode == 1 ? kTallDrop : 0);
        }
        return layer;
    }

    void CharacterRenderer::drawNpc(
        gfx::Canvas& canvas,
        const formats::NpcInfo& npc,
        core::Point feet,
        int slot,
        int frame,
        SpriteHighlight highlight
    ) {
        if (const auto layer = npcLayerOf(npc)) {
            drawLayer(canvas, *layer, npcPattern(npc), Tint::Figure, feet, slot, frame, highlight);
        }
    }

    void CharacterRenderer::drawArmedNpc(
        gfx::Canvas& canvas,
        const formats::NpcInfo& npc,
        int weaponLook,
        std::int32_t weaponColorCode,
        core::Point feet,
        int slot,
        int frame,
        SpriteHighlight highlight
    ) {
        formats::ColorPattern pattern = npcPattern(npc);
        std::optional<Layer> weapon;
        const bool human = npc.bodyType >= 1 && npc.bodyType <= 4 &&
                           (npc.directionKind == 0 || npc.directionKind == 2); // NPC body types 1..4 with direction kind 0 or 2 are human-bodied
        if (human && weaponLook > idBase(npc.bodyType)) {
            CharacterLook look;
            look.bodyType = npc.bodyType;
            look.equipmentLooks[3] = weaponLook; // equipment slot 3 is the weapon
            weapon = equipmentLayer(look, 3); // equipment slot 3 is the weapon
            if (weapon) {
                pattern.apply(weaponColorCode, 4, 3); // the weapon's colour code goes to part 4
            }
        }
        if (const auto layer = npcLayerOf(npc)) {
            drawLayer(canvas, *layer, pattern, Tint::Figure, feet, slot, frame, highlight);
        }
        if (weapon) {
            drawLayer(canvas, *weapon, pattern, Tint::Figure, feet, slot, frame, highlight);
        }
    }

    bool CharacterRenderer::npcContains(
        const formats::NpcInfo& npc,
        core::Point feet,
        int slot,
        int frame,
        core::Point point,
        SpriteHighlight highlight
    ) {
        const auto layer = npcLayerOf(npc);
        return layer && layerContains(*layer, feet, slot, frame, point, highlight);
    }

    core::Size CharacterRenderer::npcFrameSize(int spriteId, int slot) {
        const auto layer = npcLayer(spriteId);
        const auto sprite = layer ? m_sprites.find(layer->bank, spriteId) : std::nullopt;
        const auto index = sprite && sprite->get().frameCount(slot) > 0 ? sprite->get().frameIndex(slot, 0) : std::nullopt;
        if (!index) {
            return {1, 1};
        }
        const formats::RoleSpriteFrame& frame = sprite->get().frames[*index];
        return {frame.width, frame.height};
    }

    int CharacterRenderer::rideHeight(const formats::NpcInfo& npc) {
        const auto layer = npcLayer(npc.spriteId);
        const auto sprite = layer ? m_sprites.find(layer->bank, npc.spriteId) : std::nullopt;
        const auto index = sprite && sprite->get().frameCount(0) > 0 ? sprite->get().frameIndex(0, 0) : std::nullopt;
        if (!index) {
            return 0;
        }
        // ride height: a tall kind (HeightMode 1) adds 68; a large sprite adds 96 and scales 1/10 of the frame height, others add 32 and scale 7/10; less 30
        int offset = npc.heightMode == 1 ? 68 : 0;
        int scale = 7;
        if (npc.spriteSize == 1) {
            offset += 96;
            scale = 1;
        } else {
            offset += 32;
        }
        const int height = static_cast<int>(std::ceil(scale * (sprite->get().frames[*index].height / 10.0))) + offset - 30;
        return std::max(height, 0);
    }

    int CharacterRenderer::rideRise(const formats::NpcInfo& npc, int slot, int frame) {
        constexpr int kSeat = 30; // px added to the ride rise (the same 30 is subtracted in rideHeight)
        const auto layer = npcLayer(npc.spriteId);
        const auto sprite = layer ? m_sprites.find(layer->bank, npc.spriteId) : std::nullopt;
        const int count = sprite ? sprite->get().frameCount(slot) : 0;
        const auto index = count > 0 ? sprite->get().frameIndex(slot, frame % count) : std::nullopt;
        if (!index) {
            return 0;
        }
        const formats::RoleSpriteFrame& data = sprite->get().frames[*index];
        const int above = data.canvasHeight - data.offsetY - 32; // canvas ends 32 pixels below the feet
        // a tall kind doubles the rise (a large sprite uses a 100 pixel margin and adds 100); a large sprite otherwise adds 68
        if (npc.heightMode == 1) {
            return npc.spriteSize == 1 ? -((data.canvasHeight - data.offsetY - 100) * 2) + kSeat + 100 : -(above * 2) + kSeat;
        }
        return -above + kSeat + (npc.spriteSize == 1 ? 68 : 0);
    }

    int CharacterRenderer::npcHeight(int spriteId, int spriteSize, int slot, int frame) {
        const auto layer = npcLayer(spriteId);
        const auto sprite = layer ? m_sprites.find(layer->bank, spriteId) : std::nullopt;
        if (!sprite) {
            return 0;
        }
        const int count = sprite->get().frameCount(slot);
        const auto index = slot == 0 && frame == 0 ? sprite->get().firstFrameIndex() : count > 0 ? sprite->get().frameIndex(slot, frame % count) : std::nullopt;
        if (!index) {
            return 0;
        }
        const int offsetY = sprite->get().frames[*index].offsetY;
        // large sprites (size 1) are 256 high with a 100 margin, others 128 with a 32 margin; 10 pixels above the head for the name
        return spriteSize == 1 ? 256 - offsetY - 100 + 10 : 128 - offsetY - 32 + 10;
    }

    void CharacterRenderer::drawCutInFace(
        gfx::Canvas& canvas,
        const CharacterLook& look,
        core::Point feet,
        int slot,
        int scale,
        int divisor
    ) {
        const Layer face{
            bankPrefix(look.bodyType) + "f",
            idBase(look.bodyType) + 660 + look.head,
            0
        }; // cut-in faces start at sprite 660 of the body type's range
        SpriteHighlight size;
        size.scale = scale;
        size.scaleDivisor = divisor;
        drawLayer(canvas, face, look.basePattern, Tint::Face, feet, slot, 0, size);
    }

    void CharacterRenderer::drawChatHead(gfx::Canvas& canvas, const CharacterLook& look, core::Point anchor, bool mirrored) {
        constexpr int kSlot = 4; // slot 4 of the face sprite
        constexpr int kFrame = 1; // frame 1 of that slot
        const Layer face{
            bankPrefix(look.bodyType) + "f",
            idBase(look.bodyType) + 600 + look.head,
            0
        }; // portrait faces start at sprite 600 of the body type's range
        SpriteHighlight highlight;
        highlight.mirrored = mirrored;
        drawLayer(canvas, face, look.basePattern, Tint::Face, anchor, kSlot, kFrame, highlight);
    }

    void CharacterRenderer::drawLifeFace(gfx::Canvas& canvas, const CharacterLook& look, core::Point anchor) {
        const Layer face{
            bankPrefix(look.bodyType) + "f",
            idBase(look.bodyType) + 640 + look.head,
            0
        }; // life faces start at sprite 640 of the body type's range
        drawLayer(canvas, face, look.basePattern, Tint::Face, anchor, 0, 0);
    }

    bool CharacterRenderer::figureContains(
        const CharacterLook& look,
        core::Point feet,
        int slot,
        int frame,
        core::Point point
    ) {
        const std::string prefix = bankPrefix(look.bodyType);
        const int base = idBase(look.bodyType);
        return layerContains(Layer{prefix, base + look.bodySprite, 0}, feet, slot, frame, point) ||
               layerContains(Layer{prefix + "h", base + 100 + look.head, 0}, feet, slot, frame, point);
    }

    bool CharacterRenderer::portraitContains(
        const CharacterLook& look,
        core::Point anchor,
        int slot,
        int frame,
        core::Point point
    ) {
        return layerContains(
            Layer{bankPrefix(look.bodyType) + "f", idBase(look.bodyType) + 600 + look.head, 0},
            anchor,
            slot,
            frame,
            point
        ); // portrait faces start at sprite 600
    }

    bool CharacterRenderer::chatHeadContains(const CharacterLook& look, core::Point anchor, core::Point point) {
        constexpr int kSlot = 4; // slot 4 of the face sprite
        constexpr int kFrame = 1; // frame 1 of that slot
        // portrait faces start at sprite 600 of the body type's range
        return layerContains(Layer{bankPrefix(look.bodyType) + "f", idBase(look.bodyType) + 600 + look.head, 0}, anchor, kSlot, kFrame, point);
    }

    bool CharacterRenderer::layerContains(
        const Layer& layer,
        core::Point feet,
        int slot,
        int frame,
        core::Point point,
        SpriteHighlight highlight
    ) {
        const auto sprite = m_sprites.find(layer.bank, layer.sprite);
        const int count = sprite ? sprite->get().frameCount(slot) : 0;
        if (count <= 0) {
            return false;
        }
        const auto index = sprite->get().frameIndex(slot, frame % count);
        if (!index) {
            return false;
        }
        const formats::RoleSpriteFrame& data = sprite->get().frames[*index];
        if (highlight.scale != highlight.scaleDivisor) {
            const core::Rect area = scaledArea(data, layer, feet, highlight);
            if (!area.contains(point) || area.width <= 0 || area.height <= 0) {
                return false;
            }
            const int x = (point.x - area.x) * data.width / area.width;
            const int y = (point.y - area.y) * data.height / area.height;
            return data.pixels[static_cast<std::size_t>(y * data.width + x)] != 0;
        }
        const int x = point.x - (feet.x - data.canvasWidth / 2 + data.offsetX);
        const int y = point.y - (feet.y - data.canvasHeight + data.offsetY + 32 + layer.extraY); // canvas ends 32 pixels below the feet
        if (x < 0 || y < 0 || x >= data.width || y >= data.height) {
            return false;
        }
        return data.pixels[static_cast<std::size_t>(y * data.width + x)] != 0;
    }

    void CharacterRenderer::drawFaceSprite(
        gfx::Canvas& canvas,
        int spriteId,
        core::Point anchor,
        int slot,
        int frame,
        const std::optional<formats::ColorPattern>& colors
    ) {
        for (const char* bank : {"008_c01", "008"}) { // 008 banks: face sprites
            if (m_sprites.contains(bank, spriteId)) {
                drawLayer(canvas, Layer{bank, spriteId, 0}, colors ? *colors : formats::ColorPattern{}, colors ? Tint::Face : Tint::None, anchor, slot, frame);
                return;
            }
        }
    }

    void CharacterRenderer::drawBustSprite(gfx::Canvas& canvas, const CharacterLook& look, int spriteId, core::Point anchor) {
        for (const char* bank : {"009", "009_c01"}) { // 009 banks: bust sprites
            if (m_sprites.contains(bank, spriteId)) {
                drawLayer(canvas, Layer{bank, spriteId, 0}, look.basePattern, Tint::Figure, anchor, 0, 0);
                return;
            }
        }
    }

    void CharacterRenderer::drawSignSprite(
        gfx::Canvas& canvas,
        int spriteId,
        core::Point feet,
        int slot,
        int frame
    ) {
        drawLayer(canvas, Layer{"009", spriteId, 0}, formats::ColorPattern{}, Tint::None, feet, slot, frame); // 009 bank: sign sprites
    }

    void CharacterRenderer::drawHitSprite(gfx::Canvas& canvas, int spriteId, core::Point feet, int frame) {
        drawLayer(
            canvas,
            Layer{"009_c01", spriteId, 0},
            formats::ColorPattern{},
            Tint::None,
            feet,
            0,
            frame
        ); // 009_c01 bank: the sprite shown on a player a thrown item hit
    }

    void CharacterRenderer::drawCampSprite(gfx::Canvas& canvas, const std::string& bank, int spriteId, core::Point feet) {
        drawLayer(canvas, Layer{bank, spriteId, 0}, formats::ColorPattern{}, Tint::None, feet, 0, 0);
    }

    bool CharacterRenderer::campContains(const std::string& bank, int spriteId, core::Point feet, core::Point point) {
        return layerContains(Layer{bank, spriteId, 0}, feet, 0, 0, point);
    }

    int CharacterRenderer::campFrameHeight(int spriteId) {
        const auto sprite = m_sprites.find("009", spriteId);
        const auto index = sprite && sprite->get().frameCount(0) > 0 ? sprite->get().frameIndex(0, 0) : std::nullopt;
        return index ? sprite->get().frames[*index].height : 0;
    }

    void CharacterRenderer::drawVehicle(
        gfx::Canvas& canvas,
        int spriteId,
        core::Point feet,
        int slot,
        int frame
    ) {
        drawLayer(canvas, Layer{"006", spriteId, 0}, formats::ColorPattern{}, Tint::None, feet, slot, frame); // 006 bank: vehicle sprites
    }

    int CharacterRenderer::vehicleFrameCount(int spriteId, int slot) {
        const auto sprite = m_sprites.find("006", spriteId);
        return sprite ? sprite->get().frameCount(slot) : 0;
    }

    bool CharacterRenderer::vehicleContains(
        int spriteId,
        core::Point feet,
        int slot,
        int frame,
        core::Point point
    ) {
        return layerContains(Layer{"006", spriteId, 0}, feet, slot, frame, point);
    }

    int CharacterRenderer::signFrameCount(int spriteId, int slot) {
        const auto sprite = m_sprites.find("009", spriteId);
        return sprite ? sprite->get().frameCount(slot) : 0;
    }

    void CharacterRenderer::drawMovieFace(
        gfx::Canvas& canvas,
        const CharacterLook& look,
        core::Point anchor,
        int frame,
        bool mirrored
    ) {
        constexpr int kMovieFaceSprite = 620; // movie faces start at sprite 620 of the body type's range
        SpriteHighlight highlight;
        highlight.mirrored = mirrored;
        const Layer face{bankPrefix(look.bodyType) + "f", idBase(look.bodyType) + kMovieFaceSprite + look.head, 0};
        drawLayer(canvas, face, look.basePattern, Tint::Face, anchor, 0, frame, highlight);
    }

    void CharacterRenderer::drawPortrait(
        gfx::Canvas& canvas,
        const CharacterLook& look,
        core::Point anchor,
        int frame,
        int slot,
        SpriteHighlight highlight
    ) {
        const Layer face{
            bankPrefix(look.bodyType) + "f",
            idBase(look.bodyType) + 600 + look.head,
            0
        }; // portrait faces start at sprite 600 of the body type's range
        drawLayer(canvas, face, look.basePattern, Tint::Face, anchor, slot, frame, highlight);
    }

}
