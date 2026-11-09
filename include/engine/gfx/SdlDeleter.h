#ifndef NORTH_ISLAND_GFX_SDL_DELETER_H
#define NORTH_ISLAND_GFX_SDL_DELETER_H

#include <SDL3/SDL.h>

#include <memory>

namespace gfx {

    /// @brief Deleter that releases SDL objects so they can live in std::unique_ptr.
    struct SdlDeleter {
        /// @brief Destroys a window.
        /// @param window The window.
        void operator()(SDL_Window* window) const { SDL_DestroyWindow(window); }

        /// @brief Destroys a renderer.
        /// @param renderer The renderer.
        void operator()(SDL_Renderer* renderer) const { SDL_DestroyRenderer(renderer); }

        /// @brief Destroys a texture.
        /// @param texture The texture.
        void operator()(SDL_Texture* texture) const { SDL_DestroyTexture(texture); }

        /// @brief Destroys a surface.
        /// @param surface The surface.
        void operator()(SDL_Surface* surface) const { SDL_DestroySurface(surface); }

        /// @brief Destroys a cursor.
        /// @param cursor The cursor.
        void operator()(SDL_Cursor* cursor) const { SDL_DestroyCursor(cursor); }
    };

    template <typename T>
    using SdlHandle = std::unique_ptr<T, SdlDeleter>;

}

#endif
