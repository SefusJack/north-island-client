#include "client/subsystems/fish/FishTank.h"

#include <cmath>
#include <cstdlib>
#include <string>
#include <utility>

namespace client {

    namespace {

        constexpr double kScent = 150.0; // distance within which a fish smells food and swims to it, px
        constexpr int kFoodBonus = 2; // extra px per frame a fish swims at when heading for food
        constexpr int kFirstOwned = 3; // first fish slot for the owner's fish (slots 1 and 2 are permanent)
        constexpr int kPermanentLine = 100; // the permanent fish keep below this y, px
        constexpr int kBottom = 250; // y at which a food grain reaches the bottom and lands, px
        constexpr int kInTankKind = 6; // fish kind 6: the "in tank" fish, which keeps surfacing
        // start x, y of each bubble (index 0 unused), px in the tank
        constexpr core::Point kBubbleStarts[] = {{0, 0}, {100, 210}, {450, 160}};

        /// @brief The picture name of a kind of fish.
        /// @param kind Fish kind.
        /// @return Name such as fish_01.
        std::string kindName(int kind) { return std::string("fish_0") + std::to_string(kind); }

        /// @brief The width and height of a picture.
        /// @param pictures Picture library.
        /// @param id Picture asked about.
        /// @return Its size.
        core::Point sizeOf(gfx::PictureLibrary& pictures, gfx::PictureId id) { return {pictures.picture(id).width(), pictures.picture(id).height()}; }

    }

    FishTank::FishTank(
        gfx::PictureLibrary& pictures,
        Hooks hooks
    )
        : m_hooks(std::move(hooks)),
          m_engine(std::random_device{}()),
          m_randomFunction([this](int below) { return random(below); }) {
        for (int kind = 1; kind <= kKinds; ++kind) {
            m_kindPictures[static_cast<std::size_t>(kind)] = pictures.find(kindName(kind)).value_or(gfx::PictureId{});
            if (m_kindPictures[static_cast<std::size_t>(kind)].isValid()) {
                m_kindSizes[static_cast<std::size_t>(kind)] = sizeOf(pictures, m_kindPictures[static_cast<std::size_t>(kind)]);
            }
        }
        for (int fish = 1; fish <= 2; ++fish) { // fish 1 and 2 are the permanent ones
            m_fishes[static_cast<std::size_t>(fish)]
                .setPicture(m_kindPictures[static_cast<std::size_t>(fish)], m_kindSizes[static_cast<std::size_t>(fish)], m_randomFunction);
            m_fishes[static_cast<std::size_t>(fish)].minY = kPermanentLine;
        }
        const gfx::PictureId food = pictures.find("FishFood").value_or(gfx::PictureId{});
        for (TankFood& grain : m_foods) {
            grain.setPicture(food, food.isValid() ? sizeOf(pictures, food) : core::Point{});
        }
        const gfx::PictureId bubble = pictures.find("Icon_Bubble_2").value_or(gfx::PictureId{});
        for (std::size_t index = 1; index < m_bubbles.size(); ++index) {
            m_bubbles[index].setPicture(bubble, bubble.isValid() ? sizeOf(pictures, bubble) : core::Point{});
            m_bubbles[index].startX = kBubbleStarts[index].x;
            m_bubbles[index].startY = kBubbleStarts[index].y;
        }
    }

    int FishTank::random(int below) { return below <= 0 ? 0 : std::uniform_int_distribution<int>(0, below - 1)(m_engine); }

    void FishTank::setTankFish(const std::vector<int>& kinds) {
        m_kinds.clear();
        for (int fish = kFirstOwned; fish <= kFishes; ++fish) {
            m_fishes[static_cast<std::size_t>(fish)].clear();
        }
        for (int fish = kFirstOwned; fish <= kFishes; ++fish) {
            const std::size_t at = static_cast<std::size_t>(fish - kFirstOwned);
            const int kind = at < kinds.size() ? kinds[at] : 0;
            if (kind == 0 || kind > kKinds) {
                continue;
            }
            TankFish& entry = m_fishes[static_cast<std::size_t>(fish)];
            if (kind == kInTankKind) {
                entry.inTank = true;
            }
            entry.setPicture(m_kindPictures[static_cast<std::size_t>(kind)], m_kindSizes[static_cast<std::size_t>(kind)], m_randomFunction);
            bool known = false;
            for (const int listed : m_kinds) {
                known = known || listed == kind;
            }
            if (!known) {
                m_kinds.push_back(kind);
            }
        }
    }

