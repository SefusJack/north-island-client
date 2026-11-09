#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_AUTHORITY_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_GUILD_GUILD_AUTHORITY_FORM_H

#include "client/services/GameServices.h"
#include "client/models/guild/Guild.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <array>
#include <cstdint>
#include <functional>

namespace client {

    /// @brief Form that sets what the ticked guild members may do, and appoints or deposes a vice lord.
    /// @details Original: GuildAuthorityForm.xml, TTH_setauthorityForm; OK sends the ticks per ticked member (39/16).
    class GuildAuthorityForm {
        public:
            /// @brief Loads the layout and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the layout is loaded into.
            /// @param guild The guild model.
            /// @param ownId Gives the own player's id.
            GuildAuthorityForm(
                GameServices& services,
                ui::Screen& screen,
                Guild& guild,
                std::function<std::int32_t()> ownId
            );

            /// @brief Tells whether the form is visible.
            /// @return True when open.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Opens the form, or closes it when it is open.
            /// @details Original: ToggleSetAuthorityForm (AuthorityMemberSel).
            /// @param selection The ticked member's index in the guild plus one, or minus the number of them when there are several.
            void toggle(int selection);

            /// @brief Hides the form.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Shows the form with the ticked member's name and rights.
            /// @details Original: ShowCtrl.
            /// @param selection As for toggle().
            void open(int selection);

            /// @brief Flips the vice lord tick (one ticked member, from the lord only).
            /// @details Original: ToggleViceLeaderCheck.
            void toggleVice();

            /// @brief Flips a right's tick unless the own player lacks that right.
            /// @details Original: ToggleAuthorityCheck.
            /// @param right Right number from 1.
            void toggleRight(int right);

            /// @brief Sends the ticks for each ticked member and the vice lord change, then closes.
            /// @details Original: ApplyMemberAuthority; the rights go with 39/16, the appointing with 39/14, the deposing with 39/11.
            void apply();

            /// @brief Sets a right's tick and its picture.
            /// @param right Right number from 1.
            /// @param checked New tick state.
            void showCheck(int right, bool checked);

            /// @brief Sets the vice lord tick and its picture.
            /// @param checked New tick state.
            void showVice(bool checked);

            GameServices& m_services;
            ui::Screen& m_screen;
            Guild& m_guild;
            std::function<std::int32_t()> m_ownId;
            ui::Window& m_window;
            std::array<bool, 5> m_checks{};
            bool m_viceChecked = false;
    };

}

#endif
