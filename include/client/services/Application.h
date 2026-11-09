#ifndef NORTH_ISLAND_CLIENT_SERVICES_APPLICATION_H
#define NORTH_ISLAND_CLIENT_SERVICES_APPLICATION_H

#include "client/services/LaunchOptions.h"

namespace client {

    /// @brief The program: opens the window, sound and network, loads the font and runs the scenes until the player quits.
    class Application {
        public:
            /// @brief Creates the application from the parsed command line.
            /// @param options The launch options.
            explicit Application(
                LaunchOptions options
            )
                : m_options(std::move(options)) {}

            /// @brief Runs the program (loading scene, then login) until the player quits.
            /// @return The process exit code.
            int run();

        private:
            LaunchOptions m_options;
    };

}

#endif
