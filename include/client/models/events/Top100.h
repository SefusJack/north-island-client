#ifndef NORTH_ISLAND_CLIENT_MODELS_EVENTS_TOP100_H
#define NORTH_ISLAND_CLIENT_MODELS_EVENTS_TOP100_H

#include "core/Bytes.h"

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace client {

    /// @brief Model of the Top 100 window: the ranking lists received, the tab and page shown and what is asked for.
    /// @details Original: TLD_Top100Form without its window; the level and money lists page by 15 up to 300, the Q&A and PK
    /// lists by 15, the two event lists by 13.
    class Top100 {
        public:
            /// @brief The list shown (also the kind byte of the request and of the 1/x list message).
            /// @details Original: FRankKind.
            enum class Kind { None = 0, Level = 1, Money = 2, Quiz = 3, Pk = 4, EventFirst = 5, EventSecond = 6 };

            /// @brief A place of the level list.
            struct LevelRank {
                int job = 0;
                int level = 0;
                std::string name;
            };

            /// @brief A place of the PK list or of an event list; the event lists name the server before the player.
            struct Place {
                int rank = 0;
                std::string name;
                std::string server;
            };

            /// @brief Callbacks the window model needs.
            struct Hooks {
                std::function<void(const core::Bytes&)> send;
                std::function<std::string(int)> serverGroupName;
            };

            static constexpr int kRowsPerPage = 15; // ranking rows on one page (level, money, quiz, PK lists)
            static constexpr int kEventRowsPerPage = 13; // ranking rows on one page of the two event lists
            static constexpr int kMostRanks = 300; // most places kept for the level and money lists

            /// @brief Creates the model.
            /// @param hooks Send and server-name callbacks.
            explicit Top100(
                Hooks hooks
            );

            /// @brief The list shown.
            /// @return Current kind.
            Kind kind() const { return m_kind; }

            /// @brief The page shown (from 1).
            /// @return Page number.
            int page() const { return m_page; }

            /// @brief The event tab shown.
            /// @return 1 or 2.
            int eventTab() const { return m_eventTab; }

            /// @brief Size of the level list.
            /// @return Number of places.
            int levelCount() const { return m_levelCount; }

            /// @brief Size of the money list.
            /// @return Number of places.
            int moneyCount() const { return m_moneyCount; }

            /// @brief Size of the Q&A list.
            /// @return Number of places.
            int quizCount() const { return m_quizCount; }

            /// @brief Size of the PK list.
            /// @return Number of places.
            int pkCount() const { return m_pkCount; }

            /// @brief Size of an event list.
            /// @param tab Event tab (1 or 2).
            /// @return Number of places.
            int eventCount(int tab) const { return tab == 1 ? m_eventCounts[0] : m_eventCounts[1]; }

            /// @brief How many pages the list of the tab shown has.
            /// @return Page count.
            int pageCount() const;

            /// @brief How many rows a page of the tab shown has.
            /// @return 13 for the event lists, 15 otherwise.
            int rowsPerPage() const;

            /// @brief Rank of the own player in the level list.
            /// @return Place, 0 when unknown.
            int myLevelRank() const { return m_myLevelRank; }

            /// @brief Rank of the own player in the money list.
            /// @return Place, 0 when unknown.
            int myMoneyRank() const { return m_myMoneyRank; }

            /// @brief Header text sent with the first level page.
            /// @return The header.
            const std::string& header() const { return m_header; }

            /// @brief A place of the level list.
            /// @param place Place from 1.
            /// @return The place.
            const LevelRank& level(int place) const { return m_levels[static_cast<std::size_t>(place)]; }

            /// @brief A place of the money list.
            /// @param place Place from 1.
            /// @return The player name.
            const std::string& money(int place) const { return m_money[static_cast<std::size_t>(place)]; }

            /// @brief A place of the Q&A list.
            /// @param place Place from 1.
            /// @return The player name.
            const std::string& quiz(int place) const { return m_quiz[static_cast<std::size_t>(place)]; }

            /// @brief A place of the PK list.
            /// @param place Place from 1.
            /// @return The place.
            const Place& pk(int place) const { return m_pk[static_cast<std::size_t>(place)]; }

            /// @brief A place of an event list.
            /// @param tab Event tab (1 or 2).
            /// @param place Place from 1.
            /// @return The place.
            const Place& event(int tab, int place) const;

            /// @brief Opens the window: asks for the list sizes when unknown, shows the level tab otherwise.
            /// @details Original: ShowCtrl.
            void open();

            /// @brief Shows the level tab, asking for the page when it is missing.
            /// @details Original: LevelTabClick.
            void showLevel();

            /// @brief Shows the money tab, asking for the page when it is missing.
            /// @details Original: GoldTabClick.
            void showMoney();

            /// @brief Shows the Q&A tab, asking for the page when it is missing.
            /// @details Original: QaTabClick.
            void showQuiz();

            /// @brief Shows the PK tab, asking for the list when it is missing.
            /// @details Original: PkTabClick.
            void showPk();

            /// @brief Shows the event tab that was shown last.
            /// @details Original: DEventTabClick.
            void showEvents();

            /// @brief Shows an event tab, asking for its list when it is missing.
            /// @details Original: ShowDEventPage.
            /// @param tab Event tab (1 or 2; anything else is the first).
            void showEventTab(int tab);

            /// @brief Turns to the previous page.
            /// @details Original: PrevPageClick.
            void previousPage();

            /// @brief Turns to the next page, asking for it when it is missing.
            /// @details Original: NextPageClick.
            void nextPage();

            /// @brief Stores a list message from the server.
            /// @details Original: RecvTop100List (the message with its two leading bytes).
            /// @param payload Whole message; kind and page bytes follow the two leading bytes.
            void receive(const core::Bytes& payload);

        private:
            /// @brief Sends the request for the current kind and page.
            void request() { m_hooks.send(request(static_cast<int>(m_kind), m_page)); }

            /// @brief Builds a list request.
            /// @param kind List kind.
            /// @param page Page wanted.
            /// @return The request message.
            static core::Bytes request(int kind, int page);

            /// @brief Size of the list shown.
            /// @return Number of places, 0 for none.
            int currentCount() const;

            /// @brief Whether a place of the list shown has not been received.
            /// @param place Place from 1.
            /// @return True when empty.
            bool placeEmpty(int place) const;

            /// @brief Whether a place of the page asked for is missing.
            /// @return True when something is missing.
            bool pageMissing() const;

            Hooks m_hooks;
            Kind m_kind = Kind::None;
            int m_page = 0;
            int m_eventTab = 1;
            int m_refreshKind = 0;
            int m_levelCount = 0;
            int m_moneyCount = 0;
            int m_quizCount = 0;
            int m_pkCount = 0;
            std::array<int, 2> m_eventCounts{};
            int m_myLevelRank = 0;
            int m_myMoneyRank = 0;
            std::string m_header;
            std::array<LevelRank, kMostRanks + 1> m_levels;
            std::array<std::string, kMostRanks + 1> m_money;
            std::array<std::string, 21> m_quiz; // 20 Q&A places, index 0 unused
            std::vector<Place> m_pk = std::vector<Place>(2);
            std::array<std::vector<Place>, 2> m_events{std::vector<Place>(2), std::vector<Place>(2)};
    };

}

#endif
