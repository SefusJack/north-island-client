#ifndef NORTH_ISLAND_CLIENT_RENDERING_HUD_PARTY_PANELS_H
#define NORTH_ISLAND_CLIENT_RENDERING_HUD_PARTY_PANELS_H

#include "client/rendering/characters/CharacterRenderer.h"
#include "client/services/party/PartyMembers.h"
#include "gfx/Canvas.h"
#include "gfx/PictureLibrary.h"

#include <cstdint>
#include <optional>
#include <vector>

namespace client {

    /// @brief The qol client's party rows: one row per member logged in with the played account, under the own status's gold line.
    /// @details Drawn as the world view's pet (TRe_MiniPetInfo): the member's chat head facing left where the pet's figure goes, then level, HP and SP bars in the pet's pictures; how far its login has come while it is not in the world. The name shows over the row while the mouse is on it.
    class PartyPanels {
        public:
            static constexpr int kLeft = 8; // x of the rows from the HUD origin, px
            static constexpr int kTop = 204; // y of the first row from the HUD origin, px
            static constexpr int kRowWidth = 72; // width of a row, px
            static constexpr int kRowHeight = 32; // row pitch, px

            /// @brief Creates the party rows painter.
            /// @param pictures Picture library for the pet pictures.
            /// @param characters Character renderer used for the chat heads.
            PartyPanels(
                gfx::PictureLibrary& pictures,
                CharacterRenderer& characters
            )
                : m_pictures(pictures),
                  m_characters(characters) {}

            /// @brief Works out the row of each member; a member among the played account's battle fighters has none and the others move up.
            /// @param members The party members.
            /// @param battleIds The played account's side's players in the battle's places.
            /// @return One row per member, -1 when it has none.
            static std::vector<int> rows(const std::vector<PartyMemberView>& members, const std::vector<std::int32_t>& battleIds);

            /// @brief Draws the rows and the hovered member's name.
            /// @param canvas Target canvas.
            /// @param members The party members.
            /// @param battleIds The played account's side's players in the battle's places.
            /// @param view The played account's screen (the window in the screen's coordinates).
            /// @param hudScale Size of the HUD.
            /// @param mouse Mouse position in the screen's coordinates, if any.
            void draw(
                gfx::Canvas& canvas,
                const std::vector<PartyMemberView>& members,
                const std::vector<std::int32_t>& battleIds,
                const core::Rect& view,
                float hudScale,
                std::optional<core::Point> mouse
            );

            /// @brief Finds the member whose row a point of the screen is on, as last drawn.
            /// @param point Point in the screen's coordinates.
            /// @return Member index, -1 for none.
            int memberAt(core::Point point) const;

            /// @brief Draws a member's name in a hint box (like every hint) above one of its windows drawn over the leader's screen, with a frame around it.
            /// @param canvas Target canvas.
            /// @param window Window rectangle.
            /// @param name Member name.
            static void drawWindowTag(gfx::Canvas& canvas, const core::Rect& window, const std::string& name);

        private:
            /// @brief Converts a point of the screen to the rows' own coordinates (unscaled, from the window's top left).
            /// @param point Point in the screen's coordinates.
            /// @return Point in the rows' coordinates.
            core::Point toRows(core::Point point) const;

            /// @brief Draws one member's row: chat head, level, HP and SP bars, or its login state.
            /// @param canvas Target canvas.
            /// @param member The member.
            /// @param at Top-left of the row.
            void drawRow(gfx::Canvas& canvas, const PartyMemberView& member, core::Point at);

            /// @brief Tiles the chat window's dithered grid (32X32Grid) over an area: the selected member's row.
            /// @param canvas Target canvas.
            /// @param area Area to fill.
            void drawGrid(gfx::Canvas& canvas, const core::Rect& area);

            gfx::PictureLibrary& m_pictures;
            CharacterRenderer& m_characters;
            core::Point m_pivot;
            float m_scale = 1.0f;
            std::vector<int> m_rows;
    };

}

#endif
