#include "client/services/GameDirectory.h"

#include "client/services/EmbeddedLayouts.h"
#include "core/FileSystem.h"
#include "ui/LayoutError.h"

namespace client {

    std::filesystem::path GameDirectory::data(const std::string& relative) const {
        if (m_soundPacks) {
            if (auto sound = m_soundPacks(relative)) {
                return std::move(*sound);
            }
        }
        {
            std::lock_guard lock(m_resolvedLock);
            if (const auto found = m_resolved.find(relative); found != m_resolved.end()) {
                return found->second;
            }
        }
        std::filesystem::path resolved = core::resolvePath(m_data, relative);
        std::error_code error;
        if (std::filesystem::exists(resolved, error)) { // only existing files: a missing one may still appear
            std::lock_guard lock(m_resolvedLock);
            m_resolved.emplace(relative, resolved);
        }
        return resolved;
    }

    ui::LayoutSource GameDirectory::layout(const std::string& fileName) const {
        if (!m_layouts.empty()) {
            if (const auto bytes = core::readFile(m_layouts / fileName)) {
                return {fileName, core::toString(*bytes)};
            }
        }
        if (auto text = embeddedLayout(fileName)) {
            return {fileName, std::move(*text)};
        }
        throw ui::LayoutError("no layout " + fileName);
    }

}
