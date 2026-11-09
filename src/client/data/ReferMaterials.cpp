#include "client/data/ReferMaterials.h"

#include <array>

namespace client {

    namespace {

        // rows: {name, icon picture name}, codes 1..68 in order
        const std::array<ReferMaterials::Material, ReferMaterials::kMostCode> kMaterials = {
            {{"Meat", "1637"},       {"Sewage", "1404"},    {"Y. Clay", "1340"},    {"Silver", "1618"},     {"Red Iron", "1557"},    {"Pure Water", "1407"},
             {"Seafood", "1813"},    {"Milk", "1032"},      {"Egg", "1295"},        {"Black Clay", "1338"}, {"Fuel", "2040"},        {"Seed", "2758"},
             {"Silicon", "1578"},    {"Sulfur", "1486"},    {"Leaf Fibre", "1254"}, {"Copper", "1592"},     {"Iron", "1586"},        {"Wood", "1582"},
             {"Rice", "1290"},       {"Saltpeter", "1480"}, {"Mercury", "1473"},    {"Ti. Ore", "1527"},    {"Comm. Clay", "1342"},  {"Red Clay", "1341"},
             {"Glue", "1474"},       {"Coal", "1285"},      {"Creations", "1902"},  {"Magnet", "1576"},     {"Crystalloid", "1329"}, {"Steel", "1619"},
             {"White Clay", "1337"}, {"Medicine", "1465"},  {"Gold", "1556"},       {"Barley", "1596"},     {"Vegetable", "1665"},   {"Fruit", "1648"},
             {"F. Fibre", "1283"},   {"Wolf Meat", "1445"}, {"Tin Ore", "1585"},    {"Galena", "1345"},     {"Crude oil", "1471"},   {"Fine Oil", "1472"},
             {"Aluminum", "1346"},   {"G. Fibre", "1185"},  {"Bean", "1511"},       {"Hide", "1551"},       {"Beast Bone", "2091"},  {"W. Silver", "1571"},
             {"Dry Clay", "1340"},   {"Stone", "1188"},     {"Nest", "1465"},       {"Gut", "1902"},        {"Excreta", "1340"},     {"Evil Item", "1938"},
             {"Venom", "1609"},      {"Alcohol", "1060"},   {"Secretion", "1608"},  {"Grey Clay", "1339"},  {"Chemical", "1428"},    {"Mercury", "1473"},
             {"Nylon", "1429"},      {"Diamond", "2036"},   {"Beast Fur", "1537"},  {"Feather", "1457"},    {"Shell", "1452"},       {"Jade", "1941"},
             {"Gem", "1331"},        {"Crystal", "1588"}}
        };

    }

    std::optional<ReferMaterials::Material> ReferMaterials::of(int code) {
        if (code < 1 || code > kMostCode) {
            return std::nullopt;
        }
        return kMaterials[static_cast<std::size_t>(code - 1)];
    }

}
