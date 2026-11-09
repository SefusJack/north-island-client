#ifndef NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_PORTRAIT_WINDOW_H
#define NORTH_ISLAND_CLIENT_VIEWMODELS_HOUSE_PORTRAIT_WINDOW_H

#include "client/services/GameServices.h"
#include "client/models/character/CharacterLook.h"
#include "core/Bytes.h"
#include "core/Point.h"
#include "gfx/Canvas.h"
#include "ui/Screen.h"
#include "ui/Window.h"

#include <cstdint>
#include <functional>
#include <optional>

namespace client {

    /// @brief The portrait form of the Picture Frame (39153) and the Good Picture Frame (39154) of a house.
    /// @details Original: PortraitWindow.xml, HL_Menu.pas TPortraitForm; a click on the piece shows the house's owner in the frame and asks the server (62/66) for the second person (62/62), a man at the left and a woman at the right of the frame.
    class PortraitWindow {
        public:
            /// @brief Callbacks the window needs from its owner.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<bool()> inFloorRoom;
                std::function<std::int32_t()> houseOwnerId;
                std::function<std::int32_t()> ownId;
                std::function<CharacterLook()> ownLook;
                std::function<std::optional<CharacterLook>(std::int32_t)> worldLook;
                std::function<int(int)> pieceItem;
            };

            /// @brief Loads the form and wires its events.
            /// @param services Game services (layouts, pictures, characters).
            /// @param screen Screen the form is loaded into.
            /// @param hooks Owner callbacks.
            PortraitWindow(
                GameServices& services,
                ui::Screen& screen,
                Hooks hooks
            );

            /// @brief Handles a click on a Picture Frame piece: loads the owner and asks the server for the second person.
            /// @param furnIndex Index of the clicked furniture piece.
            /// @param itemId Item id of the piece.
            /// @details Original: TGdUtensil.HandleClick / HandleUtensilDrop on a Picture Frame (62/66).
            void clicked(int furnIndex, int itemId);

            /// @brief Handles the server's portrait packet: sub 0 shows the form as it is, sub 1 adds the second person.
            /// @param payload Packet payload.
            /// @details Original: RecvPortrait (62/62); sub 1 carries u8 body type, u8 head, i32 colour 1, i32 colour 5.
            void receive(const core::Bytes& payload);

            /// @brief Closes the form when the player is out of the house.
            void update();

        private:
            static constexpr int kMiddle = 113; // default x offset of a figure in the picture frame, px

            /// @brief One figure standing in the frame.
            struct Figure {
                bool shown = false;
                CharacterLook look;
                int offsetX = kMiddle;
            };

            /// @brief Removes both figures.
            /// @details Original: ClearPortraitModels.
            void clear();

            /// @brief Shows the house's owner by himself in the frame the piece makes.
            /// @details Original: LoadPortraitOwner.
            void loadOwner();

            /// @brief Selects the frame picture and its background.
            /// @param frame Frame 1..4.
            /// @details Original: SetPortraitFrame.
            void setFrame(int frame);

            /// @brief Puts a person into the left or right figure by body type (1 and 3 left, 2 and 4 right).
            /// @param bodyType Body type of the person.
            /// @param look Look of the person.
            /// @return False for any other body type.
            bool place(int bodyType, const CharacterLook& look);

            /// @brief Draws the right figure, then the left one, then the frame's own picture over them.
            /// @param canvas Target canvas.
            /// @param at Top-left of the drawing area.
            /// @details Original: DrawControl.
            void paint(gfx::Canvas& canvas, core::Point at);

            GameServices& m_services;
            ui::Screen& m_screen;
            Hooks m_hooks;
            ui::Window& m_window;
            int m_frame = 0;
            int m_furnIndex = 0;
            Figure m_left;
            Figure m_right;
    };

}

#endif
