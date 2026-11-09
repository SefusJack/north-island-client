#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_INSTANCES_INSTANCE_CREATE_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_INSTANCES_INSTANCE_CREATE_FORM_H

#include "client/services/GameServices.h"
#include "client/models/instances/InstanceCreateRow.h"
#include "client/models/instances/Instances.h"
#include "client/viewmodels/instances/InstanceMessageForm.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "ui/ListBox.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <string>
#include <vector>

namespace client {

    /// @brief The form to pick an instance to create; instances come lowest level first, ten a page.
    /// @details Original: InstanceCreateForm.xml, TCreateDupMisForm. The instance under the mouse, or the chosen one, shows its rules and description. Create (or a double click) asks for a message in InstanceMessageForm; its Confirm asks the server to create the instance (85/3).
    class InstanceCreateForm {
        public:
            /// @brief Loads the form and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param instances Instance model (rows, create request).
            /// @param messages Player message line used for refusals.
            InstanceCreateForm(
                GameServices& services,
                ui::Screen& screen,
                Instances& instances,
                PlayerMessages& messages
            );

            /// @brief Tells whether the form is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form; the list is made once.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the form; the message form goes with it.
            /// @details Original: HideCtrl.
            void close();

            /// @brief Clears the chosen row.
            /// @details Original: ClearCreateDupSelection.
            void clearSelection();

            /// @brief Rebuilds the list after the flags changed (starts again when it was made).
            /// @details Original: ReloadCreateDupList.
            void listChanged();

        private:
            /// @brief Puts every instance in the list, the first page showing.
            /// @details Original: LoadCreateDupList.
            void loadList();

            /// @brief Shows the instances of a page.
            /// @details Original: ShowCreateDupPage.
            /// @param page Page number (1-based).
            void showPage(int page);

            /// @brief Goes to the previous page.
            void previousPage();

            /// @brief Goes to the next page.
            void nextPage();

            /// @brief Chooses a row unless the message form is open.
            /// @details Original: SelectCreateDupRow.
            /// @param index Row index on the page.
            void select(int index);

            /// @brief Asks for the message when an instance is chosen and may be created.
            /// @details Original: ConfirmCreateDup.
            void confirmCreate();

            /// @brief Asks the server to create the chosen instance with the message.
            /// @details Original: SendCreateDup.
            /// @param message Message text from the message form.
            void send(const std::string& message);

            /// @brief Clears the selection and closes the form.
            /// @details Original: CloseCreateDupForm.
            void closeAndClear();

            /// @brief Paints the texts of the instance and the page numbers.
            /// @details Original: RenderWithChildren.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            ui::Screen& m_screen;
            Instances& m_instances;
            PlayerMessages& m_messages;
            ui::Window& m_window;
            ui::ListBox& m_list;
            const ui::Spot& m_titleSpot;
            const ui::Spot& m_playersSpot;
            const ui::Spot& m_levelSpot;
            const ui::Spot& m_guildSpot;
            const ui::Spot& m_timeSpot;
            const ui::Spot& m_descriptionSpot;
            const ui::Spot& m_pageNumbersSpot;
            const ui::Spot& m_messageSpot;
            InstanceMessageForm m_message;
            bool m_loaded = false;
            std::vector<InstanceCreateRow> m_all;
            std::vector<InstanceCreateRow> m_shown;
            int m_page = 0;
            int m_pages = 0;
            int m_selected = -1;
    };

}

#endif
