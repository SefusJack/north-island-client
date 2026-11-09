#include "client/rules/SignUpRules.h"

#include <algorithm>
#include <cctype>

namespace client {

    std::optional<std::string> signUpProblem(const std::string& account, const std::string& password, const std::string& again) {
        const bool lettersAndDigits = std::all_of(account.begin(), account.end(), [](char c) { return std::isalnum(static_cast<unsigned char>(c)) != 0; });
        if (account.size() < 4 || account.size() > 14 || !lettersAndDigits) { // account length 4 to 14
            return "The account needs 4 to 14 letters or digits.";
        }
        if (password.size() < 4 || password.size() > 14) { // password length 4 to 14
            return "The password needs 4 to 14 characters.";
        }
        if (password != again) {
            return "The two passwords are not the same.";
        }
        return std::nullopt;
    }

}
