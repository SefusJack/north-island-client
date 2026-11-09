#include "gfx/D3d11GpuTimer.h"

#ifdef _WIN32
#include <d3d11.h>

#include <SDL3/SDL_render.h>
#endif

namespace gfx {

#ifdef _WIN32

    std::unique_ptr<GpuTimer> D3d11GpuTimer::create(SDL_Renderer* renderer) {
        auto* device = static_cast<ID3D11Device*>(SDL_GetPointerProperty(SDL_GetRendererProperties(renderer), SDL_PROP_RENDERER_D3D11_DEVICE_POINTER, nullptr));
        if (device == nullptr) {
            return nullptr;
        }
        device->AddRef();
        ID3D11DeviceContext* context = nullptr;
        device->GetImmediateContext(&context);
        std::unique_ptr<D3d11GpuTimer> timer(new D3d11GpuTimer(renderer, device, context));
        if (context == nullptr || !timer->createQueries()) {
            return nullptr;
        }
        return timer;
    }

    D3d11GpuTimer::~D3d11GpuTimer() {
        for (Slot& slot : m_slots) {
            for (ID3D11Query* query : {slot.disjoint, slot.start, slot.end}) {
                if (query != nullptr) {
                    query->Release();
                }
            }
        }
        if (m_context != nullptr) {
            m_context->Release();
        }
        m_device->Release();
    }

    bool D3d11GpuTimer::createQueries() {
        D3D11_QUERY_DESC disjoint{D3D11_QUERY_TIMESTAMP_DISJOINT, 0};
        D3D11_QUERY_DESC stamp{D3D11_QUERY_TIMESTAMP, 0};
        for (Slot& slot : m_slots) {
            if (FAILED(m_device->CreateQuery(&disjoint, &slot.disjoint)) || FAILED(m_device->CreateQuery(&stamp, &slot.start)) ||
                FAILED(m_device->CreateQuery(&stamp, &slot.end))) {
                return false;
            }
        }
        return true;
    }

    void D3d11GpuTimer::collect() {
        for (int i = 0; i < kSlots; ++i) {
            Slot& slot = m_slots[(m_next + i) % kSlots];
            if (!slot.pending) {
                continue;
            }
            D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint{};
            const HRESULT result = m_context->GetData(slot.disjoint, &disjoint, sizeof disjoint, D3D11_ASYNC_GETDATA_DONOTFLUSH);
            if (result == S_FALSE) {
                continue;
            }
            UINT64 start = 0;
            UINT64 end = 0;
            slot.pending = false;
            if (result != S_OK || m_context->GetData(slot.start, &start, sizeof start, D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK ||
                m_context->GetData(slot.end, &end, sizeof end, D3D11_ASYNC_GETDATA_DONOTFLUSH) != S_OK) {
                continue;
            }
            if (!disjoint.Disjoint && disjoint.Frequency != 0 && end >= start) {
                m_latestMs = static_cast<double>(end - start) * 1000.0 / static_cast<double>(disjoint.Frequency); // 1000: seconds to milliseconds
            }
        }
    }

    void D3d11GpuTimer::beginFrame() {
        collect();
        Slot& slot = m_slots[m_next];
        m_active = !slot.pending;
        if (!m_active) {
            return;
        }
        SDL_FlushRenderer(m_renderer);
        m_context->Begin(slot.disjoint);
        m_context->End(slot.start);
    }

    void D3d11GpuTimer::endFrame() {
        if (!m_active) {
            return;
        }
        SDL_FlushRenderer(m_renderer);
        Slot& slot = m_slots[m_next];
        m_context->End(slot.end);
        m_context->End(slot.disjoint);
        slot.pending = true;
        m_next = (m_next + 1) % kSlots;
        m_active = false;
    }

#else

    std::unique_ptr<GpuTimer> D3d11GpuTimer::create(SDL_Renderer*) { return nullptr; }

    D3d11GpuTimer::~D3d11GpuTimer() = default;

    bool D3d11GpuTimer::createQueries() { return false; }

    void D3d11GpuTimer::collect() {}

    void D3d11GpuTimer::beginFrame() {}

    void D3d11GpuTimer::endFrame() {}

#endif

}
