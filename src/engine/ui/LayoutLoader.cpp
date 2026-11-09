#include "ui/LayoutLoader.h"

#include "ui/AnimatedIcon.h"
#include "ui/BlockList.h"
#include "ui/CheckBox.h"
#include "ui/ComboBox.h"
#include "ui/DigitText.h"
#include "ui/DrawingArea.h"
#include "ui/HintArea.h"
#include "ui/IconGrid.h"
#include "ui/ImageButton.h"
#include "ui/ImageView.h"
#include "ui/Label.h"
#include "ui/LayoutError.h"
#include "ui/ListBox.h"
#include "ui/Memo.h"
#include "ui/MessagePanel.h"
#include "ui/Panel.h"
#include "ui/ProgressBar.h"
#include "ui/ScrollBar.h"
#include "ui/Spot.h"
#include "ui/TextBox.h"
#include "ui/Window.h"

namespace ui {

    std::unique_ptr<Element> LayoutLoader::create(const std::string& type) {
        if (type == "Screen" || type == "Panel") {
            return std::make_unique<Panel>();
        }
        if (type == "IconGrid") {
            return std::make_unique<IconGrid>();
        }
        if (type == "Window") {
            return std::make_unique<Window>();
        }
        if (type == "AnimatedIcon") {
            return std::make_unique<AnimatedIcon>();
        }
        if (type == "Image") {
            return std::make_unique<ImageView>();
        }
        if (type == "ProgressBar") {
            return std::make_unique<ProgressBar>();
        }
        if (type == "ImageButton") {
            return std::make_unique<ImageButton>();
        }
        if (type == "CheckBox") {
            return std::make_unique<CheckBox>();
        }
        if (type == "ListBox") {
            return std::make_unique<ListBox>();
        }
        if (type == "Memo") {
            return std::make_unique<Memo>();
        }
        if (type == "BlockList") {
            return std::make_unique<BlockList>();
        }
        if (type == "ScrollBar") {
            return std::make_unique<ScrollBar>();
        }
        if (type == "TextBox") {
            return std::make_unique<TextBox>();
        }
        if (type == "ComboBox") {
            return std::make_unique<ComboBox>();
        }
        if (type == "Label") {
            return std::make_unique<Label>();
        }
        if (type == "DigitText") {
            return std::make_unique<DigitText>();
        }
        if (type == "HintArea") {
            return std::make_unique<HintArea>();
        }
        if (type == "DrawingArea") {
            return std::make_unique<DrawingArea>();
        }
        if (type == "MessagePanel") {
            return std::make_unique<MessagePanel>();
        }
        if (type == "Spot") {
            return std::make_unique<Spot>();
        }
        return nullptr;
    }

    std::unique_ptr<Element> LayoutLoader::build(const xml::Node& node, const LoadContext& context) {
        const std::string where = "line " + std::to_string(node.line()) + ", <" + node.name() + ">: ";
        std::unique_ptr<Element> element = create(node.name());
        if (!element) {
            throw LayoutError(where + "unknown element");
        }
        try {
            for (const auto& attribute : node.attributes()) {
                element->applyAttribute(attribute.first, attribute.second, context);
            }
        } catch (const LayoutError& error) {
            throw LayoutError(where + error.what());
        }
        for (const xml::Node& child : node.children()) {
            element->addChild(build(child, context));
        }
        element->finishLoading(context);
        return element;
    }

}
