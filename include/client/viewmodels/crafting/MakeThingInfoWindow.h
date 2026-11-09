#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_MAKE_THING_INFO_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_CRAFTING_MAKE_THING_INFO_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/house/HomeFurniture.h"
#include "client/models/crafting/MakeCheck.h"
#include "client/models/crafting/MakeJobs.h"
#include "client/rules/MakeMaterials.h"
#include "client/viewmodels/chat/PlayerMessages.h"
#include "core/Bytes.h"
#include "gfx/PictureId.h"
#include "ui/BlockList.h"
#include "ui/DrawingArea.h"
#include "ui/Screen.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

#include <functional>
#include <string>

namespace client {

    /// @brief The manufacture window (MakeThingInfoWindow.xml): a recipe's product, its materials with the counts the player has, the making time and, with a piece 38052 in the house, how many times to make it.
    /// @details Original: RE_Menu.pas TRe_MakeThingInfoForm. Confirm asks the server (64/1).
    class MakeThingInfoWindow {
        public:
            /// @brief Callbacks into the scene: packet send, house floor room test, refusal text, bathing state and the maid partner choice.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
                std::function<std::string()> refusal;
                std::function<bool()> bathing;
                std::function<bool()> hasMaid;
                std::function<void(std::function<bool(int, int)>)> choosePartner;
                std::function<void()> partnerOrdered;
            };

            /// @brief Loads the window layout and wires its rows, buttons and painters.
            /// @param services Game services (layouts, recipes, items, pictures).
            /// @param screen Screen the layout is loaded into.
            /// @param furniture The home furniture.
            /// @param jobs Manufacture jobs.
            /// @param materials Material counts of bag and house.
            /// @param messages Player message line.
            /// @param hooks Scene callbacks.
            MakeThingInfoWindow(
                GameServices& services,
                ui::Screen& screen,
                const HomeFurniture& furniture,
                MakeJobs& jobs,
                const MakeMaterials& materials,
                PlayerMessages& messages,
                Hooks hooks
            );

            /// @brief Whether the window is showing.
            /// @return True while visible.
            bool isOpen() const { return m_window.isVisible(); }

            /// @brief Shows the recipe made at a piece of furniture (ShowMakeThingRecipe and ShowCtrl).
            /// @param recipe Recipe id.
            /// @param piece Furniture piece the recipe is made at.
            /// @param blueprintSource Source of the blueprint (0, 0 with its place for none).
            /// @param blueprintSlot Place of the blueprint.
            void show(int recipe, int piece, int blueprintSource, int blueprintSlot);

            /// @brief Closes the window (HideCtrl).
            void close();

        private:
            /// @brief Counts what the bag and the house hold into the rows and check marks (CheckMakeMaterials), and cuts the quantity.
            void refresh();

            /// @brief Confirms the making; with a Maid a thing that takes time asks which partner makes it (ConfirmMakeThing).
            void confirm();

            /// @brief Does the checks then sends 64/1 with the partner (the rest of ConfirmMakeThing and NpcMakeItemPicked's new order).
            /// @param viaPartners True when called from the partners' form.
            /// @param petSlot Partner's place (0 for the player).
            /// @param petId Partner's id (0 for the player).
            /// @return True when the partners' form is done.
            bool submit(bool viaPartners, int petSlot, int petId);

            /// @brief Gets the number of batches asked for.
            /// @return The quantity.
            int quantity() const;

            /// @brief Sets the number of batches asked for.
            /// @param quantity New quantity.
            void setQuantity(int quantity);

            /// @brief Whether a piece 38052 stands in the house (it lets a recipe be made more than once).
            /// @return True when it does.
            bool hasQuantityPiece() const;

            /// @brief Paints the window's content (TRe_MakeThingInfoForm.DrawControl).
            /// @param canvas Target canvas.
            /// @param at Top-left of the paint area.
            void paint(gfx::Canvas& canvas, core::Point at);

            /// @brief Paints the rows' check marks (RenderWithChildren's marks).
            /// @param canvas Target canvas.
            /// @param at Top-left of the paint area.
            void paintMarks(gfx::Canvas& canvas, core::Point at);

            /// @brief Gets the minutes of all the batches asked for, with the furniture's savings.
            /// @return Minutes.
            int makeMinutes() const;

            /// @brief Fills the material rows from the recipe.
            void fillRows();

            GameServices& m_services;
            ui::Screen& m_screen;
            const HomeFurniture& m_furniture;
            MakeJobs& m_jobs;
            const MakeMaterials& m_materials;
            PlayerMessages& m_messages;
            Hooks m_hooks;
            ui::Window& m_window;
            ui::BlockList& m_list;
            ui::TextBox& m_count;
            const ui::Spot& m_titleSpot;
            const ui::Spot& m_productSpot;
            const ui::Spot& m_neededSpot;
            const ui::Spot& m_timeSpot;
            const ui::Spot& m_quantitySpot;
            const ui::Spot& m_plusSpot;
            const ui::Spot& m_frameSpot;
            const ui::Spot& m_rowColorSpot;
            const ui::Spot& m_markSpot;
            int m_recipe = 0;
            int m_piece = 0;
            int m_blueprintSource = 0;
            int m_blueprintSlot = 0;
            MakeCheck m_check;
            gfx::PictureId m_enough;
            gfx::PictureId m_lack;
            gfx::PictureId m_grid;
            gfx::PictureId m_plus;
            gfx::PictureId m_quantity;
    };

}

#endif
