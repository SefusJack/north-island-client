#include "client/rules/StoreItemText.h"

#include "client/rules/Equipping.h"

namespace client {

    std::string storeItemText(const formats::ItemRecord& item, bool withLevel) {
        std::string text;
        const auto add = [&text](const std::string& part) { text += text.empty() ? part : ", " + part; };
        for (std::size_t index = 0; index < item.attributeKinds.size(); ++index) {
            const int kind = item.attributeKinds[index];
            std::string name;
            switch (kind) {
                case 25: name = "HP"; break; // attribute kind 25: HP
                case 26: name = "SP"; break; // attribute kind 26: SP
                case 207: name = "MaxHp"; break; // attribute kind 207: max HP
                case 208: name = "MaxSp"; break; // attribute kind 208: max SP
                case 210: name = "ATK"; break; // attribute kind 210: attack
                case 211: name = "DEF"; break; // attribute kind 211: defence
                case 214: name = "SPD"; break; // attribute kind 214: speed
                case 215: name = "MATK"; break; // attribute kind 215: magic attack
                case 216: name = "MDEF"; break; // attribute kind 216: magic defence
                case 100: name = "resistance of shoot"; break; // attribute kind 100: shot resistance (per cent)
                default: continue; // other attribute kinds are not shown
            }
            const std::int32_t value = item.attributeValues[index];
            // attribute values are stored around 100 (100 = no change)
            if (value < 100) {
                name += ": -" + std::to_string(100 - value);
            } else if (value > 100) {
                name += ": +" + std::to_string(value - 100);
            } else {
                continue;
            }
            add(name);
            if (kind == 100) { // attribute kind 100: shot resistance
                text += "%";
            }
        }
        if (withLevel && isEquipment(item)) {
            add("Obtained at: " + std::to_string(item.requiredLevel));
        }
        return text.empty() ? item.description : text;
    }

}
