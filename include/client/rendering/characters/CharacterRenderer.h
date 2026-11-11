#ifndef NORTH_ISLAND_CLIENT_RENDERING_CHARACTERS_CHARACTER_RENDERER_H
#define NORTH_ISLAND_CLIENT_RENDERING_CHARACTERS_CHARACTER_RENDERER_H

#include "client/models/character/CharacterLook.h"
#include "client/rendering/characters/RoleSpriteLibrary.h"
#include "client/rendering/characters/SpriteHighlight.h"
#include "core/Point.h"
#include "core/Rect.h"
#include "core/Size.h"
#include "formats/NpcInfo.h"
#include "gfx/Canvas.h"
#include "gfx/Texture.h"

#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <tuple>

namespace client {

    /// @brief Draws player characters, NPCs, pets and vehicles from their role sprites, and the portraits.
    /// @details The figure is body, equipment and head in the game's layer order, without the map's shadow. Recoloured frames are
    /// uploaded once and kept. Positions are the character's feet: a frame's canvas is centred on x and ends 32 pixels below y
    /// (weapons another 68). Action slots: walking 0..7, standing 8..15 (direction 0 = north, counter-clockwise: 1 north-west ... 7 north-east).
    class CharacterRenderer {
        public:
            /// @brief Creates the renderer.
            /// @param sprites Role sprite library.
            explicit CharacterRenderer(
                RoleSpriteLibrary& sprites
            )
                : m_sprites(sprites) {}

            /// @brief Draws a character's figure (body, equipment, head).
            /// @param canvas Target canvas.
            /// @param look The character's look.
            /// @param feet Screen position of the feet.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            /// @param highlight Mouse-over highlight.
            void drawFigure(
                gfx::Canvas& canvas,
                const CharacterLook& look,
                core::Point feet,
                int slot,
                int frame,
                SpriteHighlight highlight = {}
            );

            /// @brief Draws the face from the face bank; slot 2 is the large face of the select and creation screens, slot 0 the one of the status panel.
            /// @param canvas Target canvas.
            /// @param look The character's look.
            /// @param anchor Screen anchor of the portrait.
            /// @param frame Animation frame.
            /// @param slot Face slot.
            /// @param highlight Mouse-over highlight.
            void drawPortrait(
                gfx::Canvas& canvas,
                const CharacterLook& look,
                core::Point anchor,
                int frame,
                int slot = 2, // sprite slot 2 by default
                SpriteHighlight highlight = {}
            );

            /// @brief Draws the large face of a movie's role.
            /// @details Original: DrawMovieRoleFace; the face bank's sprite 20 after the speaker's, direction 0, frame 0 to 2.
            /// @param canvas Target canvas.
            /// @param look The role's look.
            /// @param anchor Screen anchor.
            /// @param frame Face frame 0..2.
            /// @param mirrored Whether the face is turned.
            void drawMovieFace(
                gfx::Canvas& canvas,
                const CharacterLook& look,
                core::Point anchor,
                int frame,
                bool mirrored
            );

            /// @brief Draws the large face of the skill cut-in, scaled on its feet.
            /// @details The face banks' sprite 660 + head (as the portraits' 600 + head); slot 0 faces left, 1 right.
            /// @param canvas Target canvas.
            /// @param look The character's look.
            /// @param feet Screen position of the feet.
            /// @param slot 0 facing left, 1 facing right.
            /// @param scale Scale numerator.
            /// @param divisor Scale divisor.
            void drawCutInFace(
                gfx::Canvas& canvas,
                const CharacterLook& look,
                core::Point feet,
                int slot,
                int scale,
                int divisor
            );

            /// @brief Draws the small head beside a chat line.
            /// @details Original: DrawLineSenderHead (the portrait's face sprite in slot 4, frame 1).
            /// @param canvas Target canvas.
            /// @param look The sender's look.
            /// @param anchor Screen anchor.
            /// @param mirrored Turned to face left (the qol party rows).
            void drawChatHead(gfx::Canvas& canvas, const CharacterLook& look, core::Point anchor, bool mirrored = false);

            /// @brief Draws the small face of the Q&A screen's lives, the manufacture partners' form and the Trojan War disguise.
            /// @details Original: DrawConundrumRoles (face banks' sprite 640 + head, slot 0, frame 0; the original's FHead + 2641 of body type 1
            /// is the same), DrawMakeFormPreviewPlayer.
            /// @param canvas Target canvas.
            /// @param look The character's look.
            /// @param anchor Screen anchor.
            void drawLifeFace(gfx::Canvas& canvas, const CharacterLook& look, core::Point anchor);

