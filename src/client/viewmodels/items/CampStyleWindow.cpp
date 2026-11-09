#include "client/viewmodels/items/CampStyleWindow.h"

#include "client/rendering/world/CampSprite.h"
#include "client/models/world/Camps.h"
#include "protocol/TentRequests.h"
#include "ui/DrawingArea.h"

namespace client {

    namespace {

        constexpr int kPaletteStyles = 11; // styles in set 1 (Tent Palette)
        constexpr int kClayStyles = 18; // styles in set 2 (Shape Clay)
        constexpr int kScreenWidth = 800; // game window width, px
        constexpr int kFirstClayStyle = 9071; // first camp sprite id (bank 009) drawn from bank 009_c01
        constexpr int kLastClayStyle = 9093; // last camp sprite id of that range
        constexpr int kPlainStyleSprite = 9001; // camp sprite 9001 (bank 009): the plain tent, style 1

        /// @brief Loads CampStyleWindow.xml and returns its window.
        /// @param services Game services (layout, load context).
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("CampStyleWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("CampStyleWindow");
        }

    }

    CampStyleWindow::CampStyleWindow(
        GameServices& services,
        ui::Screen& screen,
        BagMarks& marks,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_marks(marks),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_tentSpot(screen.get<ui::Spot>("CampStyleTentFeet")) {
        screen.get<ui::DrawingArea>("CampStylePage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onCampStyleNext", [this](const ui::EventArgs&) { step(1); });
        screen.on("onCampStylePrevious", [this](const ui::EventArgs&) { step(-1); });
        screen.on("onCampStyleOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onCampStyleClose", [this](const ui::EventArgs&) { close(); });
    }

    int CampStyleWindow::styleCount() const { return m_set == 2 ? kClayStyles : kPaletteStyles; }

    void CampStyleWindow::request(int bagSlot, int set) {
        if (m_slot >= 1 && m_slot <= BagMarks::kSlots) {
            m_marks.setUseMark(m_slot, 0);
            m_hooks.marksChanged();
        }
        m_slot = bagSlot;
        m_set = set;
        m_hooks.send(protocol::tentPaletteRequest(bagSlot, set));
    }

    void CampStyleWindow::open(int bagSlot, int set) {
        m_slot = bagSlot;
        m_set = set;
        if (m_slot >= 1 && m_slot <= BagMarks::kSlots) {
            m_marks.setUseMark(m_slot, BagMarks::kCampStyle);
            m_hooks.marksChanged();
        }
        const core::Rect items = m_hooks.itemWindowBounds();
        const core::Rect area = m_window.bounds();
        int left = items.x - area.width < 0 ? items.x + items.width : items.x - area.width;
        if (left + area.width >= kScreenWidth) {
            const int spaceRight = kScreenWidth - left;
            const int shift = area.width - spaceRight + 20; // extra pixel margin from the screen edge
            left -= shift;
            m_hooks.moveItemWindow(items.x - shift);
        }
        m_window.setBounds({left, items.y, area.width, area.height});
        m_index = 1;
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void CampStyleWindow::close() {
        reset();
        m_window.setVisible(false);
    }

    void CampStyleWindow::reset() {
        m_index = 1;
        // style sets: 1 = Tent Palette, 2 = Shape Clay
        if ((m_set == 1 || m_set == 2) && m_slot >= 1 && m_slot <= BagMarks::kSlots && m_marks.useMark(m_slot) == BagMarks::kCampStyle) {
            m_marks.setUseMark(m_slot, 0);
            m_hooks.marksChanged();
        }
    }

    void CampStyleWindow::step(int direction) {
        if (m_set != 1 && m_set != 2) {
            return;
        }
        m_index += direction;
        if (m_index < 1 || m_index > styleCount()) {
            m_index = direction > 0 ? 1 : styleCount();
        }
    }

    void CampStyleWindow::confirm() {
        if (m_slot < 1 || m_slot > BagMarks::kSlots || (m_set != 1 && m_set != 2) || m_index < 1 || m_index > styleCount()) {
            return;
        }
        m_hooks.send(protocol::tentStyleRequest(m_index, m_slot, m_set));
        if (m_marks.useMark(m_slot) == BagMarks::kCampStyle) {
            m_marks.setUseMark(m_slot, 0);
            m_hooks.marksChanged();
        }
        m_index = 1;
        m_window.setVisible(false);
    }

    void CampStyleWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const int style = m_index == 1 ? kPlainStyleSprite : Camps::kindId(m_index, m_set);
        CampSprite sprite{"009", style}; // sprite bank "009": the camp / tent sprites
        if (m_index != 1 && style >= kFirstClayStyle && style <= kLastClayStyle) {
            sprite.bank = "009_c01";
        }
        m_services.characters.drawCampSprite(canvas, sprite.bank, sprite.id, {at.x + m_tentSpot.at().x, at.y + m_tentSpot.at().y});
    }

}
