#include "gfx/D3d9GpuTimer.h"

#ifdef _WIN32
#include <d3d9.h>

#include <SDL3/SDL_render.h>
#endif

namespace gfx {

#ifdef _WIN32

    std::unique_ptr<GpuTimer> D3d9GpuTimer::create(SDL_Renderer* renderer) {
        auto* device =
            static_cast<IDirect3DDevice9*>(SDL_GetPointerProperty(SDL_GetRendererProperties(renderer), SDL_PROP_RENDERER_D3D9_DEVICE_POINTER, nullptr));
        if (device == nullptr) {
            return nullptr;
        }
        device->AddRef();
        std::unique_ptr<D3d9GpuTimer> timer(new D3d9GpuTimer(renderer, device));
        if (!timer->createQueries()) {
            return nullptr;
        }
        return timer;
    }

    D3d9GpuTimer::~D3d9GpuTimer() {
        for (Slot& slot : m_slots) {
            for (IDirect3DQuery9* query : {slot.disjoint, slot.frequency, slot.start, slot.end}) {
                if (query != nullptr) {
                    query->Release();
                }
            }
        }
        m_device->Release();
    }

    bool D3d9GpuTimer::createQueries() {
        for (Slot& slot : m_slots) {
            if (FAILED(m_device->CreateQuery(D3DQUERYTYPE_TIMESTAMPDISJOINT, &slot.disjoint)) ||
                FAILED(m_device->CreateQuery(D3DQUERYTYPE_TIMESTAMPFREQ, &slot.frequency)) ||
                FAILED(m_device->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &slot.start)) || FAILED(m_device->CreateQuery(D3DQUERYTYPE_TIMESTAMP, &slot.end))) {
                return false;
            }
        }
        return true;
    }

    void D3d9GpuTimer::collect() {
        for (int i = 0; i < kSlots; ++i) {
            Slot& slot = m_slots[(m_next + i) % kSlots];
            if (!slot.pending) {
                continue;
            }
            BOOL disjoint = FALSE;
            UINT64 frequency = 0;
            UINT64 start = 0;
            UINT64 end = 0;
            const HRESULT result = slot.end->GetData(&end, sizeof end, 0);
            if (result == S_FALSE) {
                continue;
            }
            if (result != S_OK || slot.start->GetData(&start, sizeof start, 0) != S_OK || slot.frequency->GetData(&frequency, sizeof frequency, 0) != S_OK ||
                slot.disjoint->GetData(&disjoint, sizeof disjoint, 0) != S_OK) {
                slot.pending = false;
                continue;
            }
            slot.pending = false;
            if (!disjoint && frequency != 0 && end >= start) {
                m_latestMs = static_cast<double>(end - start) * 1000.0 / static_cast<double>(frequency); // 1000: seconds to milliseconds
            }
        }
    }

    void D3d9GpuTimer::beginFrame() {
        collect();
        Slot& slot = m_slots[m_next];
        m_active = !slot.pending;
        if (!m_active) {
            return;
        }
        SDL_FlushRenderer(m_renderer);
        slot.disjoint->Issue(D3DISSUE_BEGIN);
        slot.start->Issue(D3DISSUE_END);
    }

    void D3d9GpuTimer::endFrame() {
        if (!m_active) {
            return;
        }
        SDL_FlushRenderer(m_renderer);
        Slot& slot = m_slots[m_next];
        slot.end->Issue(D3DISSUE_END);
        slot.frequency->Issue(D3DISSUE_END);
        slot.disjoint->Issue(D3DISSUE_END);
        slot.pending = true;
        m_next = (m_next + 1) % kSlots;
        m_active = false;
    }

#else

    std::unique_ptr<GpuTimer> D3d9GpuTimer::create(SDL_Renderer*) { return nullptr; }

    D3d9GpuTimer::~D3d9GpuTimer() = default;

    bool D3d9GpuTimer::createQueries() { return false; }

    void D3d9GpuTimer::collect() {}

    void D3d9GpuTimer::beginFrame() {}

    void D3d9GpuTimer::endFrame() {}

#endif

}