            /// @brief Tells whether a screen point is on a drawn pixel of a figure (its body or head in that slot and frame).
            /// @details Original: FMouseOver.
            /// @param look The character's look.
            /// @param feet Screen position of the feet.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            /// @param point Screen point.
            /// @return True when on a drawn pixel.
            bool figureContains(
                const CharacterLook& look,
                core::Point feet,
                int slot,
                int frame,
                core::Point point
            );

            /// @brief Tells whether a screen point is on a drawn pixel of a portrait.
            /// @details Original: FMouseOver.
            /// @param look The character's look.
            /// @param anchor Screen anchor.
            /// @param slot Face slot.
            /// @param frame Animation frame.
            /// @param point Screen point.
            /// @return True when on a drawn pixel.
            bool portraitContains(
                const CharacterLook& look,
                core::Point anchor,
                int slot,
                int frame,
                core::Point point
            );

            /// @brief Tells whether a screen point is on a drawn pixel of the chat head.
            /// @details Original: FMouseOver.
            /// @param look The sender's look.
            /// @param anchor Screen anchor.
            /// @param point Screen point.
            /// @return True when on a drawn pixel.
            bool chatHeadContains(const CharacterLook& look, core::Point anchor, core::Point point);

            /// @brief Draws a face of the 008 banks, untinted unless colours are given.
            /// @details Original: DrawPreviewHuman for body type 0 in a list (the GM's 8084). With colors, an NPC's four colour codes tint
            /// its face as DrawRoleSprite does for a speaker that is not a player (the hair parts).
            /// @param canvas Target canvas.
            /// @param spriteId Face sprite id.
            /// @param anchor Screen anchor.
            /// @param slot Face slot.
            /// @param frame Animation frame.
            /// @param colors Optional colour pattern.
            void drawFaceSprite(
                gfx::Canvas& canvas,
                int spriteId,
                core::Point anchor,
                int slot,
                int frame,
                const std::optional<formats::ColorPattern>& colors = std::nullopt
            );

            /// @brief Gives the NPC kind's four colour codes as a pattern.
            /// @details Original: LoadFromNpcTable.
            /// @param npc The NPC kind.
            /// @return The colour pattern.
            static formats::ColorPattern npcPattern(const formats::NpcInfo& npc);

            /// @brief Draws the bust of a Picture Frame's form: a sprite of the 009 banks in the person's own colours.
            /// @details Original: TPortraitForm.DrawControl.
            /// @param canvas Target canvas.
            /// @param look The person's look.
            /// @param spriteId Bust sprite id.
            /// @param anchor Screen anchor.
            void drawBustSprite(gfx::Canvas& canvas, const CharacterLook& look, int spriteId, core::Point anchor);

            /// @brief Counts the frames the figure has in an action slot (for animation).
            /// @param look The character's look.
            /// @param slot Action slot.
            /// @return The frame count.
            int figureFrameCount(const CharacterLook& look, int slot);

            /// @brief Gives the image size of the body's first frame in an action slot.
            /// @details Original: GetBodySpriteWidth / Height.
            /// @param look The character's look.
            /// @param slot Action slot.
            /// @return The size.
            core::Size figureFrameSize(const CharacterLook& look, int slot);

            /// @brief Gives where the body's first frame in a slot lies on the screen for feet there (what the mouse can point at).
            /// @param look The character's look.
            /// @param feet Screen position of the feet.
            /// @param slot Action slot.
            /// @return The screen rectangle.
            core::Rect figureFrameArea(const CharacterLook& look, core::Point feet, int slot);

            /// @brief Picks the slot an NPC sprite really has for a wanted one (some sprites face fewer ways).
            /// @details Original: GetSpriteDirection. NPCs are sprites of the 001 banks in their own colours.
            /// @param spriteId NPC sprite id.
            /// @param directionKind How many ways the sprite faces.
            /// @param slot Wanted slot.
            /// @return The slot to use.
            int npcSlot(int spriteId, int directionKind, int slot);

            /// @brief Counts the frames an NPC sprite has in a slot.
            /// @param spriteId NPC sprite id.
            /// @param slot Action slot.
            /// @return The frame count.
            int npcFrameCount(int spriteId, int slot);

