#include "ui/LoadContext.h"

#include "core/StringUtil.h"
#include "ui/LayoutError.h"

namespace ui {

    gfx::PictureId LoadContext::picture(const std::string& name) const {
        if (name.empty()) {
            return {};
        }
        return m_pictures.find(name).value_or(gfx::PictureId{});
    }

    gfx::PictureId LoadContext::pictureFile(const std::string& relativePath) const {
        if (relativePath.empty()) {
            return {};
        }
        return m_pictures.loadFile(m_resolveFile(relativePath)).value_or(gfx::PictureId{});
    }

    int LoadContext::parseInt(const std::string& value) {
        const auto parsed = core::parseInt(core::trim(value));
        if (!parsed) {
            throw LayoutError("not a number: \"" + value + "\"");
        }
        return *parsed;
    }

    bool LoadContext::parseBool(const std::string& value) {
        if (value == "true" || value == "True") {
            return true;
        }
        if (value == "false" || value == "False") {
            return false;
        }
        throw LayoutError("not true or false: \"" + value + "\"");
    }

    std::vector<int> LoadContext::parseInts(const std::string& value, std::size_t count) {
        const std::vector<std::string> parts = core::split(value, ',');
        if (parts.size() != count) {
            throw LayoutError("expected " + std::to_string(count) + " numbers: \"" + value + "\"");
        }
        std::vector<int> numbers;
        for (const std::string& part : parts) {
            numbers.push_back(parseInt(part));
        }
        return numbers;
    }

    core::Point LoadContext::parsePoint(const std::string& value) {
        const std::vector<int> n = parseInts(value, 2); // x and y
        return {n[0], n[1]};
    }

    core::Rect LoadContext::parseRect(const std::string& value) {
        const std::vector<int> n = parseInts(value, 4); // x, y, width and height
        return {n[0], n[1], n[2], n[3]};
    }

    core::Color LoadContext::parseColor(const std::string& value) {
        const auto color = core::Color::parse(core::trim(value));
        if (!color) {
            throw LayoutError("not a colour (#RRGGBB): \"" + value + "\"");
        }
        return *color;
    }

    gfx::TextEffect LoadContext::parseEffect(const std::string& value) {
        if (value == "Plain") {
            return gfx::TextEffect::Plain;
        }
        if (value == "Outline") {
            return gfx::TextEffect::Outline;
        }
        if (value == "Shadow") {
            return gfx::TextEffect::Shadow;
        }
        throw LayoutError("unknown text effect \"" + value + "\"");
    }

}
