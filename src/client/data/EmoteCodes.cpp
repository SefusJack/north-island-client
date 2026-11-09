#include "client/data/EmoteCodes.h"

#include <array>

namespace client {

    namespace {

        const std::array<std::string, kEmoteCount + 1> kCodes = {"",   "@|", ":$",  ">|", "-\"", "XD", "TT", ":D", ":S", "==", "8D",
                                                                 "%#", "8(", ":)",  "^T", ">@",  ":(", ":!", "JL", "8o", ":[", ":m",
                                                                 ":o", "|T", "\"_", "*:", "j-",  ":P", "|E", "zZ", "@#", "-P"};

        const std::array<std::string, kEmoteCount + 1> kNames = {
            "",
            "Arrogant",
            "Shy",
            "V-style eyebrow",
            "drawing a line",
            "XD",
            "weeping",
            "laugh",
            "doubt",
            "sweating",
            "laugh a hearty laugh",
            "Bright eyes",
            "a bit unusual",
            "Smile",
            "burst into tears",
            "Angry",
            "in bad temper",
            "smoking",
            "cross your fingers",
            "be stupefied",
            "To be alone",
            "whistle",
            "stand aghast",
            "spit out",
            "Looking like at the bottom of fortune's wheel",
            "want to kill someone",
            "feel deeply ashamed",
            "put out tongue",
            "breathe out",
            "dozing off",
            "chaos",
            "Nod"
        };

    }

    const std::string& emoteCode(int number) { return kCodes[number >= 1 && number <= kEmoteCount ? static_cast<std::size_t>(number) : 0]; }

    const std::string& emoteName(int number) { return kNames[number >= 1 && number <= kEmoteCount ? static_cast<std::size_t>(number) : 0]; }

    int emoteAt(const std::string& text, std::size_t pos) {
        if (pos + 1 >= text.size()) {
            return 0;
        }
        for (int number = 1; number <= kEmoteCount; ++number) {
            const std::string& code = kCodes[static_cast<std::size_t>(number)];
            if (text[pos] == code[0] && text[pos + 1] == code[1]) {
                return number;
            }
        }
        return 0;
    }

}
