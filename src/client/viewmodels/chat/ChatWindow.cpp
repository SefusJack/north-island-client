#include "client/viewmodels/chat/ChatWindow.h"

#include "client/data/EmoteCodes.h"
#include "client/rules/TextFilter.h"

#include "gfx/TextStyle.h"

#include <algorithm>

namespace client {

    namespace {

        constexpr int kCharWidth = 8; // width of one character, px
        constexpr int kIconSize = 16; // emote icon size, px
        constexpr std::uint64_t kIconFrameMs = 210; // time per emote icon frame, ms
        constexpr int kScreenWidth = 800; // game screen width, px
        constexpr int kScreenHeight = 600; // game screen height, px
        constexpr int kMinWidth = 224; // smallest chat form width, px
        constexpr int kMinHeight = 72; // smallest chat form height, px
        constexpr int kResizeOffset = 24; // resize button size / offset from the form's right edge, px

        // slices of the "Panel_TalkForm2_1" strip picture (see-through look): top, repeated middle, bottom
        constexpr int kStripWidth = 45; // strip width, px
        constexpr int kStripTop = 38; // height of the top slice, px
        constexpr int kStripMiddle = 12; // height of the repeated middle slice, px
        constexpr int kStripBottom = 36; // height of the bottom slice, px

        /// @brief Loads ChatWindow.xml into the screen.
        /// @param services Game services (layout, load context).
        /// @param screen Screen to load into.
        /// @return The loaded panel.
        ui::Element& loadPanel(GameServices& services, ui::Screen& screen) {
            return screen.load(services.game.layout("ChatWindow.xml"), services.loadContext());
        }

        /// @brief Draws a one-pixel dash-dot line along one side (Windows' psDashDot).
        /// @param canvas Target canvas.
        /// @param from Start point.
        /// @param length Length in px.
        /// @param horizontal True for a horizontal line, else vertical.
        /// @param color Line colour.
        void dashDot(
            gfx::Canvas& canvas,
            core::Point from,
            int length,
            bool horizontal,
            core::Color color
        ) {
            constexpr int kPattern[] = {9, 3, 3, 3}; // run lengths in px: dash 9, gap 3, dot 3, gap 3
            int at = 0;
            for (int part = 0; at < length; part = (part + 1) % 4) { // the pattern has four parts: dash, gap, dot, gap
                const int run = std::min(kPattern[part], length - at);
                if (part % 2 == 0) {
                    canvas.fillRect(horizontal ? core::Rect{from.x + at, from.y, run, 1} : core::Rect{from.x, from.y + at, 1, run}, color);
                }
                at += kPattern[part];
            }
        }

    }

