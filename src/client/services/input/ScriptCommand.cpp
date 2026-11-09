#include "client/services/input/ScriptCommand.h"

#include <cctype>
#include <cstdlib>

namespace client {

    std::vector<std::string> ScriptCommand::words(const std::string& line) {
        std::vector<std::string> found;
        std::string word;
        bool inWord = false;
        bool quoted = false;
        for (const char c : line) {
            if (c == '"') {
                quoted = !quoted;
                inWord = true;
            } else if (std::isspace(static_cast<unsigned char>(c)) != 0 && !quoted) {
                if (inWord) {
                    found.push_back(word);
                    word.clear();
                    inWord = false;
                }
            } else {
                word += c;
                inWord = true;
            }
        }
        if (inWord) {
            found.push_back(word);
        }
        return found;
    }

    std::optional<int> ScriptCommand::number(const std::string& word) {
        const std::size_t first = !word.empty() && word[0] == '-' ? 1 : 0;
        if (first == word.size() || word.size() - first > 9) { // at most 9 digits so the value fits an int
            return std::nullopt;
        }
        for (std::size_t at = first; at < word.size(); ++at) {
            if (std::isdigit(static_cast<unsigned char>(word[at])) == 0) {
                return std::nullopt;
            }
        }
        return std::atoi(word.c_str());
    }

    std::optional<ScriptObjectKind> ScriptCommand::kindNamed(const std::string& name) {
        for (const ScriptObjectKind kind :
             {ScriptObjectKind::Self,
              ScriptObjectKind::Npc,
              ScriptObjectKind::NpcIndex,
              ScriptObjectKind::Player,
              ScriptObjectKind::Ground,
              ScriptObjectKind::Furniture,
              ScriptObjectKind::FurnitureItem}) {
            if (kindName(kind) == name) {
                return kind;
            }
        }
        return std::nullopt;
    }

    std::string ScriptCommand::kindName(ScriptObjectKind kind) {
        switch (kind) {
            case ScriptObjectKind::Self: // the own player
                return "self";
            case ScriptObjectKind::Npc: // NPC by its Npc.dat number
                return "npc";
            case ScriptObjectKind::NpcIndex: // NPC by its place number on the map
                return "npcindex";
            case ScriptObjectKind::Player: // another player by id
                return "player";
            case ScriptObjectKind::Ground: // a thing on the ground by index
                return "ground";
            case ScriptObjectKind::Furniture: // a house piece by number
                return "furn";
            case ScriptObjectKind::FurnitureItem: // first house piece with an item number
                return "furnitem";
        }
        return "?";
    }

    std::optional<ScriptObjectRef> ScriptCommand::object(const std::vector<std::string>& words, std::size_t first, std::string& error) {
        if (first >= words.size()) {
            error = "an object kind is missing (self, npc, npcindex, player, ground, furn, furnitem)";
            return std::nullopt;
        }
        const auto kind = kindNamed(words[first]);
        if (!kind) {
            error = "unknown object kind " + words[first];
            return std::nullopt;
        }
        ScriptObjectRef object{*kind, 0};
        if (*kind == ScriptObjectKind::Self) {
            return object;
        }
        const auto id = first + 1 < words.size() ? number(words[first + 1]) : std::nullopt;
        if (!id) {
            error = "the number of the " + words[first] + " is missing or not a number";
            return std::nullopt;
        }
        object.id = *id;
        return object;
    }

    std::optional<ScriptClickSpec> ScriptCommand::click(const std::vector<std::string>& words, std::string& error) {
        if (words.empty()) {
            return std::nullopt;
        }
        std::string name = words[0];
        ScriptClickSpec spec;
        if (name.rfind("ctrl", 0) == 0) {
            spec.ctrl = true;
            name.erase(0, 4); // length of the "ctrl" prefix
        } else if (name.rfind('d', 0) == 0) {
            spec.clicks = 2; // d prefix: a double click
            name.erase(0, 1);
        } else if (name.rfind('r', 0) == 0) {
            spec.button = ui::MouseButton::Right;
            name.erase(0, 1);
        }
        if (name == "clicknpc") {
            const auto id = words.size() > 1 ? number(words[1]) : std::nullopt;
            if (!id) {
                error = words[0] + " needs the NPC number";
                return std::nullopt;
            }
            spec.target = {ScriptObjectKind::Npc, *id};
            return spec;
        }
        if (name == "clickobj") {
            const auto target = object(words, 1, error);
            if (!target) {
                return std::nullopt;
            }
            spec.target = *target;
            return spec;
        }
        return std::nullopt;
    }

}
