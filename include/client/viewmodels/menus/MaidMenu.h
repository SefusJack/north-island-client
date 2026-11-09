#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_MENUS_MAID_MENU_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_MENUS_MAID_MENU_H

#include "client/services/GameServices.h"
#include "core/Bytes.h"
#include "ui/DrawingArea.h"
#include "ui/ListBox.h"
#include "ui/Screen.h"
#include "ui/Spot.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief The Wit Maid's menu: a wide talk box with her figure, words and choices.
    /// @details Original: MaidMenu.xml, HL_MaidMenu.pas THL_MaidManager with HL_TalkMsgFormPlus_1 in its wide layout.
    /// A click on one of her pieces of furniture in a house starts it. A wide talk box at the bottom (Form_Talk_1)
    /// shows her large figure at the left and her words, then the choices (the item bank, the pets' inn, her
    /// clothes, ...); the bank and the inn open the game's own forms, and a costume of the bag changes the piece (76/3).
    class MaidMenu {
        public:
            /// @brief Callbacks into packet sending, the furniture and the bank and inn forms.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<int(int furnIndex)> pieceItem;
                std::function<int(int itemId)> suitSlot;
                std::function<void(int furnIndex, int itemId)> changePiece;
                std::function<void()> openBank;
                std::function<void()> openInn;
            };

            /// @brief Loads the menu layout and wires its painter and choice list.
            /// @param services Shared game services.
            /// @param screen Screen the menu is loaded into.
            /// @param hooks Callbacks into the surrounding systems.
            MaidMenu(
                GameServices& services,
                ui::Screen& screen,
                Hooks hooks
            );

            /// @brief Tells whether the menu is going on (it keeps the player from walking and the map from taking presses).
            /// @details Original: THL_MaidManager.FActive.
            /// @return True while the menu is active.
            bool isActive() const { return m_active; }

            /// @brief Tells whether the box is on the screen.
            /// @return True when shown.
            bool isOpen() const { return m_visible; }

            /// @brief Starts the menu for a clicked piece of furniture.
            /// @details Original: DZ_Thing.pas HandleUtensilClick; a piece of a maid kind above 1 was clicked.
            /// @param furnIndex Number of the piece of furniture.
            void open(int furnIndex);

            /// @brief Handles the server's maid action.
            /// @details Original: RecvMaidAction. 76/1 the item bank, 76/2 the inn, 76/3 the new costume, 76/250 and the rest close it.
            /// @param payload Message payload; byte 1 is the sub id.
            void received(const core::Bytes& payload);

            /// @brief Tells the server the bank or the inn closed while the menu is active (76/250).
            /// @details Original: TH_BankItemForm.HideCtrl, TRe_InnForm.HideCtrl.
            void formClosed();

            /// @brief Resets the menu and hides the box (a map change).
            /// @details Original: ResetMaidManager.
            void reset();

        private:
            /// @brief Shows one page of the menu: greeting, choices, clothes question or clothes choices.
            /// @details Original: ShowMaidMenuPage.
            /// @param page Page number.
            void showPage(int page);

            /// @brief Handles a choice of the main menu.
            /// @details Original: HandleMaidChoice.
            /// @param choice Index of the choice.
            void handleChoice(int choice);

            /// @brief Handles a costume choice; the choice counts on past the first page.
            /// @details Original: ChangeMaidCostume.
            /// @param choice Index of the choice on the page.
            void changeCostume(int choice);

            /// @brief Sends the chosen costume change (76/3).
            /// @details Original: SendMaidCostumeChange.
            void sendCostumeChange();

            /// @brief Shows her words; a click closes the box and calls the callback.
            /// @details Original: ShowShortMsgCb.
            /// @param text Words to show.
            /// @param onClosed Called when the box is clicked away.
            void say(const std::string& text, std::function<void()> onClosed);

            /// @brief Shows a list of choices.
            /// @details Original: ShowTalkChoiceClose.
            /// @param choices Choice captions.
            /// @param onChoice Called with the chosen index.
            void choose(const std::vector<std::string>& choices, std::function<void(int)> onChoice);

            /// @brief Closes the box on a click and calls what was asked for.
            /// @details Original: HandleMouseUp of the box.
            void clicked();

            /// @brief Hides the box and its choices.
            void hide();

            /// @brief Paints the talk box, her words, her figure and the next arrow.
            /// @param canvas Target canvas.
            /// @param at Top-left of the box.
            /// @param nowMs Current time in milliseconds.
            void paint(gfx::Canvas& canvas, core::Point at, std::uint64_t nowMs);

            /// @brief Paints her figure by the kind of her piece.
            /// @details Original: DrawPreviewHuman for body type 250.
            /// @param canvas Target canvas.
            void paintFigure(gfx::Canvas& canvas);

            GameServices& m_services;
            Hooks m_hooks;
            ui::DrawingArea& m_content;
            ui::ListBox& m_choices;
            const ui::Spot& m_textSpot;
            const ui::Spot& m_figureSpot;
            bool m_active = false;
            bool m_visible = false;
            std::optional<int> m_furn;
            int m_page = 1;
            core::Bytes m_costume;
            int m_figureItem = 0;
            bool m_speaker = false;
            std::vector<std::string> m_lines;
            std::function<void()> m_onClosed;
            std::function<void(int)> m_onChoice;
    };

}

#endif
