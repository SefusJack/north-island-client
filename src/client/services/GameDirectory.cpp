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
        return core::resolvePath(m_data, relative);
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
