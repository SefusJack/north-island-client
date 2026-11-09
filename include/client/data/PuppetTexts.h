#ifndef NORTH_ISLAND_CLIENT_DATA_PUPPET_TEXTS_H
#define NORTH_ISLAND_CLIENT_DATA_PUPPET_TEXTS_H

#include <string>

namespace client {

    constexpr const char* kPuppetSizeLimit = "The capacity is at its limit.";
    constexpr const char* kPuppetExpandTo = "Expand the limit to ";
    constexpr const char* kPuppetNeed = ", which needs ";
    constexpr const char* kPuppetScrollsSure = " enhancement scrolls. Expand?";
    constexpr const char* kPuppetInTraining = "Memory Card capacity cannot be expanded while training.";
    constexpr const char* kPuppetCardFull = "The capacity has reached its limit.";
    constexpr const char* kPuppetNotEnoughScrolls = "Not enough enhancement scrolls.";
    constexpr const char* kPuppetSizeIs = "The Memory Card capacity is now: ";

    /// @brief Formats the training time left as "2 Day 3 Hour 4 Minute ", leaving out the larger units when they are 0.
    /// @details Original: RecvTrainTimeLeft.
    /// @param minutes Minutes left.
    /// @return The text.
    std::string puppetDuration(int minutes);

}

#endif
