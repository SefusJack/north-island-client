#include "gfx/GpuTimer.h"

#include "gfx/D3d11GpuTimer.h"
#include "gfx/D3d9GpuTimer.h"
#include "gfx/GlGpuTimer.h"

namespace gfx {

    std::unique_ptr<GpuTimer> GpuTimer::create(SDL_Renderer* renderer, const std::string& backendName) {
        if (backendName == "direct3d") {
            return D3d9GpuTimer::create(renderer);
        }
        if (backendName == "direct3d11") {
            return D3d11GpuTimer::create(renderer);
        }
        if (backendName == "opengl") {
            return GlGpuTimer::create(renderer);
        }
        return nullptr;
    }

}
