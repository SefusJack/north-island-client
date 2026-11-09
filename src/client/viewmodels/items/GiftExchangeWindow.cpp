#include "client/viewmodels/items/GiftExchangeWindow.h"

#include "client/models/items/GiftCardFile.h"
#include "core/ByteReader.h"
#include "gfx/TextRenderer.h"
#include "gfx/TextStyle.h"
#include "protocol/GiftCardRequests.h"

#include <algorithm>
#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr int kMessageMs = 2000; // how long a player message stays up, ms
        constexpr int kBagSlots = Inventory::kBagSlots;
        constexpr int kSlashHalf = 4; // half the width of the "/" between page and page count, px

        /// @brief Loads GiftExchangeWindow.xml and returns its window.
        /// @param services Game services (layout, load context).
        /// @param screen Screen to load into.
        /// @return The loaded window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("GiftExchangeWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("GiftExchange");
        }

        /// @brief Builds a plain text style of one colour.
        /// @param color Text colour.
        /// @return The style.
        gfx::TextStyle plain(const core::Color& color) { return gfx::TextStyle{color, gfx::TextEffect::Plain, std::nullopt}; }

    }

    GiftExchangeWindow::GiftExchangeWindow(
        GameServices& services,
        ui::Screen& screen,
        const Inventory& inventory,
        const BagMarks& marks,
        PlayerMessages& messages,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_inventory(inventory),
          m_marks(marks),
          m_messages(messages),
          m_hooks(std::move(hooks)),
          m_window(loadWindow(services, screen)),
          m_times(screen.get<ui::TextBox>("GiftTimes")),
          m_checks{
              screen.get<ui::ImageButton>("GiftCheck1"),
              screen.get<ui::ImageButton>("GiftCheck2"),
              screen.get<ui::ImageButton>("GiftCheck3"),
              screen.get<ui::ImageButton>("GiftCheck4")
          },
          m_items{
              screen.get<ui::DrawingArea>("GiftItem1"),
              screen.get<ui::DrawingArea>("GiftItem2"),
              screen.get<ui::DrawingArea>("GiftItem3"),
              screen.get<ui::DrawingArea>("GiftItem4")
          },
          m_nameSpot(screen.get<ui::Spot>("GiftName")),
          m_pageSpot(screen.get<ui::Spot>("GiftPage")),
          m_quantitySpot(screen.get<ui::Spot>("GiftQuantity")),
          m_markSpot(screen.get<ui::Spot>("GiftMark")),
          m_rowNameSpot(screen.get<ui::Spot>("GiftRowName")) {
        m_checkedPicture = services.pictures.find("btn_Check_1").value_or(gfx::PictureId{});
        m_uncheckedPicture = services.pictures.find("btn_UnCheck_1").value_or(gfx::PictureId{});
        screen.get<ui::DrawingArea>("GiftPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        for (int row = 0; row < kPerPage; ++row) {
            m_items[static_cast<std::size_t>(row)].get().setPainter([this, row](gfx::Canvas& canvas, core::Point at, std::uint64_t) {
                paintItem(canvas, at, row);
            });
            screen.on("onGiftCheck" + std::to_string(row + 1), [this, row](const ui::EventArgs&) { check(row + 1); });
        }
        screen.on("onGiftExchange", [this](const ui::EventArgs&) { exchange(); });
        screen.on("onGiftPrevious", [this](const ui::EventArgs&) { prevPage(); });
        screen.on("onGiftNext", [this](const ui::EventArgs&) { nextPage(); });
        screen.on("onGiftClose", [this](const ui::EventArgs&) { hide(); });
        m_cards = GiftCardFile::read(services.game.data("Data/Gec.Dat"));
        m_fresh.assign(m_cards.size(), false);
        reset(false);
    }

    void GiftExchangeWindow::reset(bool clearFlags) {
        m_current = GiftCard{};
        m_bagSlot = 0;
        m_page = 1;
        m_selected = 0;
        m_times.setText("1");
        if (clearFlags) {
            std::fill(m_fresh.begin(), m_fresh.end(), false);
        }
    }

    int GiftExchangeWindow::indexOf(int cardId) const {
        for (std::size_t index = 0; index < m_cards.size(); ++index) {
            if (m_cards[index].id == cardId) {
                return static_cast<int>(index);
            }
        }
        return -1;
    }

    int GiftExchangeWindow::pageCount() const { return (static_cast<int>(m_current.choices.size()) + kPerPage - 1) / kPerPage; }

    std::string GiftExchangeWindow::itemName(int itemId) const {
        const auto record = m_services.items.find(itemId);
        return record ? record->name : std::string();
    }

    void GiftExchangeWindow::use(int bagSlot) {
        reset(false);
        const int cardId = m_inventory.bagItem(bagSlot).itemId;
        if (cardId == 0) {
            return;
        }
        const int index = indexOf(cardId);
        if (index >= 0) {
            m_current = m_cards[static_cast<std::size_t>(index)];
            m_bagSlot = bagSlot;
            if (m_fresh[static_cast<std::size_t>(index)]) {
                show();
                return;
            }
        }
        const int version = index >= 0 ? m_cards[static_cast<std::size_t>(index)].version : 0;
        m_services.connection.send(protocol::giftCardUseRequest(bagSlot, cardId, version));
    }

    void GiftExchangeWindow::open(int bagSlot) {
        reset(false);
        const int cardId = m_inventory.bagItem(bagSlot).itemId;
        if (cardId == 0) {
            return;
        }
        const int index = indexOf(cardId);
        if (index < 0) {
            return;
        }
        m_current = m_cards[static_cast<std::size_t>(index)];
        m_bagSlot = bagSlot;
        m_fresh[static_cast<std::size_t>(index)] = true;
        show();
    }

    void GiftExchangeWindow::show() {
        if (m_hooks.fighting()) {
            m_messages.show("Can't be used during the battle", kMessageMs);
            hide();
        } else if (m_bagSlot == 0 || m_current.id == 0) {
            hide();
        } else {
            refreshPage();
            m_window.setVisible(true);
            if (m_screen.context().bringToFront) {
                m_screen.context().bringToFront(m_window.id());
            }
        }
    }

    void GiftExchangeWindow::refreshPage() {
        const int choices = static_cast<int>(m_current.choices.size());
        for (int row = 0; row < kPerPage; ++row) {
            const bool shown = (m_page - 1) * kPerPage + row < choices;
            ui::ImageButton& tick = m_checks[static_cast<std::size_t>(row)];
            tick.setPicture(m_uncheckedPicture);
            tick.setVisible(shown);
            m_items[static_cast<std::size_t>(row)].get().setVisible(shown);
        }
        if (m_selected > 0 && (m_selected + kPerPage - 1) / kPerPage == m_page) {
            m_checks[static_cast<std::size_t>(m_selected - (m_page - 1) * kPerPage - 1)].get().setPicture(m_checkedPicture);
        }
    }

    void GiftExchangeWindow::prevPage() {
        if (m_page > 1) {
            --m_page;
            refreshPage();
        }
    }

    void GiftExchangeWindow::nextPage() {
        if (m_page < pageCount()) {
            ++m_page;
            refreshPage();
        } else {
            m_messages.show("Last Page", kMessageMs);
        }
    }

    void GiftExchangeWindow::check(int tag) {
        if (tag == 0 || tag > kPerPage) {
            return;
        }
        const int selected = m_selected;
        if (selected > 0 && (selected + kPerPage - 1) / kPerPage == m_page) {
            m_checks[static_cast<std::size_t>(selected - (m_page - 1) * kPerPage - 1)].get().setPicture(m_uncheckedPicture);
        }
        const int index = (m_page - 1) * kPerPage + tag;
        if (static_cast<int>(m_current.choices.size()) < index) {
            return;
        }
        if (selected != index) {
            m_checks[static_cast<std::size_t>(tag - 1)].get().setPicture(m_checkedPicture);
            m_selected = index;
        } else {
            m_checks[static_cast<std::size_t>(tag - 1)].get().setPicture(m_uncheckedPicture);
            m_selected = 0;
        }
    }

    int GiftExchangeWindow::times() const {
        const std::string& text = m_times.text();
        return text.empty() ? 0 : std::stoi(text);
    }

    void GiftExchangeWindow::exchange() {
        if (m_selected <= 0 || m_selected > static_cast<int>(m_current.choices.size()) || m_times.text().empty()) {
            return;
        }
        if (m_times.text() == "0") {
            m_messages.show("Use time cannot be 0", kMessageMs);
            return;
        }
        const int count = times();
        const GiftCard::Choice& choice = m_current.choices[static_cast<std::size_t>(m_selected - 1)];
        std::string text = "Exchange [#R" + itemName(choice.itemId) + "/#R]" + (choice.count > 1 ? "X" + std::to_string(choice.count) : std::string());
        if (count > 1) {
            text += "\rExchange " + std::to_string(count) + " Time";
        }
        text += "\rSure? ";
        m_hooks.ask(text, [this] { confirmExchange(); }, [] {});
        hide();
    }

    void GiftExchangeWindow::confirmExchange() {
        const int count = times();
        if (m_selected < 1 || m_selected > static_cast<int>(m_current.choices.size())) {
            return;
        }
        const GiftCard::Choice choice = m_current.choices[static_cast<std::size_t>(m_selected - 1)];
        if (!m_services.items.find(choice.itemId)) {
            return;
        }
        int have = 0;
        for (int slot = 1; slot <= kBagSlots; ++slot) {
            if (m_inventory.bagItem(slot).itemId == m_current.id && m_marks.isAvailable(slot)) {
                have += m_inventory.bagItem(slot).count;
                if (have >= count) {
                    break;
                }
            }
        }
        if (have < count) {
            m_messages.show("Not enough exchanging cards", kMessageMs);
            return;
        }
        const int total = choice.count * count;
        int need = 0;
        if (m_inventory.isStackable(choice.itemId)) {
            need = (total + Inventory::kMaxStack - 1) / Inventory::kMaxStack;
        } else if (const core::Size cells = m_inventory.cellsOf(choice.itemId); cells.width <= 1 && cells.height <= 1) {
            need = total;
        }
        if (need == 0) {
            return;
        }
        for (int slot = 1; slot <= kBagSlots; ++slot) {
            if (m_inventory.bagItem(slot).empty()) {
                if (--need == 0) {
                    break;
                }
            }
        }
        if (need > 0) {
            m_messages.show("Not enough space\rNot enough space" + std::to_string(need) + "slot", kMessageMs);
            return;
        }
        m_services.connection.send(protocol::giftCardExchangeRequest(m_bagSlot, count, m_selected, choice.itemId));
    }

    void GiftExchangeWindow::receive(const core::Bytes& payload) {
        if (payload.size() < 3) { // minimum packet length (header and sub id)
            return;
        }
        switch (payload[2]) { // byte offset of the sub id
            case 1: receiveVersions(payload); break; // 23/108/1: versions of the server's lists
            case 2: receiveLists(payload); break; // 23/108/2: the lists themselves
            case 3: receiveDone(payload); break; // 23/108/3: an exchange was done
            case 4: open(payload.size() > 3 ? payload[3] : 0); break; // 23/108/4: open the form for a bag place (u8 slot at offset 3)
            default: break; // unknown sub id: ignored
        }
    }

    void GiftExchangeWindow::receiveVersions(const core::Bytes& payload) {
        core::ByteReader reader(payload, 3); // start offset after header and sub id
        const int count = reader.u16();
        std::vector<int> state(m_cards.size(), 0);
        std::vector<int> wanted;
        for (int entry = 0; entry < count && !reader.failed(); ++entry) {
            const int cardId = reader.u16();
            const int version = reader.u8();
            int ask = cardId;
            for (std::size_t index = 0; index < m_cards.size(); ++index) {
                if (m_cards[index].id == cardId) {
                    state[index] = 1;
                    if (m_cards[index].version == version) {
                        state[index] = 2;
                        ask = 0;
                        break;
                    }
                }
            }
            wanted.push_back(ask);
        }
        for (std::size_t index = 0; index < m_cards.size(); ++index) {
            if (state[index] == 0) {
                m_cards[index] = GiftCard{};
            }
        }
        std::vector<int> asked;
        for (const int card : wanted) {
            if (card != 0) {
                asked.push_back(card);
            }
        }
        if (!asked.empty()) {
            m_services.connection.send(protocol::giftCardListRequest(asked));
        }
    }

    void GiftExchangeWindow::receiveLists(const core::Bytes& payload) {
        core::ByteReader reader(payload, 3); // start offset after header and sub id
        const int count = reader.u16();
        for (int entry = 0; entry < count && !reader.failed(); ++entry) {
            GiftCard card;
            card.id = reader.u16();
            card.version = reader.u8();
            const int choices = reader.u8();
            for (int choice = 0; choice < choices && !reader.failed(); ++choice) {
                GiftCard::Choice item;
                item.itemId = reader.u16();
                item.count = reader.u8();
                card.choices.push_back(item);
            }
            if (reader.failed()) {
                break;
            }
            int place = indexOf(card.id);
            if (place < 0) {
                place = indexOf(0);
            }
            if (place < 0) {
                m_cards.push_back(GiftCard{});
                m_fresh.push_back(false);
                place = static_cast<int>(m_cards.size()) - 1;
            }
            m_cards[static_cast<std::size_t>(place)] = std::move(card);
            m_fresh[static_cast<std::size_t>(place)] = true;
        }
        GiftCardFile::write(m_services.game.data("Data/Gec.Dat"), m_cards);
    }

    void GiftExchangeWindow::receiveDone(const core::Bytes& payload) {
        core::ByteReader reader(payload, 3); // start offset after header and sub id
        const int cardId = reader.u16();
        const int count = reader.u8();
        const int itemId = reader.u16();
        const int amount = reader.u8();
        if (reader.failed()) {
            return;
        }
        m_hooks.chatLine("Use [" + itemName(cardId) + "]X" + std::to_string(count) + " Obtained " + itemName(itemId) + "X" + std::to_string(amount * count));
    }

    void GiftExchangeWindow::update() {
        if (!m_window.isVisible()) {
            return;
        }
        if (m_hooks.fighting() || m_bagSlot < 1 || m_bagSlot > kBagSlots || m_inventory.bagItem(m_bagSlot).itemId == 0 ||
            m_inventory.bagItem(m_bagSlot).itemId != m_current.id) {
            hide();
            return;
        }
        if (m_times.text().empty()) {
            m_times.setText("0");
        }
        if (times() > kMostTimes) {
            m_times.setText(std::to_string(kMostTimes));
        }
    }

    void GiftExchangeWindow::paint(gfx::Canvas& canvas, core::Point at) {
        if (const auto quantity = m_services.pictures.find("Icon_MakeThingInfo_MakeQuant_2")) {
            canvas.drawPicture(*quantity, {at.x + m_quantitySpot.at().x, at.y + m_quantitySpot.at().y});
        }
        if (const auto mark = m_services.pictures.find("Icon_ExchangeMark_1")) {
            canvas.drawPicture(*mark, {at.x + m_markSpot.at().x, at.y + m_markSpot.at().y});
        }
        const std::string name = itemName(m_current.id);
        if (!name.empty()) {
            const int width = gfx::TextRenderer::textWidth(name, gfx::TextEffect::Plain);
            canvas.drawText({at.x + m_nameSpot.at().x - width / 2, at.y + m_nameSpot.at().y}, name, plain(m_nameSpot.color()), width);
        }
        int page = m_page;
        int pages = pageCount();
        if (pages == 0) {
            page = 1;
            pages = 1;
        }
        const std::string current = std::to_string(page) + " ";
        const std::string last = "/ " + std::to_string(pages);
        const int currentWidth = gfx::TextRenderer::textWidth(current, gfx::TextEffect::Plain);
        canvas.drawText({at.x + m_pageSpot.at().x - kSlashHalf - currentWidth, at.y + m_pageSpot.at().y}, current, plain(m_pageSpot.color()), currentWidth);
        canvas.drawText(
            {at.x + m_pageSpot.at().x - kSlashHalf, at.y + m_pageSpot.at().y},
            last,
            plain(m_pageSpot.color()),
            gfx::TextRenderer::textWidth(last, gfx::TextEffect::Plain)
        );
        for (int row = 0; row < kPerPage; ++row) {
            const std::size_t index = static_cast<std::size_t>((m_page - 1) * kPerPage + row);
            if (index >= m_current.choices.size()) {
                break;
            }
            const GiftCard::Choice& choice = m_current.choices[index];
            std::string text = itemName(choice.itemId);
            if (choice.count > 1) {
                text += " (X" + std::to_string(choice.count) + ")";
            }
            canvas.drawText(
                {at.x + m_rowNameSpot.at().x, at.y + m_rowNameSpot.at().y + row * m_rowNameSpot.pitch().y},
                text,
                plain(m_rowNameSpot.color()),
                gfx::TextRenderer::textWidth(text, gfx::TextEffect::Plain)
            );
        }
    }

    void GiftExchangeWindow::paintItem(gfx::Canvas& canvas, core::Point at, int row) {
        const std::size_t index = static_cast<std::size_t>((m_page - 1) * kPerPage + row);
        if (index >= m_current.choices.size()) {
            return;
        }
        const auto record = m_services.items.find(m_current.choices[index].itemId);
        if (!record) {
            return;
        }
        if (const auto icon = m_services.pictures.find(std::to_string(record->iconId))) {
            canvas.drawPicture(
                *icon,
                at,
                core::Rect{0, 0, m_items[static_cast<std::size_t>(row)].get().bounds().width, m_items[static_cast<std::size_t>(row)].get().bounds().height}
            );
        }
    }

}
