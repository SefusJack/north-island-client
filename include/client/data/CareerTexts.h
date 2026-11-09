#ifndef NORTH_ISLAND_CLIENT_DATA_CAREER_TEXTS_H
#define NORTH_ISLAND_CLIENT_DATA_CAREER_TEXTS_H

#include <string>

namespace client {

    /// @brief Gives the name of a class of the career forms (MZ_Menu.pas): 1 Killer, 2 Warrior, 3 Knight, 4 Wit, 5 Priest, 6 Seer.
    /// @details The names keep the trailing blank of the game's JobNames.
    /// @param career Career number (1..6).
    /// @return The class name, or an empty string for an unknown career.
    std::string careerName(int career);

    /// @brief Gives the description of a class, as the form draws it under the models (GetCareerDescription).
    /// @param career Career number (1..6).
    /// @return The description, or an empty string for an unknown career.
    std::string careerDescription(int career);

    /// @brief Gives the bonus text of a class, as the form draws it under the models (GetCareerBonusText).
    /// @param career Career number (1..6).
    /// @return The bonus text, or an empty string for an unknown career.
    std::string careerBonusText(int career);

    /// @brief Gives the direction a model faces when it is chosen (SetShowModelCareerPose).
    /// @details Models at rest face kCareerRestPose, the one under the mouse kCareerHoverPose (SetShowModelHoverPose).
    /// @param career Career number (1..6).
    /// @return The pose number.
    int careerChosenPose(int career);
    constexpr int kCareerRestPose = 16; // pose 16: action pose of a career model at rest in the choose-career form
    constexpr int kCareerHoverPose = 58; // pose 58: action pose of the model under the mouse

}

#endif
