#include "client/viewmodels/bank/PropertyWindow.h"

#include "client/rendering/painters/ScaledItemIcon.h"
#include "core/ByteReader.h"
#include "core/ByteWriter.h"
#include "gfx/TextStyle.h"
#include "ui/DrawingArea.h"

#include <algorithm>
#include <utility>

namespace client {

    namespace {

        constexpr int kMostRecords = 1000; // most lost property records read from the server
        constexpr int kPointsEach = 100; // points charged per reclaimed item
        constexpr int kShortMs = 1000; // message display time, ms

        /// @brief Loads PropertyWindow.xml and returns its root window.
        /// @param services Game services (layout and load context).
        /// @param screen Screen to load into.
        /// @return The "Property" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("PropertyWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("Property");
        }

    }

    PropertyWindow::PropertyWindow(
        GameServices& services,
        ui::Screen& screen,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_list(screen.get<ui::BlockList>("PropertyList")),
          m_scroll(screen.get<ui::ScrollBar>("PropertyScroll")),
          m_iconSpot(screen.get<ui::Spot>("PropertyRowIcon")),
          m_nameSpot(screen.get<ui::Spot>("PropertyRowName")),
          m_costSpot(screen.get<ui::Spot>("PropertyCost")),
          m_markSpot(screen.get<ui::Spot>("PropertyMark")) {
        m_marked.fill(-1);
        screen.get<ui::DrawingArea>("PropertyPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onPropertyClose", [this](const ui::EventArgs&) { close(); });
        screen.on("onPropertyOk", [this](const ui::EventArgs&) { confirm(); });
        screen.on("onPropertyUp", [this](const ui::EventArgs&) { scrollTo(m_list.topIndex() - 1); });
        screen.on("onPropertyDown", [this](const ui::EventArgs&) { scrollTo(m_list.topIndex() + 1); });
        screen.on("onPropertyScrolled", [this](const ui::EventArgs& args) { scrollTo(args.index); });
        screen.on("onPropertyPick", [this](const ui::EventArgs& args) { mark(args.index); });
        screen.on("onPropertyUnpick", [this](const ui::EventArgs& args) { unmark(args.index); });
    }

    void PropertyWindow::received(const core::Bytes& payload) {
        core::ByteReader reader(payload, 1);
        switch (reader.u8()) {
            case 1: // 86/1 server: lost property list
                list(payload);
                break;
            // 86/2 server: reclaim done (u16 item)
            case 2: {
                const int itemId = reader.u16();
                if (!reader.failed()) {
                    const auto item = m_services.items.find(itemId);
                    m_hooks.chatLine(
                        "Reclaim lost property " + (item ? item->name : std::string()) + " Points deducted successfully:" + std::to_string(kPointsEach)
                    );
                }
                break;
            }
            // 86/3 server: reclaim error (u8 code)
            case 3: {
                const int code = reader.u8();
                if (code == 1) { // error code 1: bag space not enough
                    m_messages.show("Item bag space is not enough", kShortMs);
                } else if (code == 2) { // error code 2: nothing can be redeemed
                    m_messages.show("At present , nothing can be redeemed.", kShortMs);
                }
                break;
            }
            default: // other 86 sub ids: ignored
                break;
        }
    }

    void PropertyWindow::close() {
        m_window.setVisible(false);
        m_records.clear();
        m_values.clear();
        m_marked.fill(-1);
        m_list.clear();
    }

    void PropertyWindow::list(const core::Bytes& payload) {
        close();
        core::ByteReader reader(payload, 2);
        const int count = reader.u16();
        for (int index = 0; index < count && static_cast<int>(m_records.size()) < kMostRecords; ++index) {
            Record record;
            record.itemId = reader.u16();
            reader.u8();
            reader.u16();
            record.fusion = reader.u8();
            reader.u8();
            record.pose = reader.u16();
            const std::int32_t value = reader.i32();
            if (reader.failed()) {
                break;
            }
            m_records.push_back(record);
            m_values.push_back(value);
            const auto item = m_services.items.find(record.itemId);
            if (!item) {
                continue;
            }
            const int iconItem = record.pose != 0 && m_services.items.find(record.pose) ? record.pose : record.itemId;
            const ScaledItemIcon icon = scaledItemIcon(m_services, iconItem, m_iconSpot.size());
            ui::BlockList::Row row;
            ui::BlockList::PictureBlock picture;
            picture.picture = icon.picture;
            picture.size = icon.size;
            picture.area = m_iconSpot.size();
            picture.horizontal = ui::BlockList::Align::Centre;
            picture.vertical = ui::BlockList::Align::Centre;
            picture.source = icon.source;
            row.pictures.push_back(picture);
            ui::BlockList::TextBlock name;
            name.text = record.fusion != 0 ? item->name + "+" + std::to_string(record.fusion) : item->name;
            name.offset = m_nameSpot.at();
            name.area = m_nameSpot.size();
            name.vertical = ui::BlockList::Align::Centre;
            name.color = m_nameSpot.color();
            row.texts.push_back(name);
            m_list.addRow(std::move(row));
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
        scrollTo(0);
    }

    void PropertyWindow::scrollTo(int top) {
        const int rows = m_list.rowCount();
        const int shown = m_list.visibleRows();
        m_list.setTopIndex(std::clamp(top, 0, std::max(0, rows - shown)));
        m_scroll.setRange(rows, shown);
        m_scroll.setPosition(m_list.topIndex());
    }

    int PropertyWindow::markedCount() const {
        return static_cast<int>(std::count_if(m_marked.begin(), m_marked.end(), [](int row) { return row != -1; }));
    }

    void PropertyWindow::mark(int row) {
        if (row < 0 || row >= m_list.rowCount() || std::find(m_marked.begin(), m_marked.end(), row) != m_marked.end()) {
            return;
        }
        const auto free = std::find(m_marked.begin(), m_marked.end(), -1);
        if (free != m_marked.end()) {
            *free = row;
        }
    }

    void PropertyWindow::unmark(int row) {
        const auto found = std::find(m_marked.begin(), m_marked.end(), row);
        if (found != m_marked.end()) {
            *found = -1;
        }
    }

    void PropertyWindow::paint(gfx::Canvas& canvas, core::Point at) const {
        const std::string cost = std::to_string(markedCount() * kPointsEach);
        const int length = static_cast<int>(cost.size());
        // 5 = half a digit's width to centre the text; 8 = digit width; 12 = text height
        const core::Point costAt = at + m_costSpot.at();
        canvas.drawText({costAt.x - length * 5, costAt.y}, cost, {m_costSpot.color(), gfx::TextEffect::Plain, std::nullopt}, length * 8, 12);
        const int top = m_list.topIndex();
        const core::Rect& mark = m_markSpot.rect();
        for (const int row : m_marked) {
            if (row != -1 && row >= top && row <= top + m_markSpot.rows()) {
                canvas.fillRectAlpha({at.x + mark.x, at.y + mark.y + (row - top - 1) * mark.height, mark.width, mark.height}, m_markSpot.color());
            }
        }
    }

    void PropertyWindow::confirm() {
        const int count = markedCount();
        const int cost = count * kPointsEach;
        if (cost == 0) {
            return;
        }
        if (m_hooks.points() - cost < 0) {
            m_messages.show("Not enough points", kShortMs);
            return;
        }
        m_hooks.ask(
            "Reclaim lost property #R#B" + std::to_string(count) + "/#R/#B Piece, points are needed #R#B" + std::to_string(cost) +
                "/#R/#B Points, are you sure?",
            [this] { send(); }
        );
    }

    void PropertyWindow::send() {
        core::ByteWriter writer;
        writer.u8(86).u8(2).u8(static_cast<std::uint8_t>(markedCount())); // action 86, sub 2 (reclaim), count of marked rows
        for (const int row : m_marked) {
            if (row != -1) {
                const std::int32_t value = row < static_cast<int>(m_values.size()) ? m_values[static_cast<std::size_t>(row)] : 0;
                writer.u16(static_cast<std::uint16_t>(value & 0xFFFF)); // low 16 bits of the value
            }
        }
        m_hooks.send(writer.take());
        close();
    }

}