            /// @brief Gives the image size of the NPC's first frame in a slot.
            /// @details Original: GetBodySpriteWidth / Height.
            /// @param spriteId NPC sprite id.
            /// @param slot Action slot.
            /// @return The size.
            core::Size npcFrameSize(int spriteId, int slot);

            /// @brief Draws the NPC kind's map sprite in its four colour codes.
            /// @details Original: LoadFromNpcTable.
            /// @param canvas Target canvas.
            /// @param npc The NPC kind.
            /// @param feet Screen position of the feet.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            /// @param highlight Mouse-over highlight.
            void drawNpc(
                gfx::Canvas& canvas,
                const formats::NpcInfo& npc,
                core::Point feet,
                int slot,
                int frame,
                SpriteHighlight highlight = {}
            );

            /// @brief Draws a pet of a human body type holding its weapon.
            /// @details Original: DrawHumanLayers for FHumKind 4: the weapon bank of the pet's body type by the look id of the item it wears,
            /// in the pet's colours with the weapon's part 4 colour code.
            /// @param canvas Target canvas.
            /// @param npc The pet's NPC kind.
            /// @param weaponLook Look id of the weapon item (0: none).
            /// @param weaponColorCode The weapon's part 4 colour code.
            /// @param feet Screen position of the feet.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            /// @param highlight Mouse-over highlight.
            void drawArmedNpc(
                gfx::Canvas& canvas,
                const formats::NpcInfo& npc,
                int weaponLook,
                std::int32_t weaponColorCode,
                core::Point feet,
                int slot,
                int frame,
                SpriteHighlight highlight = {}
            );

            /// @brief Tells whether a screen point is on a drawn pixel of that NPC.
            /// @details Original: FMouseOver.
            /// @param npc The NPC kind.
            /// @param feet Screen position of the feet.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            /// @param point Screen point.
            /// @param highlight Mouse-over highlight.
            /// @return True when on a drawn pixel.
            bool npcContains(
                const formats::NpcInfo& npc,
                core::Point feet,
                int slot,
                int frame,
                core::Point point,
                SpriteHighlight highlight = {}
            );

            /// @brief Gives how high the NPC's frame reaches above its feet (for the name).
            /// @details Original: FSpriteHeight.
            /// @param spriteId NPC sprite id.
            /// @param spriteSize Sprite size class.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            /// @return The height in pixels.
            int npcHeight(int spriteId, int spriteSize, int slot, int frame);

            /// @brief Gives how much higher a rider's name, bubble and emotion go over a pet of this kind.
            /// @details Original: LoadFromNpcTable, FRideHeight.
            /// @param npc The pet's NPC kind.
            /// @return The extra height in pixels.
            int rideHeight(const formats::NpcInfo& npc);

            /// @brief Gives where a rider's feet lie against the pet's, as a shift of y (negative: up), for the pet's frame in a slot.
            /// @details Original: DrawMovieRoleSprite.
            /// @param npc The pet's NPC kind.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            /// @return The y shift.
            int rideRise(const formats::NpcInfo& npc, int slot, int frame);

            /// @brief Draws a figure in shallow water: the body and the head in the swimming action (66 and the direction), nothing worn.
            /// @details Original: DrawHumanSprite, FCellAttr 8.
            /// @param canvas Target canvas.
            /// @param look The character's look.
            /// @param feet Screen position of the feet.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            void drawSwimmer(
                gfx::Canvas& canvas,
                const CharacterLook& look,
                core::Point feet,
                int slot,
                int frame
            );

