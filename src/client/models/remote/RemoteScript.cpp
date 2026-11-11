#include "client/models/remote/RemoteScript.h"

// Lua is built as C++ (see CMakeLists.txt), so its headers are included without extern "C" and its errors unwind as exceptions
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr std::int32_t kFileKey = 7654321; // player id xor this names the per-player files (as the Remote Control file)
        constexpr int kHookEvery = 1000; // instructions between two counts of the instruction cap

        // helpers the scripts get before their own code runs
        constexpr const char* kPrelude = R"lua(
function filter(list, keep)
  local kept = {}
  for _, f in ipairs(list) do
    if keep(f) then kept[#kept + 1] = f end
  end
  return kept
end
function alive(list) return filter(list, function(f) return f.alive end) end
function fallen(list) return filter(list, function(f) return not f.alive end) end
function lowest(list, key)
  local best
  for _, f in ipairs(list) do
    if best == nil or f[key] < best[key] then best = f end
  end
  return best
end
function highest(list, key)
  local best
  for _, f in ipairs(list) do
    if best == nil or f[key] > best[key] then best = f end
  end
  return best
end
function has_status(f, skill)
  for _, s in ipairs(f.statuses) do
    if s == skill then return true end
  end
  return false
end
)lua";

        /// @brief What a protected decide call works on.
        struct DecideCall {
            const RemoteBattleView* view = nullptr;
            std::optional<RemoteDecision> decision;
        };

        /// @brief What a protected load works on.
        struct LoadCall {
            const std::string* source = nullptr;
            const std::string* name = nullptr;
            const std::function<void(const std::string&)>* print = nullptr;
        };

        void setInteger(lua_State* state, const char* key, lua_Integer value) {
            lua_pushinteger(state, value);
            lua_setfield(state, -2, key);
        }

        /// @brief Reads a slot argument: a slot number or a fighter table.
        int slotArgument(lua_State* state, int index) {
            if (lua_istable(state, index)) {
                lua_getfield(state, index, "slot");
                const auto slot = static_cast<int>(luaL_checkinteger(state, -1));
                lua_pop(state, 1);
                return slot;
            }
            return static_cast<int>(luaL_checkinteger(state, index));
        }

        int print(lua_State* state) {
            const auto* out = static_cast<const std::function<void(const std::string&)>*>(lua_touserdata(state, lua_upvalueindex(1)));
            std::string line;
            const int count = lua_gettop(state);
            for (int i = 1; i <= count; ++i) {
                if (i > 1) {
                    line += '\t';
                }
                line += luaL_tolstring(state, i, nullptr);
                lua_pop(state, 1);
            }
            if (*out) {
                (*out)(line);
            }
            return 0;
        }

        int canUse(lua_State* state) {
            const auto* view = static_cast<const RemoteBattleView*>(lua_touserdata(state, lua_upvalueindex(1)));
            const auto skill = static_cast<int>(luaL_checkinteger(state, 1));
            const int slot = slotArgument(state, 2);
            lua_pushboolean(state, view->canUse && view->canUse(skill, slot));
            return 1;
        }

        int hasItem(lua_State* state) {
            const auto* view = static_cast<const RemoteBattleView*>(lua_touserdata(state, lua_upvalueindex(1)));
            const auto item = static_cast<int>(luaL_checkinteger(state, 1));
            lua_pushboolean(state, view->hasItem && view->hasItem(item));
            return 1;
        }

        void pushIntegers(lua_State* state, const std::vector<int>& values) {
            lua_createtable(state, static_cast<int>(values.size()), 0);
            for (std::size_t i = 0; i < values.size(); ++i) {
                lua_pushinteger(state, values[i]);
                lua_rawseti(state, -2, static_cast<lua_Integer>(i + 1));
            }
        }

        void pushFighter(lua_State* state, const RemoteFighter& fighter) {
            lua_createtable(state, 0, 20); // 20 fields
            setInteger(state, "slot", fighter.slot);
            setInteger(state, "side", fighter.side);
            setInteger(state, "row", fighter.row);
            setInteger(state, "col", fighter.col);
            setInteger(state, "kind", fighter.kind);
            setInteger(state, "id", fighter.id);
            setInteger(state, "owner", fighter.ownerId);
            lua_pushstring(state, fighter.name.c_str());
            lua_setfield(state, -2, "name");
            setInteger(state, "hp", fighter.hp);
            setInteger(state, "max_hp", fighter.maxHp);
            setInteger(state, "sp", fighter.sp);
            setInteger(state, "max_sp", fighter.maxSp);
            // percents of the most HP and SP, 0..100 (100 when the most is unknown)
            setInteger(state, "hp_pct", fighter.maxHp > 0 ? static_cast<lua_Integer>(fighter.hp) * 100 / fighter.maxHp : 100);
            setInteger(state, "sp_pct", fighter.maxSp > 0 ? static_cast<lua_Integer>(fighter.sp) * 100 / fighter.maxSp : 100);
            setInteger(state, "level", fighter.level);
            setInteger(state, "element", fighter.element);
            lua_pushboolean(state, fighter.hp > 0);
            lua_setfield(state, -2, "alive");
            pushIntegers(state, fighter.statusSkills);
            lua_setfield(state, -2, "statuses");
            pushIntegers(state, fighter.statusKinds);
            lua_setfield(state, -2, "status_kinds");
        }

        /// @brief Pushes the argument b of decide.
        void pushView(lua_State* state, const RemoteBattleView& view) {
            lua_createtable(state, 0, 14); // 14 fields
            setInteger(state, "battle", view.battle);
            setInteger(state, "round", view.round);
            lua_pushstring(state, view.actorKind == 2 ? "pet" : "player"); // actor kind 2: the pet
            lua_setfield(state, -2, "actor");
            setInteger(state, "ally_side", view.allySide);
            setInteger(state, "enemy_side", view.enemySide);
            const int b = lua_gettop(state);
            lua_createtable(state, static_cast<int>(view.fighters.size()), 0);
            const int all = lua_gettop(state);
            lua_newtable(state);
            const int allies = lua_gettop(state);
            lua_newtable(state);
            const int enemies = lua_gettop(state);
            lua_Integer allCount = 0;
            lua_Integer allyCount = 0;
            lua_Integer enemyCount = 0;
            for (const RemoteFighter& fighter : view.fighters) {
                pushFighter(state, fighter);
                const int table = lua_gettop(state);
                lua_pushvalue(state, table);
                lua_rawseti(state, all, ++allCount);
                lua_pushvalue(state, table);
                lua_rawseti(state, fighter.side == view.allySide ? allies : enemies, fighter.side == view.allySide ? ++allyCount : ++enemyCount);
                if (fighter.slot == view.actorSlot) {
                    lua_pushvalue(state, table);
                    lua_setfield(state, b, "self");
                }
                if (fighter.slot == view.ownSlot) {
                    lua_pushvalue(state, table);
                    lua_setfield(state, b, "player");
                }
                if (fighter.slot == view.petSlot) {
                    lua_pushvalue(state, table);
                    lua_setfield(state, b, "pet");
                }
                lua_pop(state, 1);
            }
            lua_setfield(state, b, "enemies");
            lua_setfield(state, b, "allies");
            lua_setfield(state, b, "fighters");
            lua_pushlightuserdata(state, const_cast<RemoteBattleView*>(&view));
            lua_pushcclosure(state, canUse, 1);
            lua_setfield(state, b, "can_use");
            lua_pushlightuserdata(state, const_cast<RemoteBattleView*>(&view));
            lua_pushcclosure(state, hasItem, 1);
            lua_setfield(state, b, "has_item");
            lua_getglobal(state, "memory");
            lua_setfield(state, b, "memory");
        }

        /// @brief Protected: runs decide(b) and reads its answer into the DecideCall.
        int runDecide(lua_State* state) {
            auto* call = static_cast<DecideCall*>(lua_touserdata(state, 1));
            lua_getglobal(state, "decide");
            pushView(state, *call->view);
            lua_call(state, 1, 1);
            if (lua_isnil(state, -1)) {
                return 0;
            }
            if (!lua_istable(state, -1)) {
                return luaL_error(state, "decide must return a table or nil");
            }
            const int answer = lua_gettop(state);
            RemoteDecision decision;
            lua_getfield(state, answer, "skill");
            lua_getfield(state, answer, "item");
            if (!lua_isnil(state, -2)) {
                decision.id = static_cast<int>(luaL_checkinteger(state, -2));
            } else if (!lua_isnil(state, -1)) {
                decision.item = true;
                decision.id = static_cast<int>(luaL_checkinteger(state, -1));
            } else {
                return luaL_error(state, "decide must return {skill = id, target = f} or {item = id, target = f}");
            }
            lua_getfield(state, answer, "target");
            if (lua_isnil(state, -1)) {
                return luaL_error(state, "decide's answer has no target");
            }
            decision.targetSlot = slotArgument(state, lua_gettop(state));
            call->decision = decision;
            return 0;
        }

        /// @brief Protected: opens the sandboxed libraries, the prelude, and runs the script once.
        int runLoad(lua_State* state) {
            auto* call = static_cast<LoadCall*>(lua_touserdata(state, 1));
            const std::pair<const char*, lua_CFunction> libraries[] = {
                {LUA_GNAME, luaopen_base},
                {LUA_TABLIBNAME, luaopen_table},
                {LUA_STRLIBNAME, luaopen_string},
                {LUA_MATHLIBNAME, luaopen_math},
            };
            for (const auto& [name, open] : libraries) {
                luaL_requiref(state, name, open, 1);
                lua_pop(state, 1);
            }
            // no file access and no precompiled chunks
            for (const char* name : {"dofile", "loadfile", "load", "require"}) {
                lua_pushnil(state);
                lua_setglobal(state, name);
            }
            lua_getglobal(state, LUA_STRLIBNAME);
            lua_pushnil(state);
            lua_setfield(state, -2, "dump");
            lua_pop(state, 1);
            lua_pushlightuserdata(state, const_cast<std::function<void(const std::string&)>*>(call->print));
            lua_pushcclosure(state, print, 1);
            lua_setglobal(state, "print");
            lua_newtable(state);
            lua_setglobal(state, "memory");
            if (luaL_loadbufferx(state, kPrelude, std::char_traits<char>::length(kPrelude), "=prelude", "t") != LUA_OK) {
                return lua_error(state);
            }
            lua_call(state, 0, 0);
            if (luaL_loadbufferx(state, call->source->data(), call->source->size(), call->name->c_str(), "t") != LUA_OK) {
                return lua_error(state);
            }
            lua_call(state, 0, 0);
            if (lua_getglobal(state, "decide") != LUA_TFUNCTION) {
                return luaL_error(state, "the script has no function decide(b)");
            }
            return 0;
        }

        /// @brief Protected: gives the script a new, empty memory table.
        int clearMemory(lua_State* state) {
            lua_newtable(state);
            lua_setglobal(state, "memory");
            return 0;
        }

    }

    RemoteScript::RemoteScript(std::function<void(const std::string&)> print) : m_print(std::move(print)) {}

    RemoteScript::~RemoteScript() { unload(); }

    std::filesystem::path RemoteScript::pathFor(const std::filesystem::path& userRoot, std::int32_t playerId) {
        return userRoot / ("sd" + std::to_string(playerId ^ kFileKey) + ".lua");
    }

    bool RemoteScript::load(const std::filesystem::path& path) {
        unload();
        m_error.clear();
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return false;
        }
        const std::string source{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        return loadSource(source, "@" + path.filename().string());
    }

    bool RemoteScript::loadSource(const std::string& source, const std::string& name) {
        unload();
        m_error.clear();
        m_bytes = 0;
        m_state = lua_newstate(allocate, this);
        if (!m_state) {
            m_error = "not enough memory for the script";
            return false;
        }
        lua_sethook(m_state, countInstructions, LUA_MASKCOUNT, kHookEvery);
        LoadCall load{&source, &name, &m_print};
        if (!protect(runLoad, &load)) {
            unload();
            return false;
        }
        m_battleSeen = false;
        return true;
    }

    void RemoteScript::unload() {
        if (m_state) {
            lua_close(m_state);
            m_state = nullptr;
        }
        m_bytes = 0;
    }

    std::optional<RemoteDecision> RemoteScript::decide(const RemoteBattleView& view) {
        if (!m_state) {
            return std::nullopt;
        }
        m_error.clear();
        if (!m_battleSeen || view.battle != m_battle) {
            m_battle = view.battle;
            m_battleSeen = true;
            if (!protect(clearMemory, nullptr)) {
                return std::nullopt;
            }
        }
        DecideCall call;
        call.view = &view;
        if (!protect(runDecide, &call)) {
            return std::nullopt;
        }
        return call.decision;
    }

    void* RemoteScript::allocate(void* self, void* block, std::size_t oldSize, std::size_t newSize) {
        auto* script = static_cast<RemoteScript*>(self);
        const std::size_t held = block ? oldSize : 0; // without a block oldSize tells the kind of object, not a size
        if (newSize == 0) {
            std::free(block);
            script->m_bytes -= held;
            return nullptr;
        }
        if (newSize > held && script->m_bytes - held + newSize > kMostBytes) {
            return nullptr; // Lua raises "not enough memory"
        }
        void* grown = std::realloc(block, newSize);
        if (grown) {
            script->m_bytes = script->m_bytes - held + newSize;
        }
        return grown;
    }

    void RemoteScript::countInstructions(lua_State* state, lua_Debug*) {
        void* self = nullptr;
        lua_getallocf(state, &self);
        auto* script = static_cast<RemoteScript*>(self);
        script->m_instructions += kHookEvery;
        if (script->m_instructions > kMostInstructions) {
            luaL_error(state, "the script ran too long");
        }
    }

    bool RemoteScript::protect(int (*function)(lua_State*), void* argument) {
        m_instructions = 0;
        const int top = lua_gettop(m_state);
        lua_pushcfunction(m_state, function);
        lua_pushlightuserdata(m_state, argument);
        if (lua_pcall(m_state, 1, 0, 0) != LUA_OK) {
            const char* message = lua_tostring(m_state, -1);
            m_error = message ? message : "the script failed";
            lua_settop(m_state, top);
            return false;
        }
        lua_settop(m_state, top);
        return true;
    }

}
