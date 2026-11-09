#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_CREATE_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_CREATE_FORM_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <functional>
#include <string>

namespace client {

    /// @brief The create guild form: Confirm checks the name and founds the guild, Cancel closes it.
    /// @details Original: GuildCreateForm.xml, TSe_ArmyCreateForm; the server opens it with 39/40, Confirm sends 39/1.
    class GuildCreateForm {
        public:
            /// @brief Loads the layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param messages Sink for short player messages.
            /// @param closed Called when the form closes; asks the opening event for its next result.
            GuildCreateForm(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages,
                std::function<void()> closed
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form with the keyboard in the name field.
            void open();

            /// @brief Takes the typed name and empties the field.
            /// @details Original: RecvCreateGuildResult 0 uses the name the guild got.
            /// @return The typed name.
            std::string takeName();

        private:
            /// @brief Hides the form and tells the opening event.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Checks the name and sends the founding request.
            /// @details Original: CreateGuildClick.
            void confirm();

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            std::function<void()> m_closed;
            ui::Window& m_window;
            ui::TextBox& m_name;
    };

}

#endif
