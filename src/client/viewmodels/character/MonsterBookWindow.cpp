#include "client/viewmodels/character/MonsterBookWindow.h"

#include "client/rendering/hud/InfoDigits.h"
#include "client/rendering/hud/PageNumbers.h"
#include "gfx/TextStyle.h"
#include "ui/DrawingArea.h"
#include "ui/TextWrap.h"

namespace client {

    namespace {

        constexpr int kNameChars = 14; // name wrap width, characters
        constexpr int kStandingSlot = 13; // NPC action slot of the pose shown in the book
        const std::string kRowName = "MonsterBookRow";

        /// @brief Loads MonsterBookWindow.xml and returns its root window.
        /// @param services Game services (layout and load context).
        /// @param screen Screen to load into.
        /// @return The "MonsterBook" window.
        ui::Window& loadWindow(GameServices& services, ui::Screen& screen) {
            screen.load(services.game.layout("MonsterBookWindow.xml"), services.loadContext());
            return screen.get<ui::Window>("MonsterBook");
        }

    }

    MonsterBookWindow::MonsterBookWindow(
        GameServices& services,
        ui::Screen& screen,
        const MonsterBook& book
    )
        : m_services(services),
          m_screen(screen),
          m_book(book),
          m_window(loadWindow(services, screen)),
          m_rows{
              screen.get<ui::TextBox>("MonsterBookRow1"),
              screen.get<ui::TextBox>("MonsterBookRow2"),
              screen.get<ui::TextBox>("MonsterBookRow3"),
              screen.get<ui::TextBox>("MonsterBookRow4"),
              screen.get<ui::TextBox>("MonsterBookRow5"),
              screen.get<ui::TextBox>("MonsterBookRow6"),
              screen.get<ui::TextBox>("MonsterBookRow7"),
              screen.get<ui::TextBox>("MonsterBookRow8"),
              screen.get<ui::TextBox>("MonsterBookRow9"),
              screen.get<ui::TextBox>("MonsterBookRow10")
          },
          m_hoverSpot(screen.get<ui::Spot>("MonsterBookHover")),
          m_pageSpot(screen.get<ui::Spot>("MonsterBookPageNumber")),
          m_nameSpot(screen.get<ui::Spot>("MonsterBookName")),
          m_feetSpot(screen.get<ui::Spot>("MonsterBookFeet")),
          m_elementSpot(screen.get<ui::Spot>("MonsterBookElement")),
          m_barHpSpot(screen.get<ui::Spot>("MonsterBookBarHp")),
          m_numberSpot(screen.get<ui::Spot>("MonsterBookNumber")),
          m_strengthSpot(screen.get<ui::Spot>("MonsterBookStrength")),
          m_constitutionSpot(screen.get<ui::Spot>("MonsterBookConstitution")),
          m_intelligenceSpot(screen.get<ui::Spot>("MonsterBookIntelligence")),
          m_wisdomSpot(screen.get<ui::Spot>("MonsterBookWisdom")),
          m_agilitySpot(screen.get<ui::Spot>("MonsterBookAgility")),
          m_levelSpot(screen.get<ui::Spot>("MonsterBookLevel")) {
        screen.get<ui::DrawingArea>("MonsterBookPage").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t) { paint(canvas, at); });
        screen.on("onMonsterBookRow", [this](const ui::EventArgs& args) { selectRow(std::stoi(args.source.substr(kRowName.size()))); });
        screen.on("onMonsterBookPrevious", [this](const ui::EventArgs&) { turnPage(-1); });
        screen.on("onMonsterBookNext", [this](const ui::EventArgs&) { turnPage(1); });
        screen.on("onMonsterBookClose", [this](const ui::EventArgs&) { close(); });
    }

    void MonsterBookWindow::toggle() {
        if (m_window.isVisible()) {
            close();
            return;
        }
        m_window.setVisible(true);
        if (m_screen.context().bringToFront) {
            m_screen.context().bringToFront(m_window.id());
        }
    }

    void MonsterBookWindow::turnPage(int step) {
        const int page = m_page + step;
        if (page >= 1 && page <= m_book.pageCount()) {
            m_page = page;
        }
    }

    void MonsterBookWindow::selectRow(int row) {
        m_selected = (m_page - 1) * MonsterBook::kPerPage + row;
        if (m_selected > m_book.entryCount()) {
            m_selected = 0;
        }
    }

    void MonsterBookWindow::showNames() {
        for (int row = 1; row <= MonsterBook::kPerPage; ++row) {
            const int number = (m_page - 1) * MonsterBook::kPerPage + row;
            m_rows[static_cast<std::size_t>(row - 1)].get().setText(number <= m_book.entryCount() ? m_book.entry(number).name : "");
        }
    }

    void MonsterBookWindow::paint(gfx::Canvas& canvas, core::Point at) {
        const core::Point mouse = m_screen.mouse();
        for (int row = 0; row < MonsterBook::kPerPage; ++row) {
            const core::Rect bar{
                at.x + m_hoverSpot.rect().x,
                at.y + m_hoverSpot.rect().y + row * m_hoverSpot.pitch().y,
                m_hoverSpot.rect().width,
                m_hoverSpot.rect().height
            };
            if (bar.contains(mouse)) {
                canvas.fillRectAlpha(bar, m_hoverSpot.color());
            }
        }
        showNames();
        drawPageNumbersFrom(canvas, {at.x + m_pageSpot.at().x, at.y + m_pageSpot.at().y}, m_page, m_book.pageCount());
        if (m_selected <= 0 || m_selected > m_book.entryCount()) {
            return;
        }
        const formats::NpcInfo& monster = m_book.entry(m_selected);
        canvas.drawText(
            {at.x + m_nameSpot.at().x, at.y + m_nameSpot.at().y},
            ui::wrapTextLines(monster.name, kNameChars),
            {m_nameSpot.color(), gfx::TextEffect::Plain, std::nullopt},
            m_nameSpot.rect().width,
            m_nameSpot.rect().height
        );
        const int slot = m_services.characters.npcSlot(monster.spriteId, monster.directionKind, kStandingSlot);
        m_services.characters.drawNpc(canvas, monster, {at.x + m_feetSpot.at().x, at.y + m_feetSpot.at().y}, slot, 0);
        if (const auto icon = m_services.pictures.find("Icon_Element_" + std::to_string(monster.element) + "_1")) {
            canvas.drawPicture(*icon, {at.x + m_elementSpot.at().x, at.y + m_elementSpot.at().y});
        }
        const auto number = [&](core::Point where, int value) { drawInfoDigits(canvas, {at.x + where.x, at.y + where.y}, value); };
        number(m_barHpSpot.at(), monster.barHp);
        number(m_numberSpot.at(), monster.collectionNumber);
        number(m_strengthSpot.at(), monster.strength);
        number(m_constitutionSpot.at(), monster.constitution);
        number(m_intelligenceSpot.at(), monster.intelligence);
        number(m_wisdomSpot.at(), monster.wisdom);
        number(m_agilitySpot.at(), monster.agility);
        number(m_levelSpot.at(), monster.level);
    }

}
