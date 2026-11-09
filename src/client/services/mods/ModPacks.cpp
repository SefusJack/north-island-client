#include "client/services/mods/ModPacks.h"

#include "client/rendering/characters/RoleSpriteLibrary.h"

#include "core/Bytes.h"
#include "core/FileSystem.h"
#include "core/Log.h"
#include "core/StringUtil.h"
#include "formats/ItemTable.h"
#include "formats/NpcTable.h"
#include "formats/SkillTable.h"
#include "formats/TalkTable.h"
#include "gfx/PictureLibrary.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <functional>
#include <optional>
#include <set>
#include <system_error>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace client {

    namespace {

        using Json = nlohmann::json;

        const char* const kLoadOrderFile = "load-order.json";
        const char* const kPackFile = "mod.json";
        const std::set<std::string> kSoundExtensions = {".wav", ".ogg", ".flac", ".mp3"};
        const std::set<std::string> kPictureExtensions = {".png", ".bmp", ".jpg"};

        /// @brief Reads a JSON object file, reporting when it exists but is not a JSON object.
        /// @param file The file.
        /// @param report Receives the problem line.
        /// @return The object, empty when the file is missing or invalid.
        std::optional<Json> readJson(const fs::path& file, std::vector<std::string>& report) {
            const auto data = core::readFile(file);
            if (!data) {
                return std::nullopt;
            }
            Json json = Json::parse(core::toString(*data), nullptr, false);
            if (json.is_discarded() || !json.is_object()) {
                report.push_back("cannot read " + file.string() + ": not a JSON object");
                return std::nullopt;
            }
            return json;
        }

        /// @brief Reads a string member of a JSON object.
        /// @param object The object.
        /// @param key Member name.
        /// @return The string, empty when missing or not a string.
        std::string stringOf(const Json& object, const char* key) {
            const auto found = object.find(key);
            return found != object.end() && found->is_string() ? found->get<std::string>() : std::string();
        }

        /// @brief Reads an array-of-strings member of a JSON object.
        /// @param object The object.
        /// @param key Member name.
        /// @return The strings, empty when missing; non-string items are skipped.
        std::vector<std::string> stringsOf(const Json& object, const char* key) {
            std::vector<std::string> strings;
            if (const auto found = object.find(key); found != object.end() && found->is_array()) {
                for (const Json& item : *found) {
                    if (item.is_string()) {
                        strings.push_back(item.get<std::string>());
                    }
                }
            }
            return strings;
        }

        /// @brief Reads a table id from a JSON key.
        /// @param key The key text.
        /// @return The id, empty when it is not a positive number.
        std::optional<int> idOf(const std::string& key) {
            const auto id = core::parseInt(key);
            return id && *id > 0 ? id : std::nullopt;
        }

        /// @brief Gives the lower-case extension of a file.
        /// @param file The file path.
        /// @return The extension with its dot.
        std::string extensionOf(const fs::path& file) { return core::toLower(file.extension().string()); }

    }

    ModPacks ModPacks::load(const fs::path& modsRoot) {
        ModPacks mods;
        std::error_code error;
        if (modsRoot.empty() || !fs::is_directory(modsRoot, error)) {
            return mods;
        }
        std::map<std::string, ModPack> found;
        for (const auto& entry : fs::directory_iterator(modsRoot, error)) {
            if (!entry.is_directory(error)) {
                continue;
            }
            const auto json = readJson(entry.path() / kPackFile, mods.m_report);
            if (!json) {
                mods.m_report.push_back("skipped " + entry.path().filename().string() + ": no mod.json");
                continue;
            }
            ModPack pack;
            pack.folder = entry.path();
            pack.id = stringOf(*json, "id");
            if (pack.id.empty()) {
                pack.id = entry.path().filename().string();
            }
            pack.name = stringOf(*json, "name");
            pack.version = stringOf(*json, "version");
            pack.author = stringOf(*json, "author");
            pack.description = stringOf(*json, "description");
            pack.needs = stringsOf(*json, "requires");
            pack.loadAfter = stringsOf(*json, "loadAfter");
            if (!found.emplace(pack.id, pack).second) {
                mods.m_report.push_back("skipped " + entry.path().filename().string() + ": a second pack with the id " + pack.id);
            }
        }

        const fs::path orderFile = modsRoot / kLoadOrderFile;
        Json order = readJson(orderFile, mods.m_report).value_or(Json::object());
        if (!order.contains("mods") || !order["mods"].is_array()) {
            order["mods"] = Json::array();
        }
        std::set<std::string> listed;
        for (const Json& item : order["mods"]) {
            if (!item.is_object()) {
                continue;
            }
            const std::string id = stringOf(item, "id");
            listed.insert(id);
            const auto pack = found.find(id);
            if (pack == found.end()) {
                mods.m_report.push_back("load order: " + id + " is not in mods\\");
                continue;
            }
            const auto enabled = item.find("enabled");
            if (enabled != item.end() && enabled->is_boolean() && enabled->get<bool>()) {
                mods.m_packs.push_back(pack->second);
            }
        }
        bool added = !fs::exists(orderFile, error);
        for (const auto& [id, pack] : found) {
            if (listed.count(id) == 0) {
                order["mods"].push_back({{"id", id}, {"enabled", false}});
                mods.m_report.push_back("new pack " + id + ": added to load-order.json, turned off");
                added = true;
            }
        }
        if (added && !core::writeFile(orderFile, core::toBytes(order.dump(2) + "\n"))) { // JSON indent of 2 spaces
            mods.m_report.push_back("cannot write " + orderFile.string());
        }

        for (std::size_t i = 0; i < mods.m_packs.size(); ++i) {
            const ModPack& pack = mods.m_packs[i];
            const auto position = [&](const std::string& id) {
                const auto at = std::find_if(mods.m_packs.begin(), mods.m_packs.end(), [&](const ModPack& other) { return other.id == id; });
                return at == mods.m_packs.end() ? std::optional<std::size_t>() : std::optional<std::size_t>(at - mods.m_packs.begin());
            };
            for (const std::string& required : pack.needs) {
                if (!position(required)) {
                    mods.m_report.push_back(pack.id + " needs " + required + ", which is not turned on");
                }
            }
            for (const std::string& before : pack.loadAfter) {
                if (const auto at = position(before); at && *at > i) {
                    mods.m_report.push_back(pack.id + " should come after " + before + " in the load order");
                }
            }
        }
        for (const ModPack& pack : mods.m_packs) {
            mods.index(pack);
        }
        return mods;
    }

    std::string ModPacks::soundKey(const std::string& relative) {
        std::string path = core::toLower(relative);
        std::replace(path.begin(), path.end(), '\\', '/');
        while (path.rfind("./", 0) == 0) {
            path.erase(0, 2); // length of the "./" prefix
        }
        const auto slash = path.find('/');
        if (slash == std::string::npos) {
            return {};
        }
        const std::string folder = path.substr(0, slash);
        if (folder != "sound" && folder != "mp3s") {
            return {};
        }
        const auto dot = path.rfind('.');
        if (dot != std::string::npos && dot > path.rfind('/')) {
            path.erase(dot);
        }
        return path;
    }

    void ModPacks::put(std::map<std::string, Entry>& entries, const std::string& kind, const std::string& key, Entry entry) {
        if (const auto old = entries.find(key); old != entries.end() && old->second.pack != entry.pack) {
            m_report.push_back(kind + " " + key + ": " + entry.pack + " hides " + old->second.pack);
        }
        entries[key] = std::move(entry);
    }

    void ModPacks::index(const ModPack& pack) {
        std::error_code error;
        int sounds = 0;
        for (const auto& [folder, prefix] : {std::pair<const char*, const char*>{"sounds", "sound/"}, {"mp3s", "mp3s/"}}) {
            const fs::path root = pack.folder / folder;
            if (!fs::is_directory(root, error)) {
                continue;
            }
            for (const auto& entry : fs::recursive_directory_iterator(root, error)) {
                if (!entry.is_regular_file(error) || kSoundExtensions.count(extensionOf(entry.path())) == 0) {
                    continue;
                }
                fs::path relative = entry.path().lexically_relative(root);
                relative.replace_extension();
                put(m_sounds, "sound", prefix + core::toLower(relative.generic_string()), {entry.path(), pack.id});
                ++sounds;
            }
        }
        int pictures = 0;
        for (const auto& entry : fs::directory_iterator(pack.folder / "pictures", error)) {
            if (entry.is_regular_file(error) && kPictureExtensions.count(extensionOf(entry.path())) != 0) {
                put(m_pictures, "picture", core::toLower(entry.path().stem().string()), {entry.path(), pack.id});
                ++pictures;
            }
        }
        int hdPictures = 0;
        for (const auto& entry : fs::directory_iterator(pack.folder / "pictures" / "@2x", error)) {
            if (entry.is_regular_file(error) && kPictureExtensions.count(extensionOf(entry.path())) != 0) {
                put(m_hdPictures, "picture@2x", core::toLower(entry.path().stem().string()), {entry.path(), pack.id});
                ++hdPictures;
            }
        }
        int hdFonts = 0;
        for (const auto& entry : fs::directory_iterator(pack.folder / "fonts" / "@2x", error)) {
            if (entry.is_regular_file(error) && extensionOf(entry.path()) == ".png") {
                put(m_hdFonts, "font@2x", core::toLower(entry.path().stem().string()), {entry.path(), pack.id});
                ++hdFonts;
            }
        }
        int hdFrames = 0;
        const fs::path spriteRoot = pack.folder / "sprites" / "@2x";
        if (fs::is_directory(spriteRoot, error)) {
            for (const auto& entry : fs::recursive_directory_iterator(spriteRoot, error)) {
                if (!entry.is_regular_file(error) || extensionOf(entry.path()) != ".png") {
                    continue;
                }
                const fs::path relative = entry.path().lexically_relative(spriteRoot);
                std::vector<std::string> parts;
                for (const auto& part : relative) {
                    parts.push_back(part.string());
                }
                // 3 path parts: bank, sprite, frame file
                const auto sprite = parts.size() == 3 ? core::parseInt(parts[1]) : std::nullopt;
                const auto frame = parts.size() == 3 ? core::parseInt(fs::path(parts[2]).stem().string()) : std::nullopt;
                if (!sprite || !frame || *frame < 0) {
                    m_report.push_back(pack.id + ": sprites/@2x/" + relative.generic_string() + " is not <bank>/<sprite>/<frame>.png: left out");
                    continue;
                }
                put(m_hdSprites,
                    "sprite frame@2x",
                    core::toLower(parts[0]) + "/" + std::to_string(*sprite) + "/" + std::to_string(*frame),
                    {entry.path(), pack.id});
                ++hdFrames;
            }
        }
        m_report.push_back(
            "pack " + pack.id + (pack.name.empty() ? "" : " (" + pack.name + (pack.version.empty() ? "" : " " + pack.version) + ")") + ": " +
            std::to_string(sounds) + " sounds, " + std::to_string(pictures) + " pictures" +
            (hdPictures > 0 ? ", " + std::to_string(hdPictures) + " @2x pictures" : std::string()) +
            (hdFrames > 0 ? ", " + std::to_string(hdFrames) + " @2x sprite frames" : std::string()) +
            (hdFonts > 0 ? ", " + std::to_string(hdFonts) + " @2x fonts" : std::string())
        );
    }

    std::optional<fs::path> ModPacks::sound(const std::string& relative) const {
        if (m_sounds.empty()) {
            return std::nullopt;
        }
        const auto found = m_sounds.find(soundKey(relative));
        return found != m_sounds.end() ? std::optional<fs::path>(found->second.file) : std::nullopt;
    }

    void ModPacks::addPictures(gfx::PictureLibrary& pictures) {
        for (const auto& [name, entry] : m_pictures) {
            pictures.addReplacement(name, entry.file);
        }
        for (const auto& [name, entry] : m_hdPictures) {
            pictures.addReplacement(name, entry.file, 2); // @2x pictures are twice the size each way
        }
    }

    std::optional<fs::path> ModPacks::hdFont(const std::string& fontFile) const {
        const auto found = m_hdFonts.find(core::toLower(fontFile));
        return found != m_hdFonts.end() ? std::optional<fs::path>(found->second.file) : std::nullopt;
    }

    void ModPacks::addSprites(RoleSpriteLibrary& sprites) const {
        for (const auto& [key, entry] : m_hdSprites) {
            const auto slash = key.find('/');
            const auto second = key.find('/', slash + 1);
            sprites.addHdFrame(
                key.substr(0, slash),
                static_cast<int>(*core::parseInt(key.substr(slash + 1, second - slash - 1))),
                static_cast<int>(*core::parseInt(key.substr(second + 1))),
                entry.file
            );
        }
    }

    void ModPacks::applyText(
        formats::ItemTable& items,
        formats::SkillTable& skills,
        formats::NpcTable& npcs,
        formats::TalkTable& talks,
        bool simplified
    ) {
        for (const ModPack& pack : m_packs) {
            for (const char* table : {"Item", "Skill", "Npc", "Talk"}) {
                const auto json = readJson(pack.folder / "text" / (std::string(table) + ".json"), m_report);
                if (!json) {
                    continue;
                }
                const std::string what = pack.id + " text/" + table + ".json";
                int changed = 0;
                int lost = 0;
                std::vector<std::string> unknown;
                std::set<std::string> refused;
                const auto text = [&](const Json& value) { return gameTextFromUtf8(value.get<std::string>(), simplified, lost); };
                for (const auto& [key, value] : json->items()) {
                    const auto id = idOf(key);
                    bool known = false;
                    if (!id) {
                        unknown.push_back(key);
                        continue;
                    }
                    if (std::string(table) == "Talk") {
                        std::optional<std::reference_wrapper<const Json>> line;
                        if (value.is_string()) {
                            line = std::cref(value);
                        } else if (value.is_object() && value.contains("text")) {
                            line = std::cref(value["text"]);
                        }
                        if (!line || !line->get().is_string()) {
                            refused.insert("(not a line)");
                            continue;
                        }
                        known = talks.setText(*id, text(line->get()));
                    } else if (value.is_object()) {
                        std::optional<std::string> name;
                        std::optional<std::string> description;
                        for (const auto& [field, fieldValue] : value.items()) {
                            const bool allowed = field == "name" || (field == "description" && std::string(table) != "Npc");
                            if (!allowed || !fieldValue.is_string()) {
                                refused.insert(field);
                            } else if (field == "name") {
                                name = text(fieldValue);
                            } else {
                                description = text(fieldValue);
                            }
                        }
                        if (std::string(table) == "Item") {
                            known = items.setText(*id, name, description);
                        } else if (std::string(table) == "Skill") {
                            known = skills.setText(*id, name, description);
                        } else {
                            known = !name || npcs.setName(*id, *name);
                        }
                    } else {
                        refused.insert("(not an object)");
                        continue;
                    }
                    if (known) {
                        ++changed;
                    } else {
                        unknown.push_back(key);
                    }
                }
                std::string line = what + ": " + std::to_string(changed) + " changed";
                if (!unknown.empty()) {
                    line += ", " + std::to_string(unknown.size()) + " not in the game (" + unknown.front() + (unknown.size() > 1 ? ", ..." : "") + ")";
                }
                if (!refused.empty()) {
                    line += ", left out:";
                    for (const std::string& field : refused) {
                        line += " " + field;
                    }
                }
                if (lost > 0) {
                    line += ", " + std::to_string(lost) + " texts with characters the font has not";
                }
                m_report.push_back(line);
            }
        }
    }

    std::string gameTextFromUtf8(const std::string& utf8, bool simplified, int& lost) {
        if (std::all_of(utf8.begin(), utf8.end(), [](char c) { return static_cast<unsigned char>(c) < 0x80; })) { // bytes below 0x80 are ASCII
            return utf8;
        }
#ifdef _WIN32
        const int wide = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
        std::wstring text(static_cast<std::size_t>(std::max(wide, 0)), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), text.data(), wide);
        const UINT codePage = simplified ? 936 : 950; // Windows code pages: 936 GBK (simplified), 950 Big5
        BOOL usedDefault = FALSE;
        const int size = WideCharToMultiByte(codePage, 0, text.data(), wide, nullptr, 0, "?", &usedDefault);
        std::string game(static_cast<std::size_t>(std::max(size, 0)), '\0');
        WideCharToMultiByte(codePage, 0, text.data(), wide, game.data(), size, "?", &usedDefault);
        if (usedDefault) {
            ++lost;
        }
        return game;
#else
        (void)simplified;
        std::string game;
        for (std::size_t i = 0; i < utf8.size(); ++i) {
            const auto c = static_cast<unsigned char>(utf8[i]);
            if (c < 0x80) { // ASCII byte
                game += static_cast<char>(c);
            } else if ((c & 0xC0) != 0x80) { // 10xxxxxx marks a UTF-8 continuation byte; a lead byte becomes '?'
                game += '?';
            }
        }
        ++lost;
        return game;
#endif
    }

}
