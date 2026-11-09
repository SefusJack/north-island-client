#ifndef NORTH_ISLAND_CLIENT_SERVICES_INPUT_INPUT_TRANSLATOR_H
#define NORTH_ISLAND_CLIENT_SERVICES_INPUT_INPUT_TRANSLATOR_H

#include "ui/Screen.h"

#include <SDL3/SDL.h>

namespace client {

    /// @brief Hands one SDL input event to a screen (its mouse already in logical coordinates: SDL_ConvertEventToRenderCoordinates).
    /// @param event The SDL event.
    /// @param screen Screen that receives the mouse, key and text input.
    /// @param origin Where the game's 800 x 600 screen lies in the window (the mouse is handed on in its coordinates).
    /// @return False for the quit request, true otherwise.
    bool forwardEvent(const SDL_Event& event, ui::Screen& screen, core::Point origin = {});

}

#endif