    ChatWindow::ChatWindow(
        GameServices& services,
        ui::Screen& screen
    )
        : m_services(services),
          m_screen(screen),
          m_panel(loadPanel(services, screen)),
          m_form(screen.get<ui::Element>("ChatForm")),
          m_scrollBar(screen.get<ui::ScrollBar>("ChatScroll")),
          m_dragArea(screen.get<ui::DrawingArea>("ChatDragArea")),
          m_resize(screen.get<ui::DrawingArea>("ChatResize")),
          m_lines(screen.get<ui::DrawingArea>("ChatLines")),
          m_switch(screen.get<ui::ImageButton>("ChatMode")),
          m_marqueeArea(screen.get<ui::DrawingArea>("ChatMarquee")),
          m_rowSpot(screen.get<ui::Spot>("ChatRow")),
          m_systemSpot(screen.get<ui::Spot>("ChatSystemColor")),
          m_previewSpot(screen.get<ui::Spot>("ChatPreviewColor")) {
        screen.on("onChatMode", [this](const ui::EventArgs&) { setLook((m_look + 1) % 3); }); // three looks: dithered, see-through, hidden
        screen.on("onChatScrolled", [this](const ui::EventArgs& args) {
            const std::size_t count = m_log.lineCount();
            const std::size_t top = static_cast<std::size_t>(std::max(args.index, 0));
            const std::size_t shown = static_cast<std::size_t>(rows());
            m_scrolledBack = count > shown + top ? count - shown - top : 0;
        });
        screen.on("onChatScrollUp", [this](const ui::EventArgs&) { scroll(1); });
        screen.on("onChatLock", [this](const ui::EventArgs&) { toggleLock(); });
        screen.on("onChatLinesDown", [this](const ui::EventArgs&) {
            if (m_hoveredHead != 0 && m_whisper) {
                m_whisper(m_hoveredHead, true);
            }
        });
        screen.on("onChatNamesDouble", [this](const ui::EventArgs& args) {
            const core::Rect names = m_screen.get<ui::Element>("ChatNames").bounds();
            const auto line = lineAt({args.position.x + names.x - m_lines.bounds().x, args.position.y + names.y - m_lines.bounds().y});
            if (line && m_whisper) {
                const std::int32_t sender = m_log.line(*line).senderId;
                if (sender != 0 && sender != m_ownId) {
                    m_whisper(sender, false);
                }
            }
        });
        screen.on("onChatScrollDown", [this](const ui::EventArgs&) { scroll(-1); });
        screen.on("onChatFormDown", [this](const ui::EventArgs& args) { m_grab = args.position; });
        screen.on("onChatFormDrag", [this](const ui::EventArgs& args) {
            core::Rect area = m_form.bounds();
            area.x += args.position.x - m_grab.x;
            area.y += args.position.y - m_grab.y;
            // snap to the top within 10 px; the bottom edge is held at 568, moved to 578
            if (area.y < 10) {
                area.y = 0;
            }
            if (area.y + area.height > 568) {
                area.y = 578 - area.height;
            }
            m_form.setBounds(area);
            keepOnScreen();
        });
        screen.on("onChatResizeDown", [this](const ui::EventArgs& args) {
            m_grab = args.position;
            m_resizing = true;
        });
        screen.on("onChatResizeDrag", [this](const ui::EventArgs& args) {
            const core::Rect form = m_form.bounds();
            core::Rect button = m_resize.bounds();
            const int x = button.x + args.position.x - m_grab.x;
            const int y = button.y + args.position.y - m_grab.y;
            button.x = std::clamp(x, -form.x, kScreenWidth - form.x - button.width);
            if (form.y + y >= 0 && form.y + y + button.height <= kScreenHeight) {
                button.y = y;
            }
            m_resize.setBounds(button);
        });
        screen.on("onChatResizeUp", [this](const ui::EventArgs&) {
            if (m_resizing) {
                applyResize();
            }
        });
        screen.get<ui::DrawingArea>("ChatBackground").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintBackground(canvas, at); });
        m_lines.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paintLines(canvas, at, nowMs);
            m_lines.setHitTest(m_hoveredHead != 0);
        });
        m_resize.setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paintResizeButton(canvas, at); });
        screen.get<ui::DrawingArea>("ChatResizePreview").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            paintResizePreview(canvas, at);
        });
        screen.get<ui::DrawingArea>("ChatMarquee").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
            if (!m_marquee.text().empty()) {
                canvas.drawText(at, m_marquee.text(), {m_systemSpot.color(), gfx::TextEffect::Shadow, std::nullopt}, m_marqueeArea.bounds().width);
            }
        });
        screen.get<ui::DrawingArea>("ChatAnnounce").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t) { paintAnnounce(canvas); });
        arrange();
    }

    void ChatWindow::paintAnnounce(gfx::Canvas& canvas) {
        if (!m_bannerShown) {
            return;
        }
        constexpr int kLeft = 365; // banner left edge on screen, px
        constexpr int kTopOnMap = 37; // banner top below the window top, px
        constexpr int kHeight = 15; // banner height, px
        const int top = canvas.window().top() + kTopOnMap;
        // strip background: 3 px margin each side, RGBA {61, 139, 249, 150} translucent blue
        canvas.fillRectAlpha({kLeft - 3, top - 1, AnnounceBanner::kStripWidth + 6, kHeight + 2}, {61, 139, 249, 150});
        const AnnounceBanner::Visible& visible = *m_bannerShown;
        canvas.setClip(core::Rect{kLeft + visible.destination, top, visible.sourceRight - visible.sourceLeft, kHeight});
        // banner text colour: yellow RGBA {255, 255, 0, 255}
        canvas.drawText({kLeft + visible.destination - visible.sourceLeft, top}, m_banner.text(), {{255, 255, 0, 255}, gfx::TextEffect::Outline, std::nullopt});
        canvas.setClip(std::nullopt);
    }

    void ChatWindow::setLook(int look) {
        m_look = look;
        const bool lines = look != 2;
        m_screen.get<ui::Element>("ChatSolidFrame").setVisible(look == 0);
        m_dragArea.setHitTest(look == 0);
        m_resize.setVisible(look == 0);
        m_lines.setVisible(lines);
        m_screen.get<ui::Element>("ChatLock").setVisible(lines);
        m_screen.get<ui::Element>("ChatNames").setVisible(look == 0);
        m_scrollBar.setVisible(lines);
        m_screen.get<ui::Element>("ChatScrollUp").setVisible(lines);
        m_screen.get<ui::Element>("ChatScrollDown").setVisible(lines);
        m_switch.setPosition({look == 2 ? 0 : 24, m_switch.bounds().y}); // switch button x: left edge when hidden, else 24
    }

    void ChatWindow::arrange() {
        const core::Rect form = m_form.bounds();
        const auto place = [this](const char* name, core::Rect area) { m_screen.get<ui::Element>(name).setBounds(area); };
        place("ChatBackground", {0, 0, form.width, form.height});
        place("ChatSolidFrame", {0, 0, form.width, form.height});
        m_dragArea.setBounds({0, 0, form.width, form.height});
        // lines area: x 50, y 14; 65 px right margin, 26 px bottom margin
        m_lines.setBounds({50, 14, form.width - 50 - 65, form.height - 14 - 26});
        m_log.setLineChars(static_cast<std::size_t>(m_lines.bounds().width / kCharWidth));
        // names column: x 50, y 14, 80 px wide; scroll bar at x 10, y 32 with a 25 px bottom margin; down arrow at x 8, 6 px above the bottom
        place("ChatNames", {50, 14, 80, form.height - 14 - 26});
        m_scrollBar.setBounds({10, 32, m_scrollBar.bounds().width, form.height - 32 - 25});
        const core::Rect down = m_screen.get<ui::Element>("ChatScrollDown").bounds();
        place("ChatScrollDown", {8, form.height - 6 - down.height, down.width, down.height});
        m_switch.setPosition({m_switch.bounds().x, form.height - m_switch.bounds().height});
        m_resize.setPosition({form.width - kResizeOffset, 2}); // resize button y offset
        syncScrollBar();
        m_locked = true;
        toggleLock();
    }

    void ChatWindow::keepOnScreen() {
        core::Rect area = m_form.bounds();
        // snap to an edge within 10 px: left at x < 10, right past 790, bottom past 565 (moved to 575), top at y < 10
        if (area.x < 10) {
            area.x = 0;
        }
        if (area.x + area.width > 790 && area.x > 0) {
            area.x = kScreenWidth - area.width;
        }
        if (area.y + area.height > 565) {
            area.y = 575 - area.height;
        }
        if (area.y < 10 && area.y != 575 - area.height) {
            area.y = 0;
        }
        m_form.setBounds(area);
    }

    void ChatWindow::applyResize() {
        m_resizing = false;
        core::Rect form = m_form.bounds();
        const core::Rect button = m_resize.bounds();
        const int bottom = form.y + form.height;
        int screenX = form.x + button.x;
        int screenY = form.y + button.y;
        if (screenX + kResizeOffset - form.x > kScreenWidth) {
            screenX = form.x + kScreenWidth - kResizeOffset;
        }
        // the button sits 2 px below the form's top; sizes are rounded to even numbers; the bottom edge is held at 566
        if (bottom - (screenY - 2) > kScreenHeight) {
            screenY = form.y + kScreenHeight - 2;
        }
        form.width = std::max(kMinWidth, (screenX + kResizeOffset - form.x) / 2 * 2);
        form.height = std::max(kMinHeight, (bottom - (screenY - 2)) / 2 * 2);
        form.y = (screenY - 2) / 2 * 2;
        if (form.y + form.height > 566) {
            form.y = 566 - form.height;
        }
        m_form.setBounds(form);
        keepOnScreen();
        arrange();
        m_scrolledBack = std::min(m_scrolledBack, m_log.lineCount());
        syncScrollBar();
    }

    int ChatWindow::rows() const { return std::max(1, m_lines.bounds().height / m_rowSpot.pitch().y); }

    void ChatWindow::add(int channel, const std::string& message, std::int32_t senderId, bool announce) {
        const std::size_t added = m_log.add(channel, message, senderId);
        if (m_observer) {
            m_observer(message);
        }
        if (m_services.chatHeard) {
            m_services.chatHeard(channel, message, senderId);
        }
        if (!m_scrollBar.isDragging()) {
            if (!m_locked) {
                m_scrolledBack = 0;
            } else if (senderId != 0 && senderId == m_ownId) {
                toggleLock();
                m_scrolledBack = 0;
            } else {
                const std::size_t count = m_log.lineCount();
                const std::size_t shown = static_cast<std::size_t>(rows());
                m_scrolledBack = std::min(m_scrolledBack + added, count > shown ? count - shown : 0);
            }
        }
        if (announce && channel == 0 && isTextClean(message)) {
            m_marquee.add(message);
        }
        syncScrollBar();
    }

    void ChatWindow::clear() {
        m_log.clear();
        m_scrolledBack = 0;
        syncScrollBar();
    }

    void ChatWindow::toggleLock() {
        m_locked = !m_locked;
        auto& lock = m_screen.get<ui::ImageButton>("ChatLock");
        lock.setPicture(m_services.pictures.find(m_locked ? "Btn_MsgLock_2" : "Btn_MsgLock_1").value_or(gfx::PictureId{}));
        lock.setHint(m_locked ? "Show Dialogue Located : Locked" : "Show Dialogue Located : Unlocked");
    }

    std::optional<std::size_t> ChatWindow::lineAt(core::Point local) const {
        const int rowCount = rows();
        const std::size_t count = m_log.lineCount();
        const std::size_t end = count - std::min(m_scrolledBack, count);
        const std::size_t shown = std::min<std::size_t>(end, static_cast<std::size_t>(rowCount));
        const int row = local.y / m_rowSpot.pitch().y - (rowCount - static_cast<int>(shown));
        if (local.y < 0 || row < 0 || row >= static_cast<int>(shown)) {
            return std::nullopt;
        }
        return end - shown + static_cast<std::size_t>(row);
    }

    void ChatWindow::apply(const TalkSettings& settings) {
        m_form.setBounds(settings.form);
        m_resize.setPosition(settings.resizeButton);
        m_background = settings.background;
        applyResize();
        setLook(settings.formMode);
    }

    TalkSettings ChatWindow::settings() const {
        TalkSettings settings;
        settings.formMode = m_look;
        settings.form = m_form.bounds();
        settings.background = m_background;
        settings.resizeButton = {m_resize.bounds().x, m_resize.bounds().y};
        return settings;
    }

    void ChatWindow::syncScrollBar() {
        const std::size_t count = m_log.lineCount();
        const std::size_t shown = static_cast<std::size_t>(rows());
        m_scrollBar.setRange(static_cast<int>(count), static_cast<int>(shown));
        m_scrollBar.setPosition(static_cast<int>(count > shown + m_scrolledBack ? count - shown - m_scrolledBack : 0));
    }

    void ChatWindow::scroll(int lines) {
        const std::size_t count = m_log.lineCount();
        const std::size_t shown = static_cast<std::size_t>(rows());
        const long long most = count > shown ? static_cast<long long>(count - shown) : 0;
        m_scrolledBack = static_cast<std::size_t>(std::clamp(static_cast<long long>(m_scrolledBack) + lines, 0LL, most));
        syncScrollBar();
    }

    void ChatWindow::paintBackground(gfx::Canvas& canvas, core::Point at) {
        const core::Rect form = m_form.bounds();
        if (m_look == 0) {
            const auto grid = m_services.pictures.find("32X32Grid");
            if (!grid) {
                return;
            }
            const gfx::Picture& picture = m_services.pictures.picture(*grid);
            const core::Rect
                area{at.x + 25, at.y + 8, form.width - 25 - 15, form.height - 8 - 1}; // grid area inside the frame: 25 px left, 8 px top, 15 px right margin
            for (int y = 0; y < area.height; y += picture.height()) {
                for (int x = 0; x < area.width; x += picture.width()) {
                    canvas.drawPicture(
                        *grid,
                        {area.x + x, area.y + y},
                        core::Rect{0, 0, std::min(picture.width(), area.width - x), std::min(picture.height(), area.height - y)}
                    );
                }
            }
        } else if (m_look == 1) {
            const auto strip = m_services.pictures.find("Panel_TalkForm2_1");
            if (!strip) {
                return;
            }
            canvas.drawPicture(*strip, at, core::Rect{0, 0, kStripWidth, kStripTop});
            const int middleEnd = form.height - kStripBottom;
            for (int y = kStripTop; y < middleEnd; y += kStripMiddle) {
                canvas.drawPicture(*strip, {at.x, at.y + y}, core::Rect{0, kStripTop, kStripWidth, std::min(kStripMiddle, middleEnd - y)});
            }
            canvas.drawPicture(*strip, {at.x, at.y + middleEnd}, core::Rect{0, kStripTop + kStripMiddle, kStripWidth, kStripBottom});
        }
    }

    void ChatWindow::paintLines(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        const int iconFrame = static_cast<int>(nowMs / kIconFrameMs % 2); // emote icons have two frames
        m_hoveredHead = 0;
        const int rowCount = rows();
        const std::size_t count = m_log.lineCount();
        const std::size_t end = count - std::min(m_scrolledBack, count);
        const std::size_t shown = std::min<std::size_t>(end, static_cast<std::size_t>(rowCount));
        const int width = m_lines.bounds().width;
        for (std::size_t row = 0; row < shown; ++row) {
            const ChatLine& line = m_log.line(end - shown + row);
            const int y = at.y + (rowCount - static_cast<int>(shown) + static_cast<int>(row)) * m_rowSpot.pitch().y;
            canvas.drawText({at.x, y}, line.text, {line.color, gfx::TextEffect::Shadow, std::nullopt}, width, m_rowSpot.pitch().y);
            for (std::size_t column = 0; column < line.emotes.size();) {
                const int emote = line.emotes[column] - 32; // emote codes start at ASCII 32 (a space means none)
                if (line.emotes[column] == ' ' || emote < 1 || emote > kEmoteCount) {
                    ++column;
                    continue;
                }
                if (const auto icon = m_services.pictures.find("icon_expre_" + std::to_string(emote))) {
                    // emote icon sits 2 px above the row
                    canvas
                        .drawPicture(*icon, {at.x + static_cast<int>(column) * kCharWidth, y - 2}, core::Rect{0, iconFrame * kIconSize, kIconSize, kIconSize});
                }
                column += emoteCode(emote).size();
            }
            // channels 1 .. 7 are player channels; the head is drawn 10 px left of the area and 10 px below the row top
            if (line.channel >= 1 && line.channel <= 7 && line.senderId != 0 && !line.text.empty() && line.text[0] == '(' && m_lookOf) {
                if (const auto look = m_lookOf(line.senderId)) {
                    const core::Point head{at.x - 10, y + 10};
                    m_services.characters.drawChatHead(canvas, *look, head);
                    if (m_services.characters.chatHeadContains(*look, head, m_screen.mouseIn(m_panel))) {
                        m_hoveredHead = line.senderId;
                    }
                }
            }
        }
    }

    void ChatWindow::paintResizeButton(gfx::Canvas& canvas, core::Point at) {
        const auto picture = m_services.pictures.find("Btn_Resize_1");
        if (!picture) {
            return;
        }
        const int size = m_resize.bounds().width;
        const int frame = m_resizing ? 2 : m_screen.hoveredId() == m_resize.id() ? 1 : 0; // button frame: 2 pressed, 1 hovered, 0 normal
        const int frames = std::max(1, m_services.pictures.picture(*picture).height() / size);
        canvas.drawPicture(*picture, at, core::Rect{0, std::min(frame, frames - 1) * size, size, size});
    }

    void ChatWindow::paintResizePreview(gfx::Canvas& canvas, core::Point at) {
        if (!m_resizing) {
            return;
        }
        const core::Rect form = m_form.bounds().translated(at);
        const core::Rect button = m_resize.bounds();
        const int left = form.x;
        // outline top is 3 px below the button's top; right edge is 18 px (the button's width) right of its left
        const int top = form.y + button.y + 3;
        const int right = form.x + button.x + 18;
        const int bottom = form.y + form.height;
        if (right <= left || bottom <= top) {
            return;
        }
        dashDot(canvas, {left, top}, right - left, true, m_previewSpot.color());
        dashDot(canvas, {left, bottom - 1}, right - left, true, m_previewSpot.color());
        dashDot(canvas, {left, top}, bottom - top, false, m_previewSpot.color());
        dashDot(canvas, {right - 1, top}, bottom - top, false, m_previewSpot.color());
    }

}
