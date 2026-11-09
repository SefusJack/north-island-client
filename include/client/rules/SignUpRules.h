#ifndef NORTH_ISLAND_CLIENT_RULES_SIGN_UP_RULES_H
#define NORTH_ISLAND_CLIENT_RULES_SIGN_UP_RULES_H

#include <optional>
#include <string>

namespace client {

    /// @brief Checks the qol client's sign-up form (the original opened a web page).
    /// @details The account is 4 to 14 letters and digits (the login form's and the server's rule); the password is 4 to 14 characters (the field holds 14), typed twice.
    /// @param account The account name.
    /// @param password The password.
    /// @param again The password typed a second time.
    /// @return What is wrong, for the message; nothing when the account may be asked for.
    std::optional<std::string> signUpProblem(const std::string& account, const std::string& password, const std::string& again);

}

#endif
