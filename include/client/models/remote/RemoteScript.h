#ifndef NORTH_ISLAND_CLIENT_MODELS_REMOTE_REMOTE_SCRIPT_H
#define NORTH_ISLAND_CLIENT_MODELS_REMOTE_REMOTE_SCRIPT_H

#include "client/models/remote/RemoteBattleView.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>

struct lua_State;
struct lua_Debug;

namespace client {

    /// @brief The Electron Control's battle script: a Lua file of the player's whose decide(b) picks the skill or item and the target.
    /// @details Not in the original game. The script runs sandboxed (only the base, table, string and math libraries,
    /// no file access), with a cap on its memory and on the instructions of each call. It keeps a table `memory` for the whole battle.
    /// See docs/ELECTRON_CONTROL_SCRIPTS.md for the API.
    class RemoteScript {
        public:
            static constexpr std::size_t kMostBytes = 16 * 1024 * 1024; // memory the script may hold
            static constexpr int kMostInstructions = 2000000; // Lua instructions per load or per decide

            /// @brief Creates an empty host (no script loaded).
            /// @param print Where the script's print() goes.
            explicit RemoteScript(std::function<void(const std::string&)> print);
            ~RemoteScript();
            RemoteScript(const RemoteScript&) = delete;
            RemoteScript& operator=(const RemoteScript&) = delete;

            /// @brief Builds the path of a player's script file (sd<id xor 7654321>.lua, beside the Remote Control file).
            /// @param userRoot Directory holding the per-player files.
            /// @param playerId Id of the player.
            /// @return The file path.
            static std::filesystem::path pathFor(const std::filesystem::path& userRoot, std::int32_t playerId);

            /// @brief Builds the path of the file that holds a player's chosen script path (sd<id xor 7654321>.script, one line).
            /// @param userRoot Directory holding the per-player files.
            /// @param playerId Id of the player.
            /// @return The file path.
            static std::filesystem::path choiceFor(const std::filesystem::path& userRoot, std::int32_t playerId);

            /// @brief Loads and runs a script file, replacing the one loaded.
            /// @param path The file.
            /// @return True when it loaded and defines decide; false for no file (error() empty) or a failure (error() says why).
            bool load(const std::filesystem::path& path);

            /// @brief Loads and runs a script from text, replacing the one loaded.
            /// @param source The Lua source.
            /// @param name Chunk name shown in errors.
            /// @return True when it loaded and defines decide.
            bool loadSource(const std::string& source, const std::string& name);

            /// @brief Drops the loaded script.
            void unload();

            /// @brief Whether a script is loaded.
            /// @return True when decide can run.
            bool loaded() const { return m_state != nullptr; }

            /// @brief The last load or run error.
            /// @return The message, empty when none.
            const std::string& error() const { return m_error; }

            /// @brief Runs decide(b) for the commanding fighter.
            /// @param view The battle.
            /// @return The choice; nothing when the script returned nil or failed (error() then says why).
            std::optional<RemoteDecision> decide(const RemoteBattleView& view);

        private:
            static void* allocate(void* self, void* block, std::size_t oldSize, std::size_t newSize);
            static void countInstructions(lua_State* state, lua_Debug* debug);

            /// @brief Runs a C function in protected mode under the instruction cap.
            /// @param function The function; it gets the argument as light userdata.
            /// @param argument What it works on.
            /// @return True when it ran; else error() says why. The stack is left as it was.
            bool protect(int (*function)(lua_State*), void* argument);

            std::function<void(const std::string&)> m_print;
            lua_State* m_state = nullptr;
            std::size_t m_bytes = 0;
            int m_instructions = 0;
            std::uint32_t m_battle = 0;
            bool m_battleSeen = false;
            std::string m_error;
    };

}

#endif
