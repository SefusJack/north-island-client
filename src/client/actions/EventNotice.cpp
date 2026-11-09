#include "client/actions/EventNotice.h"

#include <array>

namespace client {

    namespace {

        // rows: {sub, choice (-1: none, 0: any other), kind, duration in ms (0 for chat), text} of action 20 subs 24..61
        const std::array<EventNotice, 54> kNotices{{
            // sub 24..29: Capture the Evil Corton event (24 captured, 25 schedule, 27 reward, 28 over, 29 begins)
            {24, -1, EventNotice::Kind::Short, 2000, "The Corton has been captured."},
            {25, -1, EventNotice::Kind::Short, 2000, "8:00pm-10:00pm in every Friday, players can take part in the activity of \"Capture the Evil Corton\"."},
            {27, -1, EventNotice::Kind::Short, 2000, "You have obtianed rewards. Don't hold me any longer."},
            {28, -1, EventNotice::Kind::Chat, 0, "Dear players, the Capture the Evil Corton activity is over now."},
            {29, -1, EventNotice::Kind::Chat, 0, "Dear players, the Capture the Evil Corton activity is beginning now."},
            // sub 30 (Big5): you can enter only during the Dec 24-27 Christmas event
            {30, -1, EventNotice::Kind::Short, 2000, "Only open during the Christmas event, December 24 to 27."},
            // sub 32: Arena Competition not open yet
            {32, -1, EventNotice::Kind::Short, 2000, "You can only compete with others when the Arena Competition is opened."},
            // sub 35 (Big5): you can enter only during the Lantern Festival event, Feb 5 19:00 to Feb 7 19:00
            {35, -1, EventNotice::Kind::Short, 2000, "Only open during the Lantern Festival event, February 5 19:00 to February 7 19:00."},
            // sub 36: Lantern Festival; choice 1 (Big5): Bohai Ruins in Jizhou open for the first time; choice 2: festival ending soon
            {
                36,
                1,
                EventNotice::Kind::Chat,
                0,
                "Dear players, the Bohai Ruins in Jizhou open for the first time this Lantern Festival. What strange monsters and treasures await? Go and find out!"
            },
            {36, 2, EventNotice::Kind::Chat, 0, "Dear players, the Lantern Festival will be over soon."},
            // sub 37: Valentine's Day Flower Delivery event times
            {37,
             -1,
             EventNotice::Kind::Short,
             2000,
             "Our Valentines Day \"Flower Delivery\" activity will begin at 19:00 on Feb.13 and last until 24:00 on Feb.14."},
            // sub 38: Chinese Valentine's Day event; choice 1 begins, 2 over, 3 ends in 1 hour
            {38, 1, EventNotice::Kind::Chat, 0, "My dear players, come and join in our Chinese Valentine's Day event"},
            {38, 2, EventNotice::Kind::Chat, 0, "My dear players, our Chinese Valentine's Day event is over. Thanks for your support"},
            {38, 3, EventNotice::Kind::Chat, 0, "My dear players, our Chinese Valentine's Day event will be ending in 1 hour. Try your best to finish soon"},
            // sub 39: competition not begun
            {39, -1, EventNotice::Kind::Short, 2000, "The competition doesn't begin now."},
            // sub 40: Arena Competition; choice 1 begins, 2 over
            {40, 1, EventNotice::Kind::Chat, 0, "The Arena Competition begins now."},
            {40, 2, EventNotice::Kind::Chat, 0, "The Arena Competition is over now."},
            // sub 41: tests over (choice 1); choices 2, 3 (Big5): Rescue the Two Qiaos of Jiangdong event started / ended
            {41, 1, EventNotice::Kind::Short, 2000, "Today's tests are over."},
            {41,
             2,
             EventNotice::Kind::Chat,
             0,
             "The Rescue the Two Qiaos event has begun. Players, please hurry to the Bronze Sparrow Terrace in Yecheng to join the rescue!"},
            {41, 3, EventNotice::Kind::Chat, 0, "The Rescue the Two Qiaos event is over. You are welcome to take on the challenge again next time."},
            // sub 43 (Big5): Hinet contest sign-up closed on 7/17
            {43, -1, EventNotice::Kind::Short, 2000, "Registration for the Hinet tournament ended on 7/17. Thanks for your support."},
            // sub 45: Graduation Challenge sign-up time
            {45, -1, EventNotice::Kind::Short, 5000, "Sign up time for Graduation Challenge: \rfrom 0:00 on Dec 4th to 22:00 on Dec 5th"},
            // sub 47: vehicle refusals; choice 1 no sea/air vehicle, 2 water, 3 space, 4 shape shifted, 0 other
            {47, 1, EventNotice::Kind::Short, 2000, "No sea or air vehicles"},
            {47, 2, EventNotice::Kind::Short, 2000, "No vehicle can be used to travel on water"},
            {47, 3, EventNotice::Kind::Short, 2000, "There is no space vehicle"},
            {47, 4, EventNotice::Kind::Short, 2000, "If you have shape shifted you will be unable to ride a vehicle."},
            {47, 0, EventNotice::Kind::Short, 2000, "No appropriate vehicle"},
            // sub 49: Graduation Challenge activity times
            {
                49,
                -1,
                EventNotice::Kind::Short,
                5000,
                "The acitivity time for the challenge graduation is: \rFrom December 4th 19:00 to December 5th 22:00\rFrom December 5th 19:00 to December 5th 22:00"
            },
            // sub 50..52: registration failures (50 failed, 51 and 52 no space for prizes)
            {50, -1, EventNotice::Kind::Short, 3000, "You or your team member failed to register"},
            {51, -1, EventNotice::Kind::Short, 3000, "You or your team members don't have enough space to receive prizes, Unable to complete registration"},
            {52, -1, EventNotice::Kind::Short, 3000, "You or your team members don't have enough space to receive prizes"},
            // sub 55: 20 Round Challenge; choice 1 started, 2 finished, 3 schedule
            {
                55,
                1,
                EventNotice::Kind::Chat,
                0,
                "The 20 Round Challenge has started, Show us the result of your hardwork, To attend go click the Challenge Sign-up on 4F, the Capitol of Welling Village"
            },
            {55, 2, EventNotice::Kind::Chat, 0, "The activity is finished Welcome to challenge next time, Thanks for your support and participation"},
            {55, 3, EventNotice::Kind::Short, 4000, "The 20 Round Challenge is: \rFrom 7:00PM to 9:00PM Friday every week"},
            // sub 56: National Treasure event; 1 begins, 2 over, 3 schedule, 4 suspended, 5 new rewards, 6 ended
            {56,
             1,
             EventNotice::Kind::Chat,
             0,
             "The National Treasure event is beginning. Players Level 10+ please go to the 4F,the Capitol in Welling and register.     Join in the event"},
            {56,
             2,
             EventNotice::Kind::Chat,
             0,
             "The National Treasure event is over\xA1"
             "A'Welcome to come to again\xA1"
             "AThanks for your support"},
            {56, 3, EventNotice::Kind::Short, 4000, "The time of National Treasure event is\rDuring 7:00PM-9:00 PM  every Monday"},
            {56, 4, EventNotice::Kind::Short, 4000, "This weeks treasure event is suspended"},
            {56, 5, EventNotice::Kind::Chat, 0, "More great new rewards from National Treasure!  Come join in now for some great prizes!"},
            {56, 6, EventNotice::Kind::Chat, 0, "National Treasure has ended! Good luck to everyone next year!"},
            // sub 57: New Year Event 2008-2009; 1 info, 2 ended, 3 countdown in 3 min, 4 in 1 min, 5 last 10 seconds
            {
                57,
                1,
                EventNotice::Kind::Chat,
                0,
                "The New Year Event will be from 2:00 am, December 31st, 2008 to 11:00 pm, January 11th, 2009. During the event, players will obtain the New Year gift box by killing level 10+ monsters. Also, you may be lucky enough to get the Bless Pack and Super Bless Pack for super rewards."
            },
            {57, 2, EventNotice::Kind::Chat, 0, "The New Year Event has ended, thanks all participants. Please don't forget to redeem your rewards!"},
            {57,
             3,
             EventNotice::Kind::Chat,
             0,
             "New year countdown activity will begin in 3 minutes, all participants can obtain some gifts, hope all players can enjoy."},
            {57, 4, EventNotice::Kind::Chat, 0, "New year countdown activity will begin in 1 minute, are you ready?"},
            {57, 5, EventNotice::Kind::Chat, 0, "The last 10 seconds! Let's count down together..."},
            // sub 58: function not available
            {58, 4, EventNotice::Kind::Short, 4000, "At present, the function is not available"},
            // sub 60: 12 Golden Palaces activity; 1 started, 2 ended, 3 schedule, 4 ended without redeeming rewards
            {60, 1, EventNotice::Kind::Chat, 0, "12 Golden Palaces activity has started! Hope players join us actively"},
            {60,
             2,
             EventNotice::Kind::Chat,
             0,
             "The 12 Golden Palaces activity has ended, thanks for your participation\xA1"
             "I"},
            {60, 3, EventNotice::Kind::Short, 3000, "The start time of 12 Golden Palaces activity is: \r12:00 until 24:00 every sunday"},
            {60, 4, EventNotice::Kind::Short, 3000, "The 12 Golden Palaces activity has ended\rYou haven't redeemed your rewards"},
            // sub 61: seasonal events; 1 and 2 Cool Down in the Summertime, 3 and 4 New Year 2014 celebrations
            {
                61,
                1,
                EventNotice::Kind::Chat,
                0,
                "From 3:00am, August 13th until 1:00am, August 27th EST (G M T-5), players that kill monsters of level 10 or higher have a chance at getting one of two different ice cubes, the Crystal Ice Cube and Unclear Ice Cube! Click the ice cube to get a random item! Enjoy the summer by having a cold blast!"
            },
            {61, 2, EventNotice::Kind::Chat, 0, "The \"Cool Down in the Summertime\" event has finished, thanks for helping make it fun!"},
            {61,
             3,
             EventNotice::Kind::Chat,
             0,
             "The New Year celebrations will be held from December 31, 2am, 2013 to January 7, 2am, 2014 EST(G M T-5)! Take down monsters levels 10 or higher for a chance to receive New Year goodies, not to mention the ultra-rare 2014 New Year Pack! Open the pack for loadsful of surprises!"},
            {61,
             4,
             EventNotice::Kind::Chat,
             0,
             "The New Year celebrations has ended! Thank you for your active participation! Don't forget to open your 2014 New Year Packs for your attractive rewards!"},
        }};

    }

    std::optional<std::reference_wrapper<const EventNotice>> EventNotice::find(int sub, int choice) {
        std::optional<std::reference_wrapper<const EventNotice>> fallback;
        for (const EventNotice& notice : kNotices) {
            if (notice.sub != sub) {
                continue;
            }
            if (notice.choice == -1 || notice.choice == choice) {
                return notice;
            }
            if (notice.choice == 0) {
                fallback = notice;
            }
        }
        return fallback;
    }

}