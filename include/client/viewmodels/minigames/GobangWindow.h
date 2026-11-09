#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_GOBANG_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MINIGAMES_GOBANG_WINDOW_H

#include "client/services/GameServices.h"
#include "client/subsystems/minigames/GobangGame.h"
#include "client/viewmodels/minigames/AnteWindow.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief Five in a Row screen: the hall of sixteen tables with who sits at them, the chips form for a free table, and the board of a game with stones, turn, seconds left and the end.
    /// @details Original: GobangWindow.xml, LD_MiniGame.pas TGobangForm with DrawGobangHall and DrawGobangBoard. The game itself is a GobangGame.
    class GobangWindow {
        public:
            /// @brief Callbacks the window uses to reach the rest of the client.
            struct Hooks {
                GobangGame::Hooks game;
                std::function<void()> endSport;
                std::function<void()> closeInstructions;
                std::function<void()> stopMusic;
                std::function<void(const std::string&, int)> noticeTwo;
                std::function<void(std::int32_t)> whisper;
            };

            /// @brief Loads the window layout, builds the game and the chips form and wires the events.
            /// @param services Game services (layouts, pictures, characters).
            /// @param screen Screen the window is loaded into.
            /// @param hooks Callbacks to the rest of the client.
            GobangWindow(
                GameServices& services,
                ui::Screen& screen,
                Hooks hooks
            );

            /// @brief Tells whether the window is showing.
            /// @return True when the page is visible.
            bool isOpen() const { return m_page.isVisible(); }

            /// @brief Gives access to the game state.
            /// @return The GobangGame behind the window.
            GobangGame& game() { return m_game; }

            /// @brief Creates a fresh game and shows the hall under the instructions.
            /// @details Original: TGobang.Create.
            /// @param nowMs Current time in ms.
            void open(std::uint64_t nowMs);

            /// @brief Enters the hall and shows its form.
            /// @details Original: Begin on the instructions (StartFormButtonClick 1); sends 57/11/1.
            void begin();

            /// @brief Quits the game: ends the sport, closes the instructions and tells the server.
            /// @details Original: QuitGobangClick; sends 57/11/2.
            void quit();

            /// @brief Hides the window and the chips form.
            /// @details Original: FreeSportGame, the game is gone.
            void close();

        private:
            /// @brief Handles a table button: refuses when the gold does not cover the ante, asks for a bet at a free table, or joins a waiting player.
            /// @details Original: SelectGobangTable.
            /// @param number Table number, 1 to 16.
            void selectTable(int number);

            /// @brief Handles the bet confirmed on the chips form and asks for the table.
            /// @details Original: TAnteForm.ConfirmAnte; sends 57/11/3 with table index and ante code.
            /// @param bet Bet in gold.
            void confirmAnte(int bet);

            /// @brief Handles Back to Hall; asks first when an ante is at stake.
            /// @details Original: BackToHallClick; leaving a table with an ante at stake costs it.
            void backToHallClicked();

            /// @brief Resets the game and returns to the hall.
            /// @details Original: SendBackToHall; sends 57/11/6.
            void sendBackToHall();

            /// @brief Makes the player under the mouse the whisper target.
            /// @details Original: GobangMouseDown's loop over the seats (a seated player other than the own) and the opponent's face once the game began.
            /// @param mouse Mouse position on the screen.
            void whisperTo(core::Point mouse);

            /// @brief Draws the board while a game is on, else the hall.
            /// @details Original: DrawGobangGame.
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in ms.
            void paint(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws the hall: the tables, the seated players and the money.
            /// @details Original: DrawGobangHall.
            /// @param canvas Canvas to draw on.
            void paintHall(gfx::Canvas& canvas);

            /// @brief Runs the game logic and draws the stones, the cell under the mouse, whose turn it is and the end.
            /// @details Original: DrawGobangBoard (with the game's logic first).
            /// @param canvas Canvas to draw on.
            /// @param nowMs Current time in ms.
            void paintBoard(gfx::Canvas& canvas, std::uint64_t nowMs);

            /// @brief Draws the faces, the chips form state, a refused table coming back and the texts of the hall or board.
            /// @details Original: TGobangForm.DrawControl.
            /// @param canvas Canvas to draw on.
            /// @param at Top left of the form.
            /// @param nowMs Current time in ms.
            void paintForm(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Draws the table numbers, antes and player names of the hall.
            /// @param canvas Canvas to draw on.
            /// @param at Top left of the form.
            void paintHallTexts(gfx::Canvas& canvas, core::Point at);

            /// @brief Draws the two players' names and the "Opponent is thinking" status line.
            /// @param canvas Canvas to draw on.
            /// @param at Top left of the form.
            void paintBoardTexts(gfx::Canvas& canvas, core::Point at);

            /// @brief Gives a table's place in the hall grid.
            /// @param number Table number, 1 to 16.
            /// @return Column (0..3) in x and row (0..3) in y.
            static core::Point tableCell(int number) { return {(number - 1) % 4, (number - 1) / 4}; } // four tables per row

            /// @brief Gives the button of a table.
            /// @param number Table number, 1 to 16.
            /// @return The table's button.
            ui::ImageButton& tableButton(int number) { return m_tableButtons[static_cast<std::size_t>(number - 1)]; }

            /// @brief Finds a picture by name.
            /// @param name Picture name.
            /// @return The picture id, or an empty id when unknown.
            gfx::PictureId picture(const char* name) const { return m_services.pictures.find(name).value_or(gfx::PictureId{}); }

            /// @brief Draws a picture when it is valid.
            /// @param canvas Canvas to draw on.
            /// @param picture Picture to draw.
            /// @param at Where to draw it.
            void drawPicture(gfx::Canvas& canvas, gfx::PictureId picture, core::Point at) const;

            GameServices& m_services;
            ui::Screen& m_screen;
            Hooks m_hooks;
            GobangGame m_game;
            AnteWindow m_ante;
            ui::Element& m_page;
            ui::Element& m_form;
            ui::ImageButton& m_quitButton;
            ui::ImageButton& m_backButton;
            const ui::Spot& m_tableStepSpot;
            const ui::Spot& m_hallTitleSpot;
            const ui::Spot& m_deskSpot;
            const ui::Spot& m_firstSeatSpot;
            const ui::Spot& m_secondSeatSpot;
            const ui::Spot& m_moneySpot;
            const ui::Spot& m_turnBottomSpot;
            const ui::Spot& m_turnTopSpot;
            const ui::Spot& m_bottomStoneSpot;
            const ui::Spot& m_topStoneSpot;
            const ui::Spot& m_timerBarSpot;
            const ui::Spot& m_secondsBarSpot;
            const ui::Spot& m_resultSpot;
            const ui::Spot& m_labelSpot;
            const ui::Spot& m_anteBoxSpot;
            const ui::Spot& m_numberBoxSpot;
            const ui::Spot& m_firstNameSpot;
            const ui::Spot& m_secondNameSpot;
            const ui::Spot& m_emptyTextSpot;
            const ui::Spot& m_myNameSpot;
            const ui::Spot& m_opponentNameSpot;
            const ui::Spot& m_statusSpot;
            const ui::Spot& m_myFaceSpot;
            const ui::Spot& m_opponentFaceSpot;
            const ui::Spot& m_cyanSpot;
            const ui::Spot& m_yellowSpot;
            const ui::Spot& m_whiteSpot;
            std::vector<std::reference_wrapper<ui::ImageButton>> m_tableButtons;
            int m_selectedTable = 0;
            bool m_anteFormShown = false;
            bool m_anteErrorShown = false;
            std::uint64_t m_selectMs = 0;
            std::uint64_t m_nowMs = 0;
    };

}

#endif
