#ifndef AUTH_H
#define AUTH_H

#include <string>

class Auth {
public:
    // Єдина правильна версія з 5 аргументами
    bool registerUser(const std::string& username,
        const std::string& password,
        const std::string& email,
        const std::string& diet,
        const std::string& allergies);

    static bool login(const std::string& username,
        const std::string& password);
};

#endif
