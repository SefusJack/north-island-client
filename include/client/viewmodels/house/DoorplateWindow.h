#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_DOORPLATE_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_DOORPLATE_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/Doorplates.h"
#include "core/Bytes.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>

namespace client {

    /// @brief The Doorplate's form (DoorplateWindow.xml, TH_doorplateInPutForm): a field for the doorplate text of up to 14 bytes.
    /// @details Original: TH_Menu.pas TH_doorplateInPutForm, opened by the owner's click on a Doorplate 39056 in the house; the field starts with the player's doorplate. OK sends it (62/46; an empty text is 62/45, no doorplate) and the form goes; the server answers with the list of doorplates of the scene (62/46, 62/45), which Doorplates keeps and a tent shows above its owner's name.
    class DoorplateWindow {
        public:
            /// @brief Callbacks into the rest of the client that the doorplate form needs.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
                std::function<std::int32_t()> ownId;
            };

            /// @brief Loads the window layout and wires the OK and close buttons.
            /// @param services Game services (layouts).
            /// @param screen Screen the window is loaded into.
            /// @param doorplates The doorplates of the scene.
            /// @param hooks Callbacks into the rest of the client.
            DoorplateWindow(
                GameServices& services,
                ui::Screen& screen,
                const Doorplates& doorplates,
                Hooks hooks
            );

            /// @brief Shows the form with the player's doorplate in the field.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Per frame: out of the house the form goes.
            /// @details Original: DrawControl (HL_AllFormManager hides it).
            void update();

        private:
            /// @brief Sends the doorplate text (an empty field takes the doorplate away) and hides the form.
            /// @details Original: ConfirmDoorplate.
            void confirm();

            /// @brief Hides the form.
            void hide() { m_window.setVisible(false); }

            ui::Screen& m_screen;
            const Doorplates& m_doorplates;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::TextBox& m_text;
    };

}

#endif
