#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_RULES_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_RULES_FORM_H

#include "client/services/GameServices.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Point.h"
#include "ui/Memo.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <string>

namespace client {

    /// @brief The guild rules edit form (GuildRulesForm.xml, TSe_ChangeDeclareForm); Confirm sends the rules as 39/9.
    class GuildRulesForm {
        public:
            /// @brief Loads the form layout and wires its buttons and memo.
            /// @param services Game services (layouts, connection).
            /// @param screen Screen the form is loaded into.
            /// @param messages Player message line used for the "rules too long" notice.
            GuildRulesForm(
                GameServices& services,
                ui::Screen& screen,
                PlayerMessages& messages
            );

            /// @brief Tells whether the form is shown.
            /// @return True when the form is visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form below the guild tab with the current rules, or hides it when already shown.
            /// @details Original: ToggleFormShown, from the guild tab at a place.
            /// @param guildTab Corner of the guild tab the form is placed from.
            /// @param rules The rules as they are now.
            void toggle(core::Point guildTab, const std::string& rules);

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Sends the typed rules (39/9) and closes the form; does nothing for empty text.
            /// @details Original: SaveGuildRule.
            void save();

            GameServices& m_services;
            ui::Screen& m_screen;
            PlayerMessages& m_messages;
            ui::Window& m_window;
            ui::Memo& m_text;
            const ui::Spot& m_tabSpot;
    };

}

#endif
