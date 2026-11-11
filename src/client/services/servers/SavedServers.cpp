#include "client/services/servers/SavedServers.h"

#include "core/FileSystem.h"
#include "core/StringUtil.h"

#include <nlohmann/json.hpp>

namespace client {

    namespace {

        using Json = nlohmann::json;
        constexpr std::uint16_t kGamePort = 6414; // the game server's default TCP port (omitted from the shown address)
        const char* const kOldFile = "qol-servers.json"; // the file's name before it was saved-servers.json, read once to migrate

    }

    std::string DirectServer::address() const { return port == kGamePort ? host : host + ":" + std::to_string(port); }

    std::filesystem::path SavedServers::fileIn(const std::filesystem::path& userRoot) { return userRoot / "saved-servers.json"; }

    SavedServers SavedServers::loadIn(const std::filesystem::path& userRoot) {
        if (const auto data = core::readFile(fileIn(userRoot))) {
            return parse(core::toString(*data)).value_or(SavedServers{});
        }
        if (const auto data = core::readFile(userRoot / kOldFile)) {
            return parse(core::toString(*data)).value_or(SavedServers{});
        }
        return {};
    }

    std::optional<SavedServers> SavedServers::parse(const std::string& json) {
        const Json root = Json::parse(json, nullptr, false);
        if (!root.is_object()) {
            return std::nullopt;
        }
        SavedServers servers;
        if (const auto direct = root.find("direct"); direct != root.end() && direct->is_array()) {
            for (const Json& entry : *direct) {
                if (!entry.is_object() || !entry.contains("address") || !entry["address"].is_string()) {
                    continue;
                }
                if (auto server = parseAddress(entry["address"].get<std::string>())) {
                    if (entry.contains("name") && entry["name"].is_string() && !entry["name"].get<std::string>().empty()) {
                        server->name = entry["name"].get<std::string>();
                    }
                    servers.direct.push_back(std::move(*server));
                }
            }
        }
        if (const auto collapsed = root.find("collapsed"); collapsed != root.end() && collapsed->is_array()) {
            for (const Json& key : *collapsed) {
                if (key.is_string()) {
                    servers.collapsed.insert(key.get<std::string>());
                }
            }
        }
        return servers;
    }

    std::string SavedServers::toJson() const {
        Json root;
        root["direct"] = Json::array();
        for (const DirectServer& server : direct) {
            root["direct"].push_back({{"name", server.name}, {"address", server.address()}});
        }
        root["collapsed"] = Json(std::vector<std::string>(collapsed.begin(), collapsed.end()));
        return root.dump(2) + "\n";
    }

    bool SavedServers::save(const std::filesystem::path& file) const {
        std::error_code ignored;
        std::filesystem::create_directories(file.parent_path(), ignored);
        return core::writeFile(file, core::toBytes(toJson()));
    }

    std::optional<DirectServer> SavedServers::parseAddress(const std::string& text) {
        std::string host = text;
        std::uint16_t port = kGamePort;
        if (const std::size_t colon = text.find(':'); colon != std::string::npos) {
            host = text.substr(0, colon);
            const std::string digits = text.substr(colon + 1);
            if (digits.empty() || digits.size() > 5 || digits.find_first_not_of("0123456789") != std::string::npos) { // a port has at most 5 digits
                return std::nullopt;
            }
            const int number = std::stoi(digits);
            if (number < 1 || number > 65535) { // valid port range 1..65535
                return std::nullopt;
            }
            port = static_cast<std::uint16_t>(number);
        }
        const bool hostChars = host.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-") == std::string::npos;
        // 253: the longest host name allowed
        if (host.empty() || host.size() > 253 || !hostChars || host.front() == '.' || host.front() == '-' || host.back() == '.' ||
            host.find("..") != std::string::npos) {
            return std::nullopt;
        }
        DirectServer server;
        server.host = core::toLower(host);
        server.port = port;
        server.name = server.address();
        return server;
    }

}
