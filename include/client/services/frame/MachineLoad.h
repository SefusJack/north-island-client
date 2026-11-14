#ifndef NORTH_ISLAND_CLIENT_SERVICES_FRAME_MACHINE_LOAD_H
#define NORTH_ISLAND_CLIENT_SERVICES_FRAME_MACHINE_LOAD_H

#include <atomic>
#include <optional>
#include <thread>

namespace client {

    /// @brief How much of the processor and the graphics card the client is using (the performance overlay's "use" numbers).
    struct LoadReading {
        std::optional<double> cpuPercent; // of all processor cores together, as the Task Manager counts
        std::optional<double> corePercent; // of one core (100 is one core kept busy)
        std::optional<double> gpuPercent; // of the graphics card, the client's own share; nothing where the system does not tell
        std::optional<double> ramMb; // memory the client holds in RAM (its working set), MB; nothing where the system does not tell
        int cores = 1; // logical processor cores
    };

    /// @brief Measures the client's own processor and graphics card use while it is wanted.
    /// @details The processor share is the process's processor time over the time that passed, read twice a second, and
    /// the memory in RAM is read with it. The
    /// graphics card share comes from Windows' "GPU Engine" counters (Windows 10 and later), read once a second on a thread
    /// of its own because reading them takes a few ms; elsewhere there is none.
    class MachineLoad {
        public:
            MachineLoad();
            ~MachineLoad();
            MachineLoad(const MachineLoad&) = delete;
            MachineLoad& operator=(const MachineLoad&) = delete;

            /// @brief Starts or pauses the measuring (the graphics card's thread is started the first time).
            /// @param wanted True while the numbers are shown.
            void setWanted(bool wanted);

            /// @brief Takes a new processor and memory reading when the last one is half a second old; call every frame while wanted.
            /// @param nowMs The time in core::nowPreciseMs.
            void update(double nowMs);

            /// @brief The latest readings.
            /// @return The readings (nothing in a number that has not been measured yet or cannot be).
            LoadReading reading() const;

        private:
            /// @brief The graphics card thread's work: reads the counters once a second while wanted.
            void watchGpu();

            LoadReading m_reading;
            double m_cpuAtMs = 0; // when the processor time was last read
            double m_cpuTimeMs = 0; // the process's processor time then
            bool m_cpuStarted = false;
            std::thread m_gpuThread;
            std::atomic<bool> m_wanted{false};
            std::atomic<bool> m_stop{false};
            std::atomic<int> m_gpuHundredths{-1}; // the graphics card share in hundredths of a percent, -1: not known
    };

}

#endif
