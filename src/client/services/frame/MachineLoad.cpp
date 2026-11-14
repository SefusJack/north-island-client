#include "client/services/frame/MachineLoad.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <map>
#include <optional>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <pdh.h>
#include <pdhmsg.h>
#include <psapi.h>
#elif defined(__linux__)
#include <fstream>

#include <unistd.h>
#endif

namespace client {

    namespace {

        constexpr double kCpuEveryMs = 500.0; // how often the processor time is read
        constexpr int kGpuEveryMs = 1000; // how often the graphics card counters are read
        constexpr int kNapMs = 50; // the graphics card thread's sleep step, so it stops soon when asked

        /// @brief Reads the processor time the process has used so far (all its threads).
        /// @return Milliseconds.
        double processTimeMs() {
#ifdef _WIN32
            FILETIME made{};
            FILETIME gone{};
            FILETIME kernel{};
            FILETIME user{};
            if (GetProcessTimes(GetCurrentProcess(), &made, &gone, &kernel, &user) == 0) {
                return 0.0;
            }
            const auto ticks = [](const FILETIME& time) { return (static_cast<unsigned long long>(time.dwHighDateTime) << 32) | time.dwLowDateTime; };
            return static_cast<double>(ticks(kernel) + ticks(user)) / 10000.0; // a tick is 100 ns
#else
            return static_cast<double>(std::clock()) * 1000.0 / CLOCKS_PER_SEC;
#endif
        }

        /// @brief Reads how much memory the process holds in RAM (its working set; on Linux its resident set).
        /// @return MB, or nothing where it cannot be read.
        std::optional<double> processRamMb() {
            constexpr double kBytesPerMb = 1024.0 * 1024.0;
#ifdef _WIN32
            // the function is in kernel32 from Windows 7 on and in psapi.dll before; asked for by name so one build runs on both
            using GetMemoryInfo = BOOL(WINAPI*)(HANDLE, PROCESS_MEMORY_COUNTERS*, DWORD);
            static const GetMemoryInfo getMemoryInfo = [] {
                auto found = reinterpret_cast<GetMemoryInfo>(GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "K32GetProcessMemoryInfo"));
                if (found == nullptr) {
                    if (const HMODULE psapi = LoadLibraryW(L"psapi.dll")) {
                        found = reinterpret_cast<GetMemoryInfo>(GetProcAddress(psapi, "GetProcessMemoryInfo"));
                    }
                }
                return found;
            }();
            PROCESS_MEMORY_COUNTERS counters{};
            counters.cb = sizeof(counters);
            if (getMemoryInfo != nullptr && getMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)) != 0) {
                return static_cast<double>(counters.WorkingSetSize) / kBytesPerMb;
            }
#elif defined(__linux__)
            // /proc/self/statm: total pages, then the pages in RAM
            std::ifstream statm("/proc/self/statm");
            long long total = 0;
            long long resident = 0;
            if (statm >> total >> resident) {
                return static_cast<double>(resident) * static_cast<double>(sysconf(_SC_PAGESIZE)) / kBytesPerMb;
            }
