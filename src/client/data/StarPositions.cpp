#include "client/data/StarPositions.h"

#include <map>

namespace client {

    namespace {

        // table: layout slot -> (x, y) pixel position of the star in the sky
        // (slots 1..8 are on page 1, 9..14 page 2, 15..20 page 3, 21..26 page 4, 27..28 page 5; pages are 400 px wide, so x values
        // can be negative or beyond 400 and the caller shifts them by the page)
        const std::map<int, core::Point> kLayout = {{1, {220, 50}},    {2, {60, 160}},    {3, {180, 350}},   {4, {470, 70}},    {5, {600, 170}},
                                                    {6, {480, 300}},   {7, {-240, 300}},  {8, {-40, 350}},   {9, {100, 0}},     {10, {130, 220}},
                                                    {11, {80, 400}},   {12, {1300, 150}}, {13, {1340, 200}}, {14, {1176, 78}},  {15, {110, 0}},
                                                    {16, {370, 360}},  {17, {50, 400}},   {18, {2120, 50}},  {19, {-300, 280}}, {20, {660, 55}},
                                                    {21, {60, 0}},     {22, {110, 220}},  {23, {810, 210}},  {24, {860, 70}},   {25, {3240, 200}},
                                                    {26, {3140, 350}}, {27, {2650, 284}}, {28, {30, 220}}};

    }

    core::Point StarPositions::layout(int slot) {
        const auto found = kLayout.find(slot);
        return found == kLayout.end() ? core::Point{-1, -1} : found->second; // (-1, -1) = no such slot
    }

}
