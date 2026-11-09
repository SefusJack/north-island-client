#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_EMBLEM_PAGE_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_EMBLEM_PAGE_H

#include "client/services/GameServices.h"
#include "client/models/guild/EmblemEditor.h"
#include "client/models/guild/Guild.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Point.h"
#include "formats/EmblemBlock.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <functional>
#include <optional>

namespace client {

    /// @brief The Insignia page of the guild tab: a 24 x 24 badge editor with tools, palette, undo/redo and samples.
    /// @details Original: DrawOrgEmblemPage, RenderWithChildren (controls in GuildWindow.xml); Upload sends the badge with 39/18; samples are the .ens files of the user folder.
    class GuildEmblemPage {
        public:
            /// @brief Wires the page's controls.
            /// @param services Shared game services.
            /// @param screen Screen holding the controls.
            /// @param guild The guild model.
            /// @param messages Sink for short player messages.
            /// @param ownId Gives the own player's id.
            /// @param ownBadge Gives the own guild's badge, if it has one.
            GuildEmblemPage(
                GameServices& services,
                ui::Screen& screen,
                const Guild& guild,
                PlayerMessages& messages,
                std::function<std::int32_t()> ownId,
                std::function<std::optional<formats::EmblemBlock>()> ownBadge
            );

            /// @brief Reads the samples again: the own badge, then the .ens files.
            /// @details Original: TSe_TeamForm.ShowCtrl, LoadEmblemSamples.
            void loadSamples();

            /// @brief Shows or hides the page's controls and marks the chosen tool.
            /// @param shown True when the page is the current tab.
            void layout(bool shown);

            /// @brief Paints the drafting board, palette, grid, badge, samples and the stroke being drawn.
            /// @param canvas Target canvas.
            /// @param at Top left of the tab on the canvas.
            void paint(gfx::Canvas& canvas, core::Point at);

        private:
            /// @brief Writes the grid as the sample file and reads the samples again.
            /// @details Original: SaveEmblemSample.
            void saveSample();

            /// @brief Sends the badge to the server when the member has the right.
            /// @details Original: UploadGuildBadge (39/18).
            void upload();

            /// @brief Paints a badge.
            /// @details Original: DrawEmblemOnSurface.
            /// @param canvas Target canvas.
            /// @param grid Badge to paint.
            /// @param at Top left of the badge.
            void paintBadge(gfx::Canvas& canvas, const EmblemGrid& grid, core::Point at);

            /// @brief Paints the cells of a stroke in the chosen colour.
            /// @details Original: DrawLinePreview and the others.
            /// @param canvas Target canvas.
            /// @param at Top left of the tab on the canvas.
            /// @param cells Grid cells of the stroke.
            void paintCells(gfx::Canvas& canvas, core::Point at, const std::vector<core::Point>& cells);

            GameServices& m_services;
            ui::Screen& m_screen;
            const Guild& m_guild;
            PlayerMessages& m_messages;
            std::function<std::int32_t()> m_ownId;
            std::function<std::optional<formats::EmblemBlock>()> m_ownBadge;
            const ui::Spot& m_boardSpot;
            const ui::Spot& m_paletteSpot;
            const ui::Spot& m_gridSpot;
            const ui::Spot& m_badgeSpot;
            const ui::Spot& m_samplesSpot;
            const ui::Spot& m_cursorSpot;
            EmblemEditor m_editor;
    };

}

#endif
