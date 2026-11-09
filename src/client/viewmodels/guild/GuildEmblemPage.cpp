#include "client/viewmodels/guild/GuildEmblemPage.h"

#include "client/models/guild/EmblemCells.h"
#include "client/rendering/painters/EmblemPainter.h"
#include "core/FileSystem.h"
#include "core/StringUtil.h"
#include "formats/EmblemPalette.h"
#include "protocol/GuildRequests.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Tooltip.h"

#include <algorithm>
#include <filesystem>

namespace client {

    namespace {

        constexpr int kSwatchPitch = 15; // distance between palette swatches, px
        constexpr int kSwatchSize = 9; // size of the colour square cut from the palette picture, px
        constexpr int kMessageMs = 3000; // notice display time, ms
        constexpr int kBadgeRight = 5; // guild authority number 5 (1-based): may change the guild badge
        const char* const kSampleExtension = ".ens"; // sample badge file extension

        const char* const kButtons[] = {
            "GuildEmblemUndo",
            "GuildEmblemRedo",
            "GuildEmblemUpload",
            "GuildEmblemClear",
            "GuildEmblemRectangle",
            "GuildEmblemEllipse",
            "GuildEmblemLine",
            "GuildEmblemReadFile",
            "GuildEmblemSaveFile",
            "GuildEmblemPen",
            "GuildEmblemFill",
            "GuildEmblemSamplesLeft",
            "GuildEmblemSamplesRight",
            "GuildEmblemFrame",
            "GuildEmblemSamples",
            "GuildEmblemPalette",
            "GuildEmblemGrid"
        };

        /// @brief A tool button name and the tool it chooses.
        struct ToolButton {
            const char* name;
            EmblemEditor::Tool tool;
        };

        const ToolButton kToolButtons[] = {
            {"GuildEmblemPen", EmblemEditor::Tool::Pen},
            {"GuildEmblemFill", EmblemEditor::Tool::Fill},
            {"GuildEmblemLine", EmblemEditor::Tool::Line},
            {"GuildEmblemRectangle", EmblemEditor::Tool::Rectangle},
            {"GuildEmblemEllipse", EmblemEditor::Tool::Ellipse}
        };

        /// @brief Gives the place of a palette colour's swatch in the palette picture.
        /// @param color Palette colour 0..15.
        /// @return The swatch rectangle.
        core::Rect swatchOf(int color) {
            // the palette has 8 colours a row and its swatches start 4 px in
            return {color % 8 * kSwatchPitch + 4, color / 8 * kSwatchPitch + 4, kSwatchSize, kSwatchSize};
        }

    }

