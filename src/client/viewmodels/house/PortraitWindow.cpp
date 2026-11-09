#include "client/viewmodels/house/PortraitWindow.h"

#include "client/data/RoleOrder.h"
#include "core/ByteReader.h"
#include "protocol/HouseRequests.h"
#include "ui/DrawingArea.h"

#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kPictureFrame = 39153; // item 39153: Picture Frame
        constexpr int kGoodPictureFrame = 39154; // item 39154: Good Picture Frame
        constexpr int kFigureY = 104; // y of the figures' anchor in the window, px
        constexpr int kFirstBust = 9055; // bust sprite id the head and body type offsets are added to
        constexpr int kLeftOffset = 83; // x of the left figure's anchor in the window, px
        constexpr int kRightOffset = 143; // x of the right figure's anchor in the window, px

        /// @brief Loads PortraitWindow.xml and returns its window.
        /// @param services Game services providing the layout.
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PortraitWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("PortraitWindow");
        }

    }

    PortraitWindow::PortraitWindow(
        GameServices& services,
        ui::Screen& screen,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)) {
        screen.on("onPortraitClose", [this](const ui::EventArgs&) { m_window.setVisible(false); });
        screen.get<ui::DrawingArea>("PortraitPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        clear();
    }

    void PortraitWindow::clear() {
        m_left = Figure{};
        m_right = Figure{};
    }

    void PortraitWindow::setFrame(int frame) {
        m_frame = frame;
        m_window.setPicture("Portrait" + std::to_string(frame) + "a", m_services.pictures);
    }

    bool PortraitWindow::place(int bodyType, const CharacterLook& look) {
        // body types 1 and 3 are the left (man) figure, 2 and 4 the right (woman) one
        if (bodyType != 1 && bodyType != 2 && bodyType != 3 && bodyType != 4) {
            return false;
        }
        Figure& figure = bodyType == 1 || bodyType == 3 ? m_left : m_right;
        figure.look = look;
        figure.shown = true;
        return true;
    }

    void PortraitWindow::loadOwner() {
        clear();
        const int item = m_hooks.pieceItem(m_furnIndex);
        if (item == kPictureFrame) {
            setFrame(3); // frame 3: Picture Frame with the owner alone
        } else if (item == kGoodPictureFrame) {
            setFrame(4); // frame 4: Good Picture Frame with the owner alone
        } else {
            return;
        }
        const std::int32_t owner = m_hooks.houseOwnerId();
        if (owner == m_hooks.ownId()) {
            const CharacterLook look = m_hooks.ownLook();
            place(look.bodyType, look);
        } else if (const auto look = m_hooks.worldLook(owner)) {
            place(look->bodyType, *look);
        }
    }

    void PortraitWindow::clicked(int furnIndex, int itemId) {
        if (m_window.isVisible()) {
            if (m_hooks.pieceItem(m_furnIndex) == itemId) {
                return;
            }
            m_furnIndex = furnIndex;
        } else {
            m_furnIndex = furnIndex;
            loadOwner();
        }
        m_hooks.send(protocol::openPortraitRequest(furnIndex));
    }

    void PortraitWindow::receive(const core::Bytes& payload) {
        core::ByteReader reader(payload, 2); // payload offset where the sub id is read
        const int sub = reader.u8();
        const bool good = m_hooks.pieceItem(m_furnIndex) == kGoodPictureFrame;
        if (sub == 0) { // sub 0: show the form as it is
            // frames 4 / 3: Good Picture Frame / Picture Frame with the owner alone
            setFrame(good ? 4 : 3);
        } else if (sub == 1) { // sub 1: add the second person
            const int bodyType = reader.u8();
            const int head = reader.u8();
            const std::int32_t code1 = reader.i32();
            const std::int32_t code5 = reader.i32();
            if (reader.failed()) {
                return;
            }
            setFrame(good ? 2 : 1); // frames 2 / 1: Good Picture Frame / Picture Frame with two people
            if (!place(bodyType, CharacterLook::fromColorCodes(bodyType, head, code1, code5, m_services.items))) {
                return;
            }
            int offset = 0;
            switch (roleOrder(m_left.shown ? m_left.look.bodyType : 0, m_left.shown ? m_left.look.head : 0)) {
                case 1: offset = -6; break; // role order 1: shift left 6 px
                case 4: offset = -2; break; // role order 4: shift left 2 px
                case 14: offset = 2; break; // role order 14: shift right 2 px
                default: break; // other roles: no shift
            }
            m_left.offsetX = offset + kLeftOffset;
            offset = 0;
            switch (roleOrder(m_right.shown ? m_right.look.bodyType : 0, m_right.shown ? m_right.look.head : 0)) {
                case 3: // role order 3: shift right 2 px (shared with 7 and 8)
                case 7: // role order 7: shift right 2 px (shared with 3 and 8)
                case 8: offset = 2; break; // role order 8: shift right 2 px
                case 10: // role order 10: shift right 4 px (shared with 11 to 13)
                case 11: // role order 11: shift right 4 px
                case 12: // role order 12: shift right 4 px
                case 13: offset = 4; break; // role order 13: shift right 4 px
                case 15: offset = 8; break; // role order 15: shift right 8 px
                default: break; // other roles: no shift
            }
            m_right.offsetX = offset + kRightOffset;
        } else {
            return;
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void PortraitWindow::update() {
        if (m_window.isVisible() && !m_hooks.inFloorRoom()) {
            m_window.setVisible(false);
        }
    }

    void PortraitWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (m_right.shown) {
            // bust sprite offsets from the first bust: body type 2 adds 2, the other (4) adds 8
            const int bust = kFirstBust + (m_right.look.bodyType == 2 ? m_right.look.head + 2 : m_right.look.head + 8);
            m_services.characters.drawBustSprite(canvas, m_right.look, bust, {at.x + m_right.offsetX, at.y + kFigureY});
        }
        if (m_left.shown) {
            // bust sprite offsets from the first bust: body type 1 adds 1, the other (3) adds 4
            const int bust = kFirstBust + (m_left.look.bodyType == 1 ? m_left.look.head + 1 : m_left.look.head + 4);
            m_services.characters.drawBustSprite(canvas, m_left.look, bust, {at.x + m_left.offsetX, at.y + kFigureY});
        }
        if (m_frame >= 1 && m_frame <= 4) { // valid frame numbers
            if (const auto picture = m_services.pictures.find("Portrait" + std::to_string(m_frame))) {
                canvas.drawPicture(*picture, at);
            }
        }
    }

}
