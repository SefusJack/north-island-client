#ifndef NORTH_ISLAND_CLIENT_MODELS_EVENTS_ACTIVITY_SCHEDULE_H
#define NORTH_ISLAND_CLIENT_MODELS_EVENTS_ACTIVITY_SCHEDULE_H

#include "protocol/ActivityLists.h"

#include <cstdint>
#include <filesystem>
#include <vector>

namespace client {

    /// @brief The events of the schedule: up to 30, numbered from 1.
    /// @details Original: TActionListForm's FActions with FWeekDay and FVersion. The game keeps them in User\ActionInfo.dat between sessions (a record of the version, then one for each event) and asks the server for the schedule at every login with the version it holds.
    class ActivitySchedule {
        public:
            static constexpr int kMost = 30; // most events kept in the schedule

            /// @brief The day the states are of.
            /// @details Original: FWeekDay.
            /// @return 1 Monday .. 7 Sunday, 0 until the server said.
            int weekDay() const { return m_weekDay; }

            /// @brief The version of the schedule held.
            /// @return The version.
            std::int32_t version() const { return m_version; }

            /// @brief The number of events.
            /// @return Event count.
            int count() const { return static_cast<int>(m_activities.size()); }

            /// @brief An event by number.
            /// @param number Number 1..count.
            /// @return The event record.
            const protocol::ActivityRecord& at(int number) const { return m_activities[static_cast<std::size_t>(number - 1)]; }

            /// @brief Replaces the schedule with the one the server sent.
            /// @details Original: InitActions and RecvActionList.
            /// @param list The schedule.
            void replace(const protocol::ActivityList& list);

            /// @brief Applies a state message; a new day sets every state back to preparing.
            /// @details Original: RecvActionState. The game looks at the last number named, not at all of them, to start the button's blinking.
            /// @param states The state message.
            /// @return False when the state of the last number named is not 2.
            bool applyStates(const protocol::ActivityStates& states);

            /// @brief Sets every state back to preparing.
            /// @details Original: RefreshActionList.
            void resetStates();

            /// @brief The numbers of the events the page lists: not over, and held on every day or on this weekday.
            /// @details Original: RefreshPage.
            /// @return Event numbers.
            std::vector<int> listed() const;

            /// @brief Loads the schedule from User\ActionInfo.dat.
            /// @details Original: LoadActionInfo.
            /// @param file The file path.
            void load(const std::filesystem::path& file);

            /// @brief Saves the schedule to User\ActionInfo.dat.
            /// @details Original: the write of RecvActionList.
            /// @param file The file path.
            void save(const std::filesystem::path& file) const;

        private:
            std::vector<protocol::ActivityRecord> m_activities;
            int m_weekDay = 0;
            std::int32_t m_version = 0;
    };

}

#endif
