#ifndef NORTH_ISLAND_CLIENT_RENDERING_HUD_PARTY_PANELS_H
#define NORTH_ISLAND_CLIENT_RENDERING_HUD_PARTY_PANELS_H

#include "client/rendering/characters/CharacterRenderer.h"
#include "client/services/party/PartyMembers.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The qol client's party rows: one row per other own character (logged in on this client besides the selected, played
    /// one), then a "+" row that logs another account in while there is room.
    /// @details A left click on a row makes that character the selected one (PartyMembers::select), in the world and in battle; a
    /// right click asks to log it out (WorldScene::askPartyLogOut). Drawn as the world view's pet (TRe_MiniPetInfo): the member's
    /// chat head facing left where the pet's figure goes, then level, HP and SP bars in the pet's pictures; how far its login has
    /// come while it is not in the world. The name shows over the row while the mouse is on it. The rows scale with the HUD and keep
    /// to the screen's top left, like the own status.
    ///
    /// Placement (unscaled HUD pixels, an 800 x 600 screen; at most 4 rows of kRowHeight, 128 px):
    /// - World: x 8, from y 300. Above them the own status (HP, SP, EXP, gold lines to y 190) and its vehicle gauge (StatusVehicle
    ///   at 0,195,200,100: the icon and rails from y 200); below them the chat window (ChatForm from y 475 at its default size, its
    ///   frame from about 465) and the chat channel list (from y 448, x 28). The minimap, function, main and hot key buttons and the
    ///   time effect icons keep to the right and bottom right.
    /// - Battle: x 8, from y 204. The vehicle gauge is not shown in battle, while the member panels (136 px from the top) and the
    ///   own HP, SP, EXP and gold under them (to y 200) are; the rows end at y 332, left of the fighters: the left side's back
    ///   line (BattleField, row 1) stands at x 210, 185, 160, 135 with its feet at y 266, 329, 392, 455, and the command ring
    ///   reaches 68 px left of and 96 px above a fighter's feet (BattleMainForm kHomes), so its nearest buttons are at x 142
    ///   (y 170-305), 117 (233-368), 92 (296-431) and 67 (359-494), right of or under the rows' box (x 6-78). The right side
    ///   (x 540 and over) and the auto-cancel button (762,526) are far off.
    class PartyPanels {
        public:
            static constexpr int kLeft = 8; // x of the rows from the HUD origin, px
            static constexpr int kWorldTop = 300; // y of the first row in the world, px: under the vehicle gauge (195 + 100)
            static constexpr int kBattleTop = 204; // y of the first row in battle, px: under the own gold line (y 188, 12 px high)
            static constexpr int kRowWidth = 72; // width of a row, px
            static constexpr int kRowHeight = 32; // row pitch, px
            static constexpr std::uint64_t kBlinkMs = 500; // ms the attention outline is on, then off

            /// @brief Creates the party rows painter.
            /// @param pictures Picture library for the pet pictures.
            /// @param characters Character renderer used for the chat heads.
            PartyPanels(
                gfx::PictureLibrary& pictures,
                CharacterRenderer& characters
            )
                : m_pictures(pictures),
                  m_characters(characters) {}

            /// @brief Draws the rows, the "+" row and the hovered row's hint.
            /// @param canvas Target canvas.
            /// @param members The party members, a row each.
            /// @param loginRow Draw the "+" row after the members' rows.
            /// @param battle The played account is in a battle (the rows go higher, see the class).
            /// @param view The played account's screen (the window in the screen's coordinates).
            /// @param hudScale Size of the HUD.
            /// @param mouse Mouse position in the screen's coordinates, if any.
            /// @param nowMs The current time in milliseconds (the attention outline's blink).
            void draw(
                gfx::Canvas& canvas,
                const std::vector<PartyMemberView>& members,
                bool loginRow,
                bool battle,
                const core::Rect& view,
                float hudScale,
                std::optional<core::Point> mouse,
                std::uint64_t nowMs
            );

            /// @brief Finds the member whose row a point of the screen is on, as last drawn.
            /// @param point Point in the screen's coordinates.
            /// @return Member index, -1 for none (the "+" row included).
            int memberAt(core::Point point) const;

            /// @brief Tells whether a point of the screen is on the "+" row, as last drawn.
            /// @param point Point in the screen's coordinates.
            /// @return True on the "+" row.
            bool loginRowAt(core::Point point) const;

            /// @brief Draws a member's name in a hint box (like every hint) above one of its windows drawn over the selected character's screen, with a frame around it.
            /// @param canvas Target canvas.
            /// @param window Window rectangle.
            /// @param name The character's name.
            static void drawWindowTag(gfx::Canvas& canvas, const core::Rect& window, const std::string& name);

        private:
            /// @brief Converts a point of the screen to the rows' own coordinates (unscaled, from the window's top left).
            /// @param point Point in the screen's coordinates.
            /// @return Point in the rows' coordinates.
            core::Point toRows(core::Point point) const;

            /// @brief Finds the row a point of the screen is on, as last drawn.
            /// @param point Point in the screen's coordinates.
            /// @return Row index (the "+" row is the one after the members'), -1 for none.
            int rowAt(core::Point point) const;

            /// @brief Draws the "+" row.
            /// @param canvas Target canvas.
            /// @param at Top-left of the row.
            /// @param hovered The mouse is on it.
            void drawLoginRow(gfx::Canvas& canvas, core::Point at, bool hovered);

            /// @brief Draws one member's row: chat head, level, HP and SP bars, or its login state.
            /// @param canvas Target canvas.
            /// @param member The member.
            /// @param at Top-left of the row.
            void drawRow(gfx::Canvas& canvas, const PartyMemberView& member, core::Point at);

            gfx::PictureLibrary& m_pictures;
            CharacterRenderer& m_characters;
            core::Point m_pivot;
            float m_scale = 1.0f;
            int m_top = kWorldTop; // y of the first row as last drawn, px
            int m_memberRows = 0; // members' rows as last drawn
            bool m_loginRow = false; // the "+" row was last drawn
    };

}

#endif