    void FishTank::removeKind(std::size_t index) {
        if (index < m_kinds.size()) {
            m_kinds.erase(m_kinds.begin() + static_cast<std::ptrdiff_t>(index));
        }
    }

    void FishTank::dropFood(int x) {
        for (TankFood& grain : m_foods) {
            if (!grain.active) {
                grain.active = true;
                grain.posX = x;
                grain.posY = 0;
                grain.fallSpeed = 1; // px per update
                return;
            }
        }
    }

    void FishTank::clearFood() {
        for (TankFood& grain : m_foods) {
            grain.active = false;
        }
    }

    void FishTank::steerToFood() {
        for (int index = 1; index <= kFishes; ++index) {
            TankFish& fish = m_fishes[static_cast<std::size_t>(index)];
            double nearest = 999; // larger than any distance in the form
            std::reference_wrapper<const TankFood> target = std::cref(m_foods[0]);
            for (const TankFood& grain : m_foods) {
                if (!grain.active || (index < 3 && fish.minY > grain.posY)) { // fish 1 and 2 (index < 3) keep to grains below their line
                    continue;
                }
                const double distance = std::sqrt(std::pow(std::abs(fish.posX - grain.posX), 2) + std::pow(std::abs(fish.posY - grain.posY), 2));
                if (nearest > distance) {
                    nearest = distance;
                    target = std::cref(grain);
                }
            }
            if (nearest < kScent) {
                fish.destX = target.get().posX;
                fish.destY = target.get().posY;
                fish.speedBonusX = kFoodBonus;
                fish.speedBonusY = kFoodBonus;
                if (fish.posX == target.get().posX) {
                    fish.facing = 0;
                }
            }
        }
    }

    void FishTank::eatInRect(const TankFish& fish, int index) {
        for (TankFood& grain : m_foods) {
            if (!grain.active || grain.posX < fish.posX || grain.posY < fish.posY || grain.posX > fish.posX + fish.frameWidth ||
                grain.posY > fish.posY + fish.frameHeight) {
                continue;
            }
            grain.active = false;
            m_hooks.ate(index);
        }
    }

    void FishTank::eatFood() {
        for (int index = 1; index <= kFishes; ++index) {
            const TankFish& fish = m_fishes[static_cast<std::size_t>(index)];
            if (fish.active) {
                eatInRect(fish, index);
            }
        }
    }

    void FishTank::fallFood() {
        for (TankFood& grain : m_foods) {
            if (!grain.active) {
                continue;
            }
            grain.posY += grain.fallSpeed;
            if (grain.posY > kBottom) {
                grain.active = false;
                m_hooks.landed();
            }
        }
    }

    void FishTank::update(std::uint64_t nowMs) {
        for (int index = 1; index <= kFishes; ++index) {
            m_fishes[static_cast<std::size_t>(index)].update(m_randomFunction, nowMs);
        }
        fallFood();
        for (std::size_t index = 1; index < m_bubbles.size(); ++index) {
            m_bubbles[index].update(m_randomFunction);
        }
        steerToFood();
        eatFood();
    }

    void FishTank::draw(gfx::Canvas& canvas, core::Point origin, core::Point formSize, std::uint64_t nowMs) {
        for (int index = 1; index <= kFishes; ++index) {
            m_fishes[static_cast<std::size_t>(index)].draw(canvas, origin, formSize.x, nowMs);
        }
        for (TankFood& grain : m_foods) {
            grain.draw(canvas, origin, formSize, nowMs);
        }
        for (std::size_t index = 1; index < m_bubbles.size(); ++index) {
            m_bubbles[index].draw(canvas, origin, nowMs);
        }
    }

}