            /// @brief Draws a sprite of the 009 banks as it is (the battle sign over a fighting player, 9000).
            /// @details Original: DrawRoleSprite of RoleShow_009.
            /// @param canvas Target canvas.
            /// @param spriteId Sprite id.
            /// @param feet Screen position of the feet.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            void drawSignSprite(
                gfx::Canvas& canvas,
                int spriteId,
                core::Point feet,
                int slot,
                int frame
            );

            /// @brief Counts the frames of a 009 bank sprite in a slot.
            /// @param spriteId Sprite id.
            /// @param slot Action slot.
            /// @return The frame count.
            int signFrameCount(int spriteId, int slot);

            /// @brief Draws the sprite 9081 in a fixed frame for a player a thrown item hit.
            /// @details Original: DrawRoleSprite of RoleShow_009_c01 (FEmoteFx 1).
            /// @param canvas Target canvas.
            /// @param spriteId Sprite id.
            /// @param feet Screen position of the feet.
            /// @param frame Animation frame.
            void drawHitSprite(gfx::Canvas& canvas, int spriteId, core::Point feet, int frame);

            /// @brief Counts the frames of a pet's or fighter's role sprite (007 banks) in a slot.
            /// @param npc The NPC kind.
            /// @param slot Action slot.
            /// @return The frame count.
            int petFrameCount(const formats::NpcInfo& npc, int slot);

            /// @brief Draws a tent: the first frame of a sprite of the 009 banks standing on a point.
            /// @details Original: DrawCamp.
            /// @param canvas Target canvas.
            /// @param bank Sprite bank ("009" or "009_c01").
            /// @param spriteId Sprite id.
            /// @param feet Screen position of the feet.
            void drawCampSprite(gfx::Canvas& canvas, const std::string& bank, int spriteId, core::Point feet);

            /// @brief Tells whether the bank holds the sprite.
            /// @param bank Sprite bank.
            /// @param spriteId Sprite id.
            /// @return True when it does.
            bool hasSprite(const std::string& bank, int spriteId) const { return m_sprites.contains(bank, spriteId); }

            /// @brief Tells whether a screen point is on a drawn pixel of a tent's first frame.
            /// @param bank Sprite bank.
            /// @param spriteId Sprite id.
            /// @param feet Screen position of the feet.
            /// @param point Screen point.
            /// @return True when on a drawn pixel.
            bool campContains(const std::string& bank, int spriteId, core::Point feet, core::Point point);

            /// @brief Gives how high a tent's first frame is.
            /// @param spriteId Sprite id.
            /// @return The height in pixels.
            int campFrameHeight(int spriteId);

            /// @brief Draws a pet or fighter: the NPC kind's role sprite of the 007 banks in its four colour codes.
            /// @param canvas Target canvas.
            /// @param npc The NPC kind.
            /// @param feet Screen position of the feet.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            void drawPet(
                gfx::Canvas& canvas,
                const formats::NpcInfo& npc,
                core::Point feet,
                int slot,
                int frame
            );

            /// @brief Draws a vehicle: the sprite of the 006 bank a look id names (its front layer is the next one), in its own colours.
            /// @details Original: DrawRoleSprite of RoleShow_006. Slot: walking 0..7, standing 8..15.
            /// @param canvas Target canvas.
            /// @param spriteId Look id of the vehicle sprite.
            /// @param feet Screen position of the feet.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            void drawVehicle(
                gfx::Canvas& canvas,
                int spriteId,
                core::Point feet,
                int slot,
                int frame
            );

            /// @brief Counts the frames of a vehicle sprite in a slot.
            /// @param spriteId Look id of the vehicle sprite.
            /// @param slot Action slot.
            /// @return The frame count.
            int vehicleFrameCount(int spriteId, int slot);

            /// @brief Tells whether a screen point is on a drawn pixel of a vehicle.
            /// @param spriteId Look id of the vehicle sprite.
            /// @param feet Screen position of the feet.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            /// @param point Screen point.
            /// @return True when on a drawn pixel.
            bool vehicleContains(
                int spriteId,
                core::Point feet,
                int slot,
                int frame,
                core::Point point
            );

        private:
            /// @brief How a sprite takes its colours.
            enum class Tint { Figure, Face, None };

            /// @brief Shifts a palette's colours by a status's tint.
            /// @details Original: TintPaletteByKind: a status's colour over the palette's entries 16..223 (TintPaletteRange, each channel kept within 5..250).
            /// @param palette The palette to change.
            /// @param kind Status tint kind 1..8, other values change nothing.
            static void applyStatusTint(std::array<core::Color, 256>& palette, int kind); // 256-colour sprite palette

            /// @brief The key of a recoloured frame texture in the cache.
            struct FrameKey {
                std::string bank;
                int sprite;
                std::size_t frame;
                std::array<std::uint8_t, formats::ColorPattern::kSize> digits;
                Tint tint;
                bool marked = false;
                int statusTint = 0;

                /// @brief Orders keys so they can index the texture cache.
                /// @param other The key to compare with.
                /// @return True when this key sorts before the other.
                bool operator<(const FrameKey& other) const {
                    return std::tie(bank, sprite, frame, digits, tint, marked, statusTint) <
                           std::tie(other.bank, other.sprite, other.frame, other.digits, other.tint, other.marked, other.statusTint);
                }
            };

            /// @brief One sprite to draw: its bank, sprite id and extra drop.
            struct Layer {
                std::string bank;
                int sprite = 0;
                int extraY = 0;
            };

            /// @brief Finds the bank and sprite of what the character wears in an equipment slot.
            /// @details Sprites past the main bank of a kind live in its _c01 bank; weapons below xx700 are in the w bank, the rest in w1.
            /// @param look The character's look.
            /// @param slot Equipment slot 1..6.
            /// @return The layer, nothing when the slot shows nothing.
            std::optional<Layer> equipmentLayer(const CharacterLook& look, int slot);

            /// @brief Finds a piece a manteau adds.
            /// @details Original: DrawX347WeaponBase (job 0) and DrawX347WeaponPart (job 1..6); sprite 348 + job of the body type's range in the e banks.
            /// @param look The character's look.
            /// @param job 0 for the base piece, 1..6 for the class part.
            /// @return The layer, nothing for another look or a class outside 1..6.
            std::optional<Layer> manteauPiece(const CharacterLook& look, int job);

            /// @brief Tells whether a point is on a drawn pixel of the layer's frame placed as drawLayer places it.
            /// @param layer The layer.
            /// @param feet Screen position of the feet.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            /// @param point Screen point.
            /// @param highlight Scale and drop settings.
            /// @return True when on a drawn pixel.
            bool layerContains(
                const Layer& layer,
                core::Point feet,
                int slot,
                int frame,
                core::Point point,
                SpriteHighlight highlight = {}
            );

            /// @brief Gives where a scaled frame goes.
            /// @details Original: DrawMovieRoleSprite with DrawScale / ScaleDiv.
            /// @param data The frame.
            /// @param layer The layer.
            /// @param feet Screen position of the feet.
            /// @param highlight Scale settings.
            /// @return The screen rectangle.
            static core::Rect scaledArea(const formats::RoleSpriteFrame& data, const Layer& layer, core::Point feet, SpriteHighlight highlight);

            /// @brief Finds an NPC's layer with the drop of a large sprite.
            /// @details Original: DrawMovieRoleSprite; a large sprite (SpriteSize 1) sits 68 pixels lower, a tall one (HeightMode 1) 32 more.
            /// @param npc The NPC kind.
            /// @return The layer, nothing when the sprite is missing.
            std::optional<Layer> npcLayerOf(const formats::NpcInfo& npc);

            /// @brief Finds the 001 bank layer of an NPC sprite.
            /// @details Original: FindSpriteBank; the last of the later banks whose first sprite id is not above it, else 001 itself.
            /// @param spriteId NPC sprite id.
            /// @return The layer, nothing when no bank holds the sprite.
            std::optional<Layer> npcLayer(int spriteId);

            /// @brief Finds the 007 bank layer of a pet's role sprite.
            /// @details Original: FindSpriteBank for RoleShow_007; the later banks hold the sprites from their first one on.
            /// @param roleSpriteId Role sprite id.
            /// @return The layer, nothing when no bank holds the sprite.
            std::optional<Layer> petLayer(int roleSpriteId);

            /// @brief Draws one layer's frame, recoloured, placed from the feet.
            /// @param canvas Target canvas.
            /// @param layer The layer.
            /// @param pattern Colour pattern.
            /// @param tint How the sprite takes its colours.
            /// @param feet Screen position of the feet.
            /// @param slot Action slot.
            /// @param frame Animation frame.
            /// @param highlight Highlight, scale and mirror settings.
            void drawLayer(
                gfx::Canvas& canvas,
                const Layer& layer,
                const formats::ColorPattern& pattern,
                Tint tint,
                core::Point feet,
                int slot,
                int frame,
                SpriteHighlight highlight = {}
            );

            /// @brief Gives the recoloured texture of a frame, building and caching it on first use.
            /// @param canvas Canvas whose device uploads the texture.
            /// @param key Cache key.
            /// @param sprite The sprite holding the frame.
            /// @return The texture, nothing when the frame has no pixels.
            std::optional<std::reference_wrapper<const gfx::Texture>> frameTexture(gfx::Canvas& canvas, const FrameKey& key, const formats::RoleSprite& sprite);

            RoleSpriteLibrary& m_sprites;
            std::map<FrameKey, std::optional<gfx::Texture>> m_textures;
    };

}

#endif
