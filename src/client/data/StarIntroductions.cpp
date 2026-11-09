#include "client/data/StarIntroductions.h"

namespace client {

    std::string starIntroduction(int star) {
        switch (star) {
            case 1: return "JRole_5_2"; // star 1
            case 5: return "JRole_5_8"; // star 5
            case 6: return "JRole_5_4"; // star 6
            case 7: return "JRole_5_25"; // star 7
            case 8: return "JRole_5_1"; // star 8
            case 12: return "JRole_5_14"; // star 12
            case 14: return "JRole_5_12"; // star 14
            case 16: return "JRole_5_35"; // star 16
            case 19: // star 19 shares the picture of star 20
            case 20: return "JRole_5_10"; // star 20
            case 23: return "JRole_5_38"; // star 23
            case 24: return "JRole_5_24"; // star 24
            case 27: return "JRole_5_47"; // star 27
            default: return ""; // no picture
        }
    }

}
