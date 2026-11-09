#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_INSTANCES_INSTANCE_DETAIL_FORM_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_INSTANCES_INSTANCE_DETAIL_FORM_H

#include "client/services/GameServices.h"
#include "client/models/instances/Instances.h"
#include "ui/ListBox.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/Window.h"

#include <cstdint>
#include <vector>

namespace client {

    /// @brief The detail form of an open instance: name, level, players, guild and time limits, description and the list of its players.
    /// @details Original: InstanceDetailForm.xml, TDupDetailForm. The server opens it (85/4); Join asks to be let in (85/5).
    class InstanceDetailForm {
        public:
            /// @brief Loads the form and wires its events.
            /// @param services Shared game services.
            /// @param screen Screen the form is loaded into.
            /// @param instances Instance model (detail data, join request).
            InstanceDetailForm(
                GameServices& services,
                ui::Screen& screen,
                Instances& instances
            );

            /// @brief Tells whether the form is shown.
            /// @return True when visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the form and brings it to the front.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Hides the form.
            /// @details Original: HideCtrl.
            void close() { m_window.setVisible(false); }

        private:
            /// @brief Paints the texts of the instance and keeps the list of its players up to date.
            /// @details Original: RenderWithChildren.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Rebuilds the player list when the instance's member list changed.
            /// @details Original: ClearDupDetail, AddDupDetailMember and RemoveDupDetailMember are made to the instance's list; the form shows it as it is.
            void syncMembers();

            Instances& m_instances;
            ui::Screen& m_screen;
            ui::Window& m_window;
            ui::ListBox& m_members;
            const ui::Spot& m_infoSpot;
            const ui::Spot& m_titleSpot;
            std::vector<std::int32_t> m_listed;
    };

}

#endif