    GuildEmblemPage::GuildEmblemPage(
        GameServices& services,
        ui::Screen& screen,
        const Guild& guild,
        PlayerMessages& messages,
        std::function<std::int32_t()> ownId,
        std::function<std::optional<formats::EmblemBlock>()> ownBadge
    )
        : m_services(services),
          m_screen(screen),
          m_guild(guild),
          m_messages(messages),
          m_ownId(std::move(ownId)),
          m_ownBadge(std::move(ownBadge)),
          m_boardSpot(screen.get<ui::Spot>("GuildEmblemBoard")),
          m_paletteSpot(screen.get<ui::Spot>("GuildEmblemPaletteAt")),
          m_gridSpot(screen.get<ui::Spot>("GuildEmblemGridAt")),
          m_badgeSpot(screen.get<ui::Spot>("GuildEmblemBadge")),
          m_samplesSpot(screen.get<ui::Spot>("GuildEmblemSamplesAt")),
          m_cursorSpot(screen.get<ui::Spot>("GuildEmblemCursor")) {
        screen.on("onGuildEmblemPen", [this](const ui::EventArgs&) { m_editor.selectTool(EmblemEditor::Tool::Pen); });
        screen.on("onGuildEmblemFill", [this](const ui::EventArgs&) { m_editor.selectTool(EmblemEditor::Tool::Fill); });
        screen.on("onGuildEmblemLine", [this](const ui::EventArgs&) { m_editor.selectTool(EmblemEditor::Tool::Line); });
        screen.on("onGuildEmblemRectangle", [this](const ui::EventArgs&) { m_editor.selectTool(EmblemEditor::Tool::Rectangle); });
        screen.on("onGuildEmblemEllipse", [this](const ui::EventArgs&) { m_editor.selectTool(EmblemEditor::Tool::Ellipse); });
        screen.on("onGuildEmblemUndo", [this](const ui::EventArgs&) { m_editor.undo(); });
        screen.on("onGuildEmblemRedo", [this](const ui::EventArgs&) { m_editor.redo(); });
        screen.on("onGuildEmblemClear", [this](const ui::EventArgs&) { m_editor.clear(); });
        screen.on("onGuildEmblemUpload", [this](const ui::EventArgs&) { upload(); });
        screen.on("onGuildEmblemReadFile", [this](const ui::EventArgs&) {
            loadSamples();
            m_messages.show("Load the group badge model", kMessageMs);
        });
        screen.on("onGuildEmblemSaveFile", [this](const ui::EventArgs&) { saveSample(); });
        screen.on("onGuildEmblemSamplesLeft", [this](const ui::EventArgs&) { m_editor.scrollSamplesLeft(); });
        screen.on("onGuildEmblemSamplesRight", [this](const ui::EventArgs&) { m_editor.scrollSamplesRight(); });
        screen.on("onGuildEmblemSampleDown", [this](const ui::EventArgs& args) {
            for (int place = 0; place < EmblemEditor::kSamplesShown; ++place) {
                const core::Rect
                    cell{4 + place * m_samplesSpot.pitch().x, 4, m_samplesSpot.rect().width, m_samplesSpot.rect().height}; // 4: pixel inset of the first sample
                if (cell.contains(args.position)) {
                    m_editor.pickSample(place);
                    return;
                }
            }
        });
        screen.on("onGuildEmblemPaletteDown", [this](const ui::EventArgs& args) {
            // the palette is 2 rows of 8 colours; each hit box starts 4 px in and is 14 by 14
            for (int row = 0; row < 2; ++row) {
                for (int column = 0; column < 8; ++column) {
                    if (core::Rect{4 + column * kSwatchPitch, 4 + row * kSwatchPitch, 14, 14}.contains(args.position)) {
                        m_editor.selectColor(row * 8 + column);
                        return;
                    }
                }
            }
        });
        screen.on("onGuildEmblemGridDown", [this](const ui::EventArgs& args) {
            m_editor.press({args.position.x / m_gridSpot.pitch().x, args.position.y / m_gridSpot.pitch().y});
        });
        screen.on("onGuildEmblemGridDrag", [this](const ui::EventArgs& args) {
            if (core::Rect{0, 0, m_gridSpot.rect().width, m_gridSpot.rect().height}.contains(args.position)) {
                m_editor.drag({args.position.x / m_gridSpot.pitch().x, args.position.y / m_gridSpot.pitch().y});
            } else {
                m_editor.cancelStroke();
            }
        });
        screen.on("onGuildEmblemGridUp", [this](const ui::EventArgs&) { m_editor.release(); });
    }

    void GuildEmblemPage::layout(bool shown) {
        for (const char* name : kButtons) {
            m_screen.get<ui::Element>(name).setVisible(shown);
        }
        const EmblemEditor::Tool tool = m_editor.tool();
        for (const ToolButton& button : kToolButtons) {
            m_screen.get<ui::ImageButton>(button.name).setSelected(tool == button.tool);
        }
    }

    void GuildEmblemPage::loadSamples() {
        m_editor.clearSamples();
        if (const auto own = m_ownBadge()) {
            m_editor.addSample(EmblemGrid::decode(*own), "My guild badge");
        }
        const std::filesystem::path folder = m_services.game.userRoot();
        std::error_code error;
        std::vector<std::string> files;
        for (const auto& entry : std::filesystem::directory_iterator(folder, error)) {
            const std::string name = entry.path().filename().string();
            if (entry.is_regular_file() && core::toLower(entry.path().extension().string()) == kSampleExtension) {
                files.push_back(name);
            }
        }
        std::sort(files.begin(), files.end(), [](const std::string& a, const std::string& b) { return core::toLower(a) < core::toLower(b); });
        std::vector<std::string> names;
        bool lastOne = false;
        for (std::size_t i = 0; i < files.size(); ++i) {
            const std::string name = files[i].substr(0, files[i].find('.'));
            if (i > 0 && name == "sample255") { // the file named sample255 is moved to the end of the list
                lastOne = true;
            } else {
                names.push_back(name);
            }
        }
        if (lastOne) {
            names.push_back("sample255");
        }
        formats::EmblemBlock block;
        for (const std::string& name : names) {
            if (const auto data = core::readFile(folder / (name + kSampleExtension))) {
                block = formats::EmblemBlock::parse(*data);
            }
            m_editor.addSample(EmblemGrid::decode(block), name);
        }
    }

    void GuildEmblemPage::saveSample() {
        const std::filesystem::path folder = m_services.game.userRoot();
        std::error_code error;
        std::filesystem::create_directories(folder, error);
        core::writeFile(folder / (std::string("MySample") + kSampleExtension), m_editor.grid().encode().bytes());
        loadSamples();
        m_messages.show("Save the group badge model", kMessageMs);
    }

