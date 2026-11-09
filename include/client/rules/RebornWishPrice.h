#ifndef NORTH_ISLAND_CLIENT_RULES_REBORN_WISH_PRICE_H
#define NORTH_ISLAND_CLIENT_RULES_REBORN_WISH_PRICE_H

namespace client {

    /// @brief The star cost of wishing for a pet.
    /// @details Original: RebornStarTable's StarCost of a pet's grow key (Npc.dat); 14 keys cost stars, every other key (and one outside 0..50) cannot be wished for.
    /// @param growKey The pet's grow key.
    /// @return The star cost, 0 when it cannot be wished for.
    int rebornStarCost(int growKey);

    /// @brief The mark of the career of an old pet; the player who has its flag pays half.
    /// @details Original: PetCareerTable, GetPetCareerId then GetPetCareerMarkId.
    /// @param petNpcId The old pet's NPC id.
    /// @return The mark id, 0 for a pet with no career.
    int petCareerMarkId(int petNpcId);

}

#endif
