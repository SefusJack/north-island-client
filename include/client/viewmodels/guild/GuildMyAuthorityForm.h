#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_MY_AUTHORITY_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_MY_AUTHORITY_FORM_H

#include "client/services/GameServices.h"
#include "client/models/guild/Guild.h"
#include "ui/Screen.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <string>

namespace client {

    /// @brief Form showing the own name and a mark at each right the own player has in the guild.
    /// @details Original: GuildMyAuthorityForm.xml, TTH_MYauthorityForm.
    class GuildMyAuthorityForm {
        public:
            /// @brief Accessors for the own player's id and name.
            struct Own {
                std::function<std::int32_t()> id;
                std::function<std::string()> name;
            };

            /// @brief Loads the layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param guild The guild model.
            /// @param own Accessors for the own player's id and name.
            GuildMyAuthorityForm(
                GameServices& services,
                ui::Screen& screen,
                const Guild& guild,
                Own own
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form, or closes it when it is open.
            /// @details Original: ToggleMyAuthorityForm.
            void toggle();

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Shows the form with the marks of the own rights.
            /// @details Original: ShowCtrl.
            void open();

            ui::Screen& m_screen;
            const Guild& m_guild;
            Own m_own;
            ui::Window& m_window;
            ui::TextBox& m_name;
    };

}

#endif
