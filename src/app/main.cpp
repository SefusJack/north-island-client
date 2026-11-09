#include "client/services/Application.h"
#include "client/services/LaunchOptions.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

/// @brief Program entry point: parses the launch options and runs the client application.
/// @param argc Number of command line arguments.
/// @param argv Command line arguments (program name first).
/// @return Exit code of the application, or 2 when the launch options are invalid.
int main(int argc, char** argv) {
    SDL_SetMainReady();
    std::vector<std::string> arguments(argv + 1, argv + argc);

    std::filesystem::path programDirectory;
    if (const char* basePath = SDL_GetBasePath()) {
        programDirectory = std::filesystem::u8path(basePath);
    }

    std::string error;
    auto options = client::LaunchOptions::parse(arguments, programDirectory, error);
    if (!options) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Wonderland Online", error.c_str(), nullptr);
        std::cerr
            << error
            << "\nusage: north-island-client [--data GAME_FOLDER] [--user FOLDER] [--software] [--renderer NAME] [--perf] [--perf-log FILE] [--fps unlimited|vsync|N] [--window WxH|last]\n";
        return 2; // exit code for invalid command line
    }
    client::Application application(std::move(*options));
    return application.run();
}
