#include "client/rules/RebornWishPrice.h"

#include <array>

namespace client {

    namespace {

        // star cost indexed by the pet's grow key 0..50 (0 = cannot be wished for)
        constexpr std::array<int, 51> kStarCosts = {0, 2, 4, 6, 5, 8, 6, 0, 4, 6, 7, 5, 10, 0, 0, 0, 7, 8, 0, 0, 0, 0, 7, 0, 0, 0,
                                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        /// @brief One PetCareerTable row: the pet, the pet it becomes and the mark of the career.
        struct Career {
            int oldNpc;
            int careerNpc;
            int markId;
        };

        constexpr std::array<Career, 13> kCareers = {
            // {old pet NPC, career pet NPC, rebirth mark}
            {{14156, 31027, 50363}, // Xaolan; mark 50363 (flag 673)
             {14081, 31026, 50365}, // Niss; mark 50365 (flag 674)
             {14162, 31025, 50367}, // Roca; mark 50367 (flag 675)
             {14175, 31028, 50369}, // Clive; mark 50369 (flag 676)
             {14615, 31035, 50371}, // Qlaya; mark 50371 (flag 677)
             {12068, 31037, 50373}, // Magellan; mark 50373 (flag 688)
             {14609, 31034, 50377}, // Shasha; mark 50377 (flag 694)
             {14230, 31029, 50383}, // Elin; mark 50383 Elin's Rebirth (flag 772)
             {15689, 31040, 50385}, // Hayate; mark 50385 Hayate's Rebirth (flag 792)
             {14548, 31033, 50375}, // Kanako; mark 50375 (flag 693)
             {12095, 31038, 50387}, // Frederico; mark 50387 Fred's Rebirth (flag 800)
             {15657, 31039, 50389}, // Victoria; mark 50389 Victoria's Rebirth (flag 841)
             {14274, 31032, 50391}} // Sam; mark 50391 Sam's Rebirth (flag 842)
        };

    }

    int rebornStarCost(int growKey) {
        return growKey >= 0 && growKey < static_cast<int>(kStarCosts.size()) ? kStarCosts[static_cast<std::size_t>(growKey)] : 0;
    }

    int petCareerMarkId(int petNpcId) {
        int careerNpc = 0;
        for (const Career& career : kCareers) {
            if (career.oldNpc == petNpcId) {
                careerNpc = career.careerNpc;
                break;
            }
        }
        if (careerNpc == 0) {
            return 0;
        }
        for (const Career& career : kCareers) {
            if (career.oldNpc == careerNpc || career.careerNpc == careerNpc) {
                return career.markId;
            }
        }
        return 0;
    }

}
