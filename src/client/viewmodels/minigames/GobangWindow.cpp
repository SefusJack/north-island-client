#include "client/viewmodels/minigames/GobangWindow.h"

#include "client/rendering/hud/GameNumbers.h"
#include "gfx/TextStyle.h"
#include "protocol/MessageId.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace client {

    namespace {

        constexpr int kFirstSeatSlot = 52; // character action slot used to draw the first seat player
        constexpr int kSecondSeatSlot = 48; // character action slot used to draw the second seat player
        constexpr int kTextLeft = 4; // left padding of editor text, px
        constexpr int kTextHeight = 15; // height of one line of editor text, px
        constexpr std::uint64_t kResultMs = 3000; // delay before the result letters show after the end packet, ms
        constexpr std::uint64_t kSelectErrorMs = 1300; // how long tables stay hidden after a refused choice, ms
        constexpr int kNoticeMs = 1200; // duration of table notices, ms
        constexpr int kMostGold = 9999999; // largest gold a player may hold (ante + gold above this is refused)
        constexpr int kBlinkFrameMs = 500; // time per frame of the blinking last-stone cursor, ms

        /// @brief Draws a read-only editor's text (a TSe_Editor with style 2) in the box of its place.
        /// @param canvas Canvas to draw on.
        /// @param box Box of the editor.
        /// @param text Text to draw (nothing is drawn when empty).
        /// @param color Text colour.
        void drawEditorText(gfx::Canvas& canvas, core::Rect box, const std::string& text, const core::Color& color) {
            if (!text.empty()) {
                canvas.drawText({box.x + kTextLeft, box.y + (box.height - kTextHeight) / 2}, text, {color, gfx::TextEffect::Shadow, std::nullopt}, box.width);
            }
        }

    }

    GobangWindow::GobangWindow(
        GameServices& services,
        ui::Screen& screen,
        Hooks hooks
    )
        : m_services(services),
          m_screen(screen),
          m_hooks(std::move(hooks)),
          m_game([&] {
              GobangGame::Hooks game = m_hooks.game;
              game.backToHall = [this] { sendBackToHall(); };
              return game;
          }()),
          m_ante(services, screen, {m_hooks.game.gold, m_hooks.game.anteError, [this](int bet) { confirmAnte(bet); }}),
          m_page((screen.load(services.game.layout("GobangWindow.xml"), services.loadContext()), screen.get<ui::Element>("GobangPage"))),
          m_form(screen.get<ui::Element>("GobangForm")),
          m_quitButton(screen.get<ui::ImageButton>("GobangQuit")),
          m_backButton(screen.get<ui::ImageButton>("GobangBackToHall")),
          m_tableStepSpot(screen.get<ui::Spot>("GobangTableStep")),
          m_hallTitleSpot(screen.get<ui::Spot>("GobangHallTitle")),
          m_deskSpot(screen.get<ui::Spot>("GobangDesk")),
          m_firstSeatSpot(screen.get<ui::Spot>("GobangFirstSeat")),
          m_secondSeatSpot(screen.get<ui::Spot>("GobangSecondSeat")),
          m_moneySpot(screen.get<ui::Spot>("GobangMoney")),
          m_turnBottomSpot(screen.get<ui::Spot>("GobangTurnBottom")),
          m_turnTopSpot(screen.get<ui::Spot>("GobangTurnTop")),
          m_bottomStoneSpot(screen.get<ui::Spot>("GobangBottomStone")),
          m_topStoneSpot(screen.get<ui::Spot>("GobangTopStone")),
          m_timerBarSpot(screen.get<ui::Spot>("GobangTimerBar")),
          m_secondsBarSpot(screen.get<ui::Spot>("GobangSecondsBar")),
          m_resultSpot(screen.get<ui::Spot>("GobangResult")),
          m_labelSpot(screen.get<ui::Spot>("GobangLabel")),
          m_anteBoxSpot(screen.get<ui::Spot>("GobangAnteBox")),
          m_numberBoxSpot(screen.get<ui::Spot>("GobangNumberBox")),
          m_firstNameSpot(screen.get<ui::Spot>("GobangFirstName")),
          m_secondNameSpot(screen.get<ui::Spot>("GobangSecondName")),
          m_emptyTextSpot(screen.get<ui::Spot>("GobangEmptyText")),
          m_myNameSpot(screen.get<ui::Spot>("GobangMyName")),
          m_opponentNameSpot(screen.get<ui::Spot>("GobangOpponentName")),
          m_statusSpot(screen.get<ui::Spot>("GobangStatus")),
          m_myFaceSpot(screen.get<ui::Spot>("GobangMyFace")),
          m_opponentFaceSpot(screen.get<ui::Spot>("GobangOpponentFace")),
          m_cyanSpot(screen.get<ui::Spot>("GobangCyan")),
          m_yellowSpot(screen.get<ui::Spot>("GobangYellow")),
          m_whiteSpot(screen.get<ui::Spot>("GobangWhite")) {
        for (int number = 1; number <= GobangGame::kTables; ++number) {
            m_tableButtons.emplace_back(screen.get<ui::ImageButton>("GobangTable" + std::to_string(number)));
        }
        screen.get<ui::DrawingArea>("GobangPage").setPainter([this](gfx::Canvas& canvas, core::Point, std::uint64_t nowMs) { paint(canvas, nowMs); });
        screen.get<ui::DrawingArea>("GobangFormPaint").setPainter([this](gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
            paintForm(canvas, at, nowMs);
        });
        screen.on("onGobangDown", [this](const ui::EventArgs& args) {
            whisperTo(args.position);
            m_game.click(args.position);
        });
        screen.on("onGobangTable", [this](const ui::EventArgs& args) { selectTable(std::atoi(args.source.c_str() + std::string("GobangTable").size())); });
        screen.on("onGobangQuit", [this](const ui::EventArgs&) { quit(); });
        screen.on("onGobangBackToHall", [this](const ui::EventArgs&) { backToHallClicked(); });
    }

    void GobangWindow::open(std::uint64_t nowMs) {
        m_nowMs = nowMs;
        m_game = GobangGame([&] {
            GobangGame::Hooks game = m_hooks.game;
            game.backToHall = [this] { sendBackToHall(); };
            return game;
        }());
        m_selectedTable = 0;
        m_anteFormShown = false;
        m_anteErrorShown = false;
        m_page.setVisible(true);
    }

    void GobangWindow::begin() {
        m_hooks.game.send({57, 11, 1}); // 57/11/1: enter the Five in a Row hall
        m_form.setVisible(true);
    }

    void GobangWindow::quit() {
        m_game.setTableIndex(0);
        m_selectedTable = 0;
        m_game.reset();
        m_hooks.stopMusic();
        m_hooks.endSport();
        m_hooks.closeInstructions();
        close();
        m_hooks.game.send({57, 11, 2}); // 57/11/2: quit Five in a Row
    }

    void GobangWindow::close() {
        m_form.setVisible(false);
        m_page.setVisible(false);
        m_ante.close();
    }

    void GobangWindow::selectTable(int number) {
        if (number < 1 || number > GobangGame::kTables) {
            return;
        }
        const std::int64_t gold = m_hooks.game.gold();
        const int ante = m_game.anteShown(number);
        const auto refuse = [&](int code) {
            m_selectMs = m_nowMs;
            m_anteErrorShown = true;
            tableButton(2).setVisible(false); // tables 2 and 3 are hidden for the refused moment
            tableButton(3).setVisible(false);
            m_hooks.game.anteError(code);
        };
        if (ante > gold) {
            refuse(1); // ante error 1: not enough gold
            return;
        }
        if (ante + gold > kMostGold) {
            refuse(2); // ante error 2: too much gold
            return;
        }
        m_ante.setBet(0);
        if (m_anteFormShown) {
            m_anteFormShown = false;
        } else if (!m_game.started()) {
            m_selectedTable = number;
            const GobangTable& table = m_game.table(number);
            const std::int32_t own = m_hooks.game.ownId();
            if ((table.player1 == 0 || table.player1 == own) && (table.player2 == 0 || table.player2 == own)) {
                m_anteFormShown = true;
            } else {
                m_hooks.noticeTwo("Opponent is waiting", kNoticeMs);
                m_game.setTableIndex(m_selectedTable);
                m_hooks.game.send(
                    {57, 11, 3, static_cast<std::uint8_t>(m_game.tableIndex()), static_cast<std::uint8_t>(m_game.mine().ante)}
                ); // 57/11/3: join a table (u8 table index, u8 ante code)
            }
        }
    }

    void GobangWindow::confirmAnte(int bet) {
        constexpr int kMostBet = 100000; // largest ante a player may type, gold
        if (bet > kMostBet) {
            m_ante.setBet(kMostBet);
            return;
        }
        if (bet < 0) {
            m_ante.setBet(0);
            return;
        }
        m_game.setTableIndex(m_selectedTable);
        const GobangTable& table = m_game.mine();
        const std::int32_t own = m_hooks.game.ownId();
        if ((table.player1 != 0 && table.player1 != own) || (table.player2 != 0 && table.player2 != own)) {
            m_hooks.game.notice("Some players reserved the table", kNoticeMs);
            m_anteFormShown = false;
            m_game.setTableIndex(0);
            return;
        }
        for (int code = 0; code < 5; ++code) { // five ante codes (none, 1000, 5000, 10000, 100000)
            if (GobangGame::anteGold(code) == bet) {
                m_game.table(m_game.tableIndex()).ante = code;
            }
        }
        m_anteFormShown = false;
        m_hooks.game.send(
            {57, 11, 3, static_cast<std::uint8_t>(m_game.tableIndex()), static_cast<std::uint8_t>(m_game.mine().ante)}
        ); // 57/11/3: join a table (u8 table index, u8 ante code)
    }

    void GobangWindow::backToHallClicked() {
        if (m_game.tableIndex() == 0) {
            sendBackToHall();
        } else if (m_game.anteShown(m_game.tableIndex()) > 0) {
            m_hooks.game.ask("You will lose money , if you leave now\rWant to go back to hall?", [this] { sendBackToHall(); }, [] {});
        } else {
            m_hooks.game.ask("Want to go back to hall?", [this] { sendBackToHall(); }, [] {});
        }
    }

    void GobangWindow::sendBackToHall() {
        m_game.reset();
        m_hooks.game.send({57, 11, 6}); // 57/11/6: back to the hall
        m_backButton.setVisible(false);
        m_quitButton.setVisible(true);
        for (int number = 1; number <= GobangGame::kTables; ++number) {
            tableButton(number).setVisible(true);
        }
    }

    void GobangWindow::drawPicture(gfx::Canvas& canvas, gfx::PictureId picture, core::Point at) const {
        if (picture.isValid()) {
            canvas.drawPicture(picture, at);
        }
    }

    void GobangWindow::paint(gfx::Canvas& canvas, std::uint64_t nowMs) {
        m_nowMs = nowMs;
        if (m_game.started()) {
            m_quitButton.setVisible(false);
            m_backButton.setVisible(true);
            paintBoard(canvas, nowMs);
        } else {
            m_backButton.setVisible(false);
            m_quitButton.setVisible(true);
            paintHall(canvas);
        }
    }

    void GobangWindow::whisperTo(core::Point mouse) {
        const std::int32_t own = m_game.me().id;
        if (!m_game.started()) {
            for (int number = 1; number <= GobangGame::kTables; ++number) {
                const core::Point at = tableCell(number);
                const GobangTable& table = m_game.table(number);
                const GobangSeat& first = m_game.seat1(number);
                const GobangSeat& second = m_game.seat2(number);
                if (first.id != 0 && first.id != own && !table.occupied &&
                    m_services.characters.figureContains(
                        first.look,
                        {at.x * m_tableStepSpot.pitch().x + m_firstSeatSpot.at().x, at.y * m_tableStepSpot.pitch().y + m_firstSeatSpot.at().y},
                        kFirstSeatSlot,
                        0,
                        mouse
                    )) {
                    m_hooks.whisper(first.id);
                }
                if (second.id != 0 && second.id != own && !table.occupied &&
                    m_services.characters.figureContains(
                        second.look,
                        {at.x * m_tableStepSpot.pitch().x + m_secondSeatSpot.at().x, at.y * m_tableStepSpot.pitch().y + m_secondSeatSpot.at().y},
                        kSecondSeatSlot,
                        0,
                        mouse
                    )) {
                    m_hooks.whisper(second.id);
                }
            }
        } else if (
            m_game.opponent().id != 0 && m_services.characters.portraitContains(m_game.opponent().look, m_opponentFaceSpot.at(), 2, 0, mouse)
        ) { // face slot 2, frame 0
            m_hooks.whisper(m_game.opponent().id);
        }
    }

    void GobangWindow::paintHall(gfx::Canvas& canvas) {
        drawPicture(canvas, picture("GameHall"), {0, 0});
        drawPicture(canvas, picture("Gobang_C"), m_hallTitleSpot.at());
        const gfx::PictureId desk = picture("Gobang_S");
        for (int number = 1; number <= GobangGame::kTables; ++number) {
            const core::Point at = tableCell(number);
            const GobangTable& table = m_game.table(number);
            const GobangSeat& first = m_game.seat1(number);
            const GobangSeat& second = m_game.seat2(number);
            if (table.occupied) {
                drawPicture(canvas, desk, {at.x * m_tableStepSpot.pitch().x + m_deskSpot.at().x, at.y * m_tableStepSpot.pitch().y + m_deskSpot.at().y});
            }
            if (first.id != 0 && !table.occupied) {
                m_services.characters.drawFigure(
                    canvas,
                    first.look,
                    {at.x * m_tableStepSpot.pitch().x + m_firstSeatSpot.at().x, at.y * m_tableStepSpot.pitch().y + m_firstSeatSpot.at().y},
                    kFirstSeatSlot,
                    0
                );
            }
            if (second.id != 0 && !table.occupied) {
                m_services.characters.drawFigure(
                    canvas,
                    second.look,
                    {at.x * m_tableStepSpot.pitch().x + m_secondSeatSpot.at().x, at.y * m_tableStepSpot.pitch().y + m_secondSeatSpot.at().y},
                    kSecondSeatSlot,
                    0
                );
            }
            ui::ImageButton& button = tableButton(number);
            if (first.id != 0 && second.id != 0) {
                button.setVisible(false);
            } else if (!m_anteErrorShown) {
                button.setVisible(true);
            }
            if ((first.id != 0 || second.id != 0) && !table.occupied) {
                drawPicture(
                    canvas,
                    picture("GdMoney2"),
                    {at.x * m_tableStepSpot.pitch().x + m_moneySpot.at().x, at.y * m_tableStepSpot.pitch().y + m_moneySpot.at().y}
                );
            }
        }
    }

    void GobangWindow::paintBoard(gfx::Canvas& canvas, std::uint64_t nowMs) {
        m_anteFormShown = false;
        m_game.advance(nowMs);
        drawPicture(canvas, picture("Gobang_BG2"), {0, 0});
        for (int number = 1; number <= GobangGame::kTables; ++number) {
            tableButton(number).setVisible(false);
        }
        const core::Point mouse = m_screen.mouse();
        const core::Point over = GobangGame::cellAt(mouse);
        const GobangTable& table = m_game.mine();
        const gfx::PictureId black = picture("Gobang_B");
        const gfx::PictureId white = picture("Gobang_W");
        for (int row = 1; row <= GobangGame::kSize; ++row) {
            for (int column = 1; column <= GobangGame::kSize; ++column) {
                const core::Point at{(column - 1) * GobangGame::kCell + GobangGame::kBoardOrigin.x, (row - 1) * GobangGame::kCell + GobangGame::kBoardOrigin.y};
                const int stone = m_game.cell(column, row);
                // cell contents: 1 black stone, 2 white stone, 0 empty
                if (stone == 1) {
                    drawPicture(canvas, black, at);
                } else if (stone == 2) {
                    drawPicture(canvas, white, at);
                } else if (stone == 0 && (table.turn == 1 || table.turn == 2) && !table.movePending && over.x == column && over.y == row) {
                    drawPicture(canvas, picture("icon_Gobang_y"), at);
                }
            }
        }
        // turn 1 = black to move, 2 = white to move
        if (table.turn == 1 || table.turn == 2) {
            const gfx::PictureId mine = table.turn == 1 ? picture("Gobang_B2") : picture("Gobang_W2");
            const gfx::PictureId theirs = table.turn == 1 ? picture("Gobang_W2") : picture("Gobang_B2");
            if (!table.movePending) {
                drawPicture(canvas, picture("Gobang_N"), m_turnBottomSpot.at());
                drawPicture(canvas, mine, m_bottomStoneSpot.at());
            } else {
                drawPicture(canvas, picture("Gobang_N"), m_turnTopSpot.at());
                drawPicture(canvas, theirs, m_topStoneSpot.at());
            }
        }
        if (m_game.over()) {
            if (nowMs - m_game.packetMs() > kResultMs && nowMs - m_game.packetMs() <= GobangGame::kBackToHallMs) {
                drawGameLetters(canvas, m_resultSpot.at(), m_game.resultText(), 0, 1.0F, m_resultSpot.pitch().x, 0, true); // 1.0F is the letter scale
            }
            return;
        }
        if (m_game.myMove()) {
            const std::uint64_t since = nowMs - m_game.lastStoneMs();
            if (m_game.lastStoneMs() != 0 && since < 3000) { // the last stone blinks for 3000 ms
                const gfx::PictureId cursor = picture("Cursor4");
                if (cursor.isValid()) {
                    const gfx::Picture& data = canvas.pictures().picture(cursor);
                    // the cursor strip has 2 frames; the cursor sits 4 pixels right and 1 up from the stone
                    const int frameHeight = data.height() / 2;
                    const int frame = static_cast<int>(since / kBlinkFrameMs) % 2;
                    canvas.drawPicture(
                        cursor,
                        {m_game.lastStone().x + 4, m_game.lastStone().y - 1},
                        core::Rect{0, frame * frameHeight, data.width(), frameHeight}
                    );
                }
            }
            drawPicture(canvas, picture("Icon_TimerBar"), m_timerBarSpot.at());
            for (int bar = 1; bar <= m_game.secondsBars(); ++bar) {
                drawPicture(canvas, picture("Icon_SecondBar_1"), {(bar - 1) * m_secondsBarSpot.pitch().x + m_secondsBarSpot.at().x, m_secondsBarSpot.at().y});
            }
        } else if (m_game.opponent().id != 0) {
            drawPicture(canvas, picture("Icon_TimerBar"), m_timerBarSpot.at());
        }
    }

    void GobangWindow::paintForm(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs) {
        if (m_game.started()) {
            m_services.characters.drawPortrait(canvas, m_game.me().look, m_myFaceSpot.at(), 0, 2); // frame 0, face slot 2
            if (m_game.opponent().id != 0) {
                m_services.characters.drawPortrait(canvas, m_game.opponent().look, m_opponentFaceSpot.at(), 0, 2); // frame 0, face slot 2
            }
        }
        if (m_anteFormShown) {
            if (!m_ante.isOpen()) {
                m_ante.open();
            }
        } else if (m_ante.isOpen()) {
            m_ante.close();
        }
        if (nowMs - m_selectMs > kSelectErrorMs && m_anteErrorShown) {
            // tables 2 and 3 are the ones hidden by a refused choice
            if (!m_game.table(2).occupied) {
                tableButton(2).setVisible(true);
            }
            if (!m_game.table(3).occupied) {
                tableButton(3).setVisible(true);
            }
            m_anteErrorShown = false;
        }
        if (m_game.started()) {
            paintBoardTexts(canvas, at);
        } else {
            paintHallTexts(canvas, at);
        }
    }

    void GobangWindow::paintHallTexts(gfx::Canvas& canvas, core::Point at) {
        for (int number = 1; number <= GobangGame::kTables; ++number) {
            const core::Point cell = tableCell(number);
            const core::Point shift{at.x + cell.x * m_tableStepSpot.pitch().x, at.y + cell.y * m_tableStepSpot.pitch().y};
            const GobangSeat& first = m_game.seat1(number);
            const GobangSeat& second = m_game.seat2(number);
            char numberText[8]; // buffer size for a two-digit table number
            std::snprintf(numberText, sizeof numberText, "%02d", number);
            drawEditorText(
                canvas,
                {shift.x + m_numberBoxSpot.at().x, shift.y + m_numberBoxSpot.at().y, m_labelSpot.size().width, m_labelSpot.size().height},
                numberText,
                m_whiteSpot.color()
            );
            if (first.id == 0 && second.id == 0) {
                canvas.drawText(
                    {shift.x + m_emptyTextSpot.at().x, shift.y + m_emptyTextSpot.at().y},
                    "No player reserves table",
                    {m_cyanSpot.color(), gfx::TextEffect::Shadow, std::nullopt},
                    m_emptyTextSpot.size().width,
                    m_emptyTextSpot.size().height
                );
                continue;
            }
            if (!m_game.table(number).occupied) {
                drawEditorText(
                    canvas,
                    {shift.x + m_anteBoxSpot.at().x, shift.y + m_anteBoxSpot.at().y, m_labelSpot.size().width, m_labelSpot.size().height},
                    std::to_string(m_game.anteShown(number)),
                    m_yellowSpot.color()
                );
            }
            drawEditorText(
                canvas,
                {shift.x + m_firstNameSpot.at().x, shift.y + m_firstNameSpot.at().y, m_labelSpot.size().width, m_labelSpot.size().height},
                "Players:" + (first.id != 0 ? first.name : std::string("Waiting")),
                m_cyanSpot.color()
            );
            drawEditorText(
                canvas,
                {shift.x + m_secondNameSpot.at().x, shift.y + m_secondNameSpot.at().y, m_labelSpot.size().width, m_labelSpot.size().height},
                "Players:" + (second.id != 0 ? second.name : std::string("Waiting")),
                m_cyanSpot.color()
            );
        }
    }

    void GobangWindow::paintBoardTexts(gfx::Canvas& canvas, core::Point at) {
        drawEditorText(
            canvas,
            {at.x + m_myNameSpot.at().x, at.y + m_myNameSpot.at().y, m_labelSpot.size().width, m_labelSpot.size().height},
            m_game.me().name,
            m_cyanSpot.color()
        );
        if (m_game.opponent().id != 0) {
            drawEditorText(
                canvas,
                {at.x + m_opponentNameSpot.at().x, at.y + m_opponentNameSpot.at().y, m_labelSpot.size().width, m_labelSpot.size().height},
                m_game.opponent().name,
                m_cyanSpot.color()
            );
        }
        if (!m_game.over() && !m_game.myMove() && m_game.opponent().id != 0) {
            drawEditorText(
                canvas,
                {at.x + m_statusSpot.rect().x, at.y + m_statusSpot.rect().y, m_statusSpot.rect().width, m_statusSpot.rect().height},
                "Opponent is thinking",
                m_yellowSpot.color()
            );
        }
    }

}