    void GuildEmblemPage::upload() {
        const int own = m_guild.indexOf(m_ownId());
        if (own >= 0 && !m_guild.at(own).record.rights[kBadgeRight - 1]) {
            m_messages.show("Not enough authorization for you to change guild badge", kMessageMs);
            return;
        }
        m_services.connection.send(protocol::guildBadgeRequest(m_editor.grid().encode().bytes()));
        m_messages.show("Upload the guild badge", kMessageMs);
    }

    void GuildEmblemPage::paintBadge(gfx::Canvas& canvas, const EmblemGrid& grid, core::Point at) { EmblemPainter::paint(canvas, grid.encode(), at); }

    void GuildEmblemPage::paintCells(gfx::Canvas& canvas, core::Point at, const std::vector<core::Point>& cells) {
        const auto mix = m_services.pictures.find("icon_mixpaints_1");
        if (!mix) {
            return;
        }
        for (const core::Point& cell : cells) {
            canvas.drawPicture(
                *mix,
                {at.x + m_gridSpot.rect().x + cell.x * m_gridSpot.pitch().x, at.y + m_gridSpot.rect().y + cell.y * m_gridSpot.pitch().y},
                swatchOf(m_editor.paintColor())
            );
        }
    }

    void GuildEmblemPage::paint(gfx::Canvas& canvas, core::Point at) {
        const auto mix = m_services.pictures.find("icon_mixpaints_1");
        if (const auto board = m_services.pictures.find("icon_drafting_1")) {
            canvas.drawPicture(*board, at + m_boardSpot.at());
        }
        if (mix) {
            canvas.drawPicture(*mix, at + m_paletteSpot.at());
        }
        const int colour = m_editor.paintColor();
        // cursor frame 18 by 18 starting 2 px inside the swatch cell, pen width 2
        canvas.drawRectOutline(
            {at.x + m_paletteSpot.at().x + colour % 8 * kSwatchPitch + 2, at.y + m_paletteSpot.at().y + colour / 8 * kSwatchPitch + 2, 18, 18},
            m_cursorSpot.color(),
            2
        );
        if (mix) {
            for (int x = 0; x < EmblemGrid::kSide; ++x) {
                for (int y = 0; y < EmblemGrid::kSide; ++y) {
                    canvas.drawPicture(
                        *mix,
                        {at.x + m_gridSpot.rect().x + x * m_gridSpot.pitch().x, at.y + m_gridSpot.rect().y + y * m_gridSpot.pitch().y},
                        swatchOf(m_editor.grid().cells[static_cast<std::size_t>(x)][static_cast<std::size_t>(y)])
                    );
                }
            }
        }
        paintBadge(canvas, m_editor.grid(), at + m_badgeSpot.at());
        const int count = static_cast<int>(m_editor.samples().size());
        for (int place = 0; place < EmblemEditor::kSamplesShown; ++place) {
            const int index = m_editor.sampleScroll() + place;
            if (index < count) {
                paintBadge(
                    canvas,
                    m_editor.samples()[static_cast<std::size_t>(index)],
                    {at.x + m_samplesSpot.rect().x + place * m_samplesSpot.pitch().x, at.y + m_samplesSpot.rect().y}
                );
            }
        }
        if (m_editor.dragging()) {
            switch (m_editor.tool()) {
                case EmblemEditor::Tool::Line: paintCells(canvas, at, lineCells(m_editor.lineStart(), m_editor.lineEnd())); break; // line preview
                case EmblemEditor::Tool::Rectangle:
                    paintCells(canvas, at, rectangleCells(m_editor.lineStart(), m_editor.lineEnd()));
                    break; // rectangle preview
                case EmblemEditor::Tool::Ellipse: paintCells(canvas, at, ellipseCells(m_editor.lineStart(), m_editor.lineEnd())); break; // ellipse preview
                default: break; // pen and fill have no preview
            }
        }
        for (int place = 0; place < EmblemEditor::kSamplesShown; ++place) {
            const int index = m_editor.sampleScroll() + place;
            const core::Rect cell{
                at.x + m_samplesSpot.rect().x + place * m_samplesSpot.pitch().x,
                at.y + m_samplesSpot.rect().y,
                m_samplesSpot.rect().width,
                m_samplesSpot.rect().height
            };
            if (index < count && cell.contains(m_screen.mouse())) {
                ui::drawTooltipAt(
                    canvas,
                    {cell.x + 60, cell.y},
                    m_editor.sampleNames()[static_cast<std::size_t>(index)]
                ); // 60: tooltip offset right of the sample in pixels
                break;
            }
        }
    }

}
