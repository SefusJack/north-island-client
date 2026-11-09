#ifndef NORTH_ISLAND_CLIENT_DATA_CAREER_LOOKS_H
#define NORTH_ISLAND_CLIENT_DATA_CAREER_LOOKS_H

#include "client/models/character/CareerModel.h"
#include "formats/ItemTable.h"

#include <string>
#include <vector>

namespace client {

    /// @brief Builds one career form model: the player's body with the given items worn.
    /// @details Original: LoadCareerModels / LoadBeforeCareerModels for one model: TShowModel.AssignLook (the player's body, colours back to the unchanged 4s, nothing in the special slot) and SetShowModelEquip for each item of the class.
    /// @param player The player's look to take the body from.
    /// @param items The item table to find the items' look ids in.
    /// @param name Display name of the model.
    /// @param career Career number (1..6).
    /// @param itemIds Item ids the model wears.
    /// @return The finished model.
    CareerModel makeCareerModel(
        const CharacterLook& player,
        const formats::ItemTable& items,
        const std::string& name,
        int career,
        const std::vector<int>& itemIds
    );

    /// @brief Builds a class model of the career forms: killer, warrior, knight, wit, priest, seer (1..6), the priest and the seer in their own colours.
    /// @param player The player's look to take the body from.
    /// @param items The item table to find the items' look ids in.
    /// @param career Career number (1..6).
    /// @return The class model.
    CareerModel makeCareerClassModel(const CharacterLook& player, const formats::ItemTable& items, int career);

    /// @brief Builds one of the two models of the soldier / mage choice.
    /// @param player The player's look to take the body from.
    /// @param items The item table to find the items' look ids in.
    /// @param slot Model slot (1 soldier, 2 mage).
    /// @return The model.
    CareerModel makeBeforeCareerModel(const CharacterLook& player, const formats::ItemTable& items, int slot);

}

#endif
