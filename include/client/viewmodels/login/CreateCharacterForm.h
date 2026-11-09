#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_CREATE_CHARACTER_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_LOGIN_CREATE_CHARACTER_FORM_H

#include "client/services/GameServices.h"
#include "client/services/NameCheckResult.h"
#include "client/models/character/CharacterLook.h"
#include "client/models/character/FigureAnimation.h"
#include "client/rendering/characters/PortraitBlink.h"
#include "client/viewmodels/login/PasswordForm.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "client/viewmodels/login/RoleCarousel.h"
#include "formats/ColorPattern.h"
#include "protocol/CreateCharacterRequest.h"
#include "ui/DigitText.h"
#include "ui/DrawingArea.h"
#include "ui/Element.h"
#include "ui/ImageButton.h"
#include "ui/ImageView.h"
#include "ui/Label.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace client {

    /// @brief Four-page character creation wizard (role and name, stat points, element, colours) plus optional cipher page.
    /// @details Original: CreateCharacterForm.xml; name check is server action 9/2 and 9/3; reset mirrors ResetCreateCharacter.
    class CreateCharacterForm {
        public:
            /// @brief Hooks the wizard calls to talk to the rest of the client.
            struct Callbacks {
                std::function<void()> cancel;
                std::function<void(const std::string& name)> checkName;
                std::function<void(const protocol::CreateCharacterRequest&)> create;
            };

            /// @brief Loads the wizard layout and wires its controls.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param messages Short player message line.
            /// @param passwordForm Form used to ask for a cipher.
            /// @param callbacks Cancel, name check and create hooks.
            CreateCharacterForm(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                PasswordForm& passwordForm,
                Callbacks callbacks
            );

            /// @brief Opens the first page with everything reset.
            /// @param askCipher True when the account has no cipher yet.
            void show(bool askCipher);

            /// @brief Closes the wizard and resets it.
            void hide();

            /// @brief Tells whether the wizard is shown.
            /// @return True when visible.
            bool isVisible() const { return m_form.isVisible(); }

            /// @brief Handles the server's answer to the name check: on to page 2, or a message.
            /// @param result Outcome of the name check.
            void nameChecked(NameCheckResult result);

            /// @brief Advances the role carousel while on the first page.
            /// @param nowMs Current time in milliseconds.
            void update(std::uint64_t nowMs);

        private:
            /// @brief Wizard pages in order.
            enum class Page { Closed, Role, Points, Element, Colors, Cipher };

            /// @brief Puts everything back as on a new wizard.
            void reset();

            /// @brief Switches to a page and faces the figure south-west.
            /// @param page Target page.
            void goTo(Page page);

            /// @brief Shows and positions the parts that belong to the current page.
            void showPage();

            /// @brief Handles the next button for the current page.
            void next();

            /// @brief Handles the previous button for the current page.
            void previous();

            /// @brief Finishes the wizard from the colour page when no cipher is needed.
            void confirm();

            /// @brief Builds the create request, closes the wizard and reports it.
            /// @param cipher Cipher chosen on the cipher page, if any.
            void finish(std::optional<std::string> cipher);

            /// @brief Turns the figure one step around its eight directions.
            /// @param step -1 or 1.
            void turn(int step);

            /// @brief Selects a starter role.
            /// @param role Role number, 1-based.
            void selectRole(int role);

            /// @brief Selects an element.
            /// @param element Element number, 1-based.
            void selectElement(int element);

            /// @brief Selects the colour part (hair, skin, clothing, eyes) to edit.
            /// @param part Part number, 1-based.
            void selectPart(int part);

            /// @brief Moves a stat point between the pool and a stat.
            /// @param stat Stat number, 1-based.
            /// @param step -1 takes a point back, 1 gives one.
            void changeStat(int stat, int step);

            /// @brief Changes one colour channel of the chosen part.
            /// @param channel Channel number, 1-based.
            /// @param step -1 or 1.
            void changeColor(int channel, int step);

            /// @brief Resets all colours to the sprite's own.
            void resetColors();

            /// @brief Redraws numbers, pictures and name for the current state.
            void refresh();

            /// @brief Builds the look of the figure being created.
            /// @return The character look.
            CharacterLook look() const;

            /// @brief Paints the animated figure.
            /// @param canvas Target canvas.
            /// @param feet Point under the figure's feet.
            /// @param nowMs Current time in milliseconds.
            void paintFigure(gfx::Canvas& canvas, core::Point feet, std::uint64_t nowMs);

            /// @brief Paints the blinking portrait.
            /// @param canvas Target canvas.
            /// @param anchor Portrait anchor point.
            /// @param nowMs Current time in milliseconds.
            void paintPortrait(gfx::Canvas& canvas, core::Point anchor, std::uint64_t nowMs);

            /// @brief Binds a screen event to a parameterless action.
            /// @param screen Screen to register on.
            /// @param event Event name.
            /// @param action Action to run.
            void on(ui::Screen& screen, const std::string& event, std::function<void()> action);

            GameServices& m_services;
            PlayerMessages& m_messages;
            PasswordForm& m_passwordForm;
            Callbacks m_callbacks;
            ui::Element& m_form;
            std::vector<std::reference_wrapper<ui::Element>> m_pageBackgrounds;
            ui::Element& m_rolePage;
            ui::Element& m_pointsPage;
            ui::Element& m_pointsBonuses;
            ui::Element& m_pointsControls;
            ui::Element& m_elementControls;
            ui::Element& m_colorPage;
            ui::Element& m_colorControls;
            ui::DrawingArea& m_figure;
            ui::DrawingArea& m_portrait;
            ui::DigitText& m_pointsLeftText;
            std::vector<std::reference_wrapper<ui::DigitText>> m_statTexts;
            std::vector<std::reference_wrapper<ui::Label>> m_bonusTexts;
            ui::ImageView& m_description;
            ui::ImageView& m_elementFrame;
            ui::ImageView& m_partFrame;
            std::vector<std::reference_wrapper<ui::ImageView>> m_colorBars;
            ui::ImageView& m_rolePicture;
            ui::Label& m_nameText;
            ui::TextBox& m_nameBox;
            ui::ImageView& m_elementIcon;
            ui::ImageButton& m_turnLeft;
            ui::ImageButton& m_turnRight;
            ui::ImageButton& m_previousButton;
            ui::ImageButton& m_nextButton;
            ui::ImageButton& m_confirmButton;
            const ui::Spot& m_nameTextSpot;
            const ui::Spot& m_elementDescriptionSpot;
            const ui::Spot& m_elementRoleSpot;
            RoleCarousel m_carousel;

            Page m_page = Page::Closed;
            bool m_askCipher = false;
            bool m_nameCheckPending = false;
            int m_role = 1;
            int m_element = 1;
            std::array<int, 5> m_stats{};
            int m_pointsLeft = 0;
            int m_colorPart = 1;
            std::array<int, 3> m_partColors{};
            formats::ColorPattern m_colors;
            int m_direction = 11; // facing 11: default direction (directions run 8..15)
            FigureAnimation m_animation;
            PortraitBlink m_blink;
    };

}

#endif
