#ifndef NORTH_ISLAND_CLIENT_SUBSYSTEMS_FISH_FISH_TANK_H
#define NORTH_ISLAND_CLIENT_SUBSYSTEMS_FISH_FISH_TANK_H

#include "client/subsystems/fish/TankBubble.h"
#include "client/subsystems/fish/TankFish.h"
#include "client/subsystems/fish/TankFood.h"
#include "core/Point.h"
#include "gfx/PictureLibrary.h"

#include <array>
#include <cstdint>
#include <functional>
#include <random>
#include <vector>

namespace client {

    /// @brief What swims, sinks and rises in the fish tank form: fish, food grains and bubbles.
    /// @details Original: MK_Menu.pas TFishManage, TFoodManage and TBubbleManage with their procedures.
    class FishTank {
        public:
            /// @brief Callbacks the tank calls when a fish eats or a grain lands.
            struct Hooks {
                std::function<void(int fish)> ate;
                std::function<void()> landed;
            };

            static constexpr int kKinds = 7; // fish kinds (pictures), 1-based
            static constexpr int kFishes = 10; // fish slots in the tank, 1-based (the owner's fish fill slots 3 and up)
            static constexpr int kFoods = 51; // most food grains in the water at once
            static constexpr int kBubbles = 2; // bubble sprites rising in the tank, 1-based

            /// @brief Builds the tank: loads the fish, food and bubble pictures and starts the two permanent fish.
            /// @details Original: TFishManage.Create, TFoodManage.Create, TBubbleManage.Create.
            /// @param pictures Picture library to find the tank pictures in.
            /// @param hooks Callbacks for eaten and landed grains.
            FishTank(
                gfx::PictureLibrary& pictures,
                Hooks hooks
            );

            /// @brief Sets the owner's fish from the kinds in the tank; the kinds met are listed in the order they were.
            /// @details Original: RecvFishTankList (the 8 bytes of packet 23/160 9; 0 is none).
            /// @param kinds Kind of fish per owner slot, 0 for none.
            void setTankFish(const std::vector<int>& kinds);

            /// @brief The kinds the tank holds, once each.
            /// @details Original: FStrLists[0].
            /// @return List of fish kinds.
            const std::vector<int>& kinds() const { return m_kinds; }

            /// @brief Removes a kind of fish from the list.
            /// @details Original: ConfirmFishBuy.
            /// @param index Position in the kinds list.
            void removeKind(std::size_t index);

            /// @brief Drops a food grain at the top of the form.
            /// @details Original: DropFood.
            /// @param x Horizontal position of the grain.
            void dropFood(int x);

            /// @brief Removes all food grains.
            /// @details Original: ClearAllFood.
            void clearFood();

            /// @brief Updates fish, falling food, bubbles, steers fish to food and lets them eat.
            /// @details Original: RenderWithChildren: UpdateAllFish, UpdateFallingFood, UpdateAllBubbles, SteerFishToFood and FishEatFood.
            /// @param nowMs Current time in milliseconds.
            void update(std::uint64_t nowMs);

            /// @brief Draws the fish, the food and the bubbles.
            /// @details Original: DrawAllFish, DrawAllFood, DrawAllBubbles.
            /// @param canvas Target canvas.
            /// @param origin Top-left of the form.
            /// @param formSize The form's width and height.
            /// @param nowMs Current time in milliseconds.
            void draw(gfx::Canvas& canvas, core::Point origin, core::Point formSize, std::uint64_t nowMs);

            /// @brief The picture of a kind of fish.
            /// @details Original: FFishPics.
            /// @param kind Fish kind 1..kKinds.
            /// @return Picture id.
            gfx::PictureId kindPicture(int kind) const { return m_kindPictures[static_cast<std::size_t>(kind)]; }

            /// @brief The picture size of a kind of fish.
            /// @param kind Fish kind 1..kKinds.
            /// @return Picture width and height.
            core::Point kindSize(int kind) const { return m_kindSizes[static_cast<std::size_t>(kind)]; }

            /// @brief Delphi-style Random(below): 0 .. below - 1, 0 for below of 0.
            /// @param below Exclusive upper bound.
            /// @return The random number.
            int random(int below);

            /// @brief The random function handed to the fish and bubbles.
            /// @return Reference to the stored random function.
            const TankFish::Random& randomFunction() const { return m_randomFunction; }

        private:
            /// @brief Draws a fish to the nearest grain within 150 pixels and makes it hurry.
            /// @details Original: SteerFishToFood.
            void steerToFood();

            /// @brief Lets every active fish eat the grains within its picture.
            /// @details Original: FishEatFood.
            void eatFood();

            /// @brief Sinks the grains; one past the bottom line is gone and the server hears of it.
            /// @details Original: UpdateFallingFood.
            void fallFood();

            /// @brief Removes every grain within the fish's picture and reports which fish ate it.
            /// @details Original: EatFoodInRect.
            /// @param fish The fish that eats.
            /// @param index Fish number 1..10.
            void eatInRect(const TankFish& fish, int index);

            Hooks m_hooks;
            std::mt19937 m_engine;
            TankFish::Random m_randomFunction;
            std::array<gfx::PictureId, kKinds + 1> m_kindPictures{};
            std::array<core::Point, kKinds + 1> m_kindSizes{};
            std::array<TankFish, kFishes + 1> m_fishes{};
            std::array<TankFood, kFoods> m_foods{};
            std::array<TankBubble, kBubbles + 1> m_bubbles{};
            std::vector<int> m_kinds;
    };

}

#endif