#endif
            return std::nullopt;
        }

    }

    MachineLoad::MachineLoad() { m_reading.cores = static_cast<int>(std::max(1u, std::thread::hardware_concurrency())); }

    MachineLoad::~MachineLoad() {
        m_stop = true;
        if (m_gpuThread.joinable()) {
            m_gpuThread.join();
        }
    }

    void MachineLoad::setWanted(bool wanted) {
        if (wanted == m_wanted) {
            return;
        }
        m_wanted = wanted;
        m_cpuStarted = false; // the next reading starts a new stretch
        if (!wanted) {
            m_reading.cpuPercent.reset();
            m_reading.corePercent.reset();
            m_reading.ramMb.reset();
            return;
        }
#ifdef _WIN32
        if (!m_gpuThread.joinable()) {
            m_gpuThread = std::thread([this] { watchGpu(); });
        }
#endif
    }

    void MachineLoad::update(double nowMs) {
        if (!m_cpuStarted) {
            m_cpuStarted = true;
            m_cpuAtMs = nowMs;
            m_cpuTimeMs = processTimeMs();
            m_reading.ramMb = processRamMb();
            return;
        }
        if (nowMs - m_cpuAtMs < kCpuEveryMs) {
            return;
        }
        const double usedMs = processTimeMs();
        const double share = (usedMs - m_cpuTimeMs) / (nowMs - m_cpuAtMs); // processor cores kept busy
        m_reading.corePercent = share * 100.0;
        m_reading.cpuPercent = share * 100.0 / m_reading.cores;
        m_reading.ramMb = processRamMb();
        m_cpuAtMs = nowMs;
        m_cpuTimeMs = usedMs;
    }

    LoadReading MachineLoad::reading() const {
        LoadReading reading = m_reading;
        const int hundredths = m_gpuHundredths;
        if (m_wanted && hundredths >= 0) {
            reading.gpuPercent = hundredths / 100.0;
        }
        return reading;
    }

    void MachineLoad::watchGpu() {
#ifdef _WIN32
        // pdh.dll is loaded by hand: the counters' English names can be asked for from Windows Vista on only, and the
        // Windows XP build must not need a function Windows XP does not have
        using OpenQuery = PDH_STATUS(WINAPI*)(LPCWSTR, DWORD_PTR, PDH_HQUERY*);
        using AddCounter = PDH_STATUS(WINAPI*)(PDH_HQUERY, LPCWSTR, DWORD_PTR, PDH_HCOUNTER*);
        using Collect = PDH_STATUS(WINAPI*)(PDH_HQUERY);
        using GetArray = PDH_STATUS(WINAPI*)(PDH_HCOUNTER, DWORD, LPDWORD, LPDWORD, PPDH_FMT_COUNTERVALUE_ITEM_W);
        using CloseQuery = PDH_STATUS(WINAPI*)(PDH_HQUERY);
        const HMODULE pdh = LoadLibraryW(L"pdh.dll");
        if (pdh == nullptr) {
            return;
        }
        const auto openQuery = reinterpret_cast<OpenQuery>(GetProcAddress(pdh, "PdhOpenQueryW"));
        const auto addCounter = reinterpret_cast<AddCounter>(GetProcAddress(pdh, "PdhAddEnglishCounterW"));
        const auto collect = reinterpret_cast<Collect>(GetProcAddress(pdh, "PdhCollectQueryData"));
        const auto getArray = reinterpret_cast<GetArray>(GetProcAddress(pdh, "PdhGetFormattedCounterArrayW"));
        const auto closeQuery = reinterpret_cast<CloseQuery>(GetProcAddress(pdh, "PdhCloseQuery"));
        PDH_HQUERY query = nullptr;
        PDH_HCOUNTER counter = nullptr;
        // one counter per process and graphics card engine ("pid_1234_luid_..._engtype_3D"), before Windows 10 none
        const bool ready = openQuery != nullptr && addCounter != nullptr && collect != nullptr && getArray != nullptr && closeQuery != nullptr &&
                           openQuery(nullptr, 0, &query) == ERROR_SUCCESS &&
                           addCounter(query, L"\\GPU Engine(*)\\Utilization Percentage", 0, &counter) == ERROR_SUCCESS;
        if (ready) {
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
            const std::wstring own = L"pid_" + std::to_wstring(GetCurrentProcessId()) + L"_";
            const std::wstring typeMark = L"engtype_";
            std::vector<unsigned char> buffer;
            bool collecting = false; // a rate needs two readings: the first after a pause only starts the stretch
            int sleptMs = 0;
            while (!m_stop) {
                Sleep(kNapMs);
                sleptMs += kNapMs;
                if (!m_wanted) {
                    collecting = false;
                    m_gpuHundredths = -1;
                    continue;
                }
                if (collecting && sleptMs < kGpuEveryMs) {
                    continue;
                }
                sleptMs = 0;
                if (collect(query) != ERROR_SUCCESS) {
                    continue;
                }
                if (!collecting) {
                    collecting = true;
                    continue;
                }
                DWORD bytes = 0;
                DWORD count = 0;
                if (getArray(counter, PDH_FMT_DOUBLE, &bytes, &count, nullptr) != static_cast<PDH_STATUS>(PDH_MORE_DATA)) {
                    continue;
                }
                buffer.resize(bytes);
                auto* items = reinterpret_cast<PPDH_FMT_COUNTERVALUE_ITEM_W>(buffer.data());
                if (getArray(counter, PDH_FMT_DOUBLE, &bytes, &count, items) != ERROR_SUCCESS) {
                    continue;
                }
                // the client's share of each kind of engine (3D, Copy, ...); the busiest kind is the number shown
                std::map<std::wstring, double> byType;
                for (DWORD i = 0; i < count; ++i) {
                    const std::wstring name = items[i].szName != nullptr ? items[i].szName : L"";
                    const std::size_t type = name.find(typeMark);
                    if (name.compare(0, own.size(), own) == 0 && type != std::wstring::npos && items[i].FmtValue.CStatus == ERROR_SUCCESS) {
                        byType[name.substr(type + typeMark.size())] += items[i].FmtValue.doubleValue;
                    }
                }
                double busiest = 0.0;
                for (const auto& entry : byType) {
                    busiest = std::max(busiest, entry.second);
                }
                m_gpuHundredths = static_cast<int>(std::min(busiest, 100.0) * 100.0);
            }
        }
        if (query != nullptr && closeQuery != nullptr) {
            closeQuery(query);
        }
        FreeLibrary(pdh);
#endif
    }

}
