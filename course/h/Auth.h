#ifndef AUTH_H
#define AUTH_H
#include <cstring>
#include <string>

class Auth {
public:
    // Єдина правильна версія з 5 аргументами
 static bool registerUser(const std::string& u,
                             const std::string& p,
                             const std::string& email,
                             const std::string& diet,
                             const std::string& allergies);

    static bool login(const std::string& username, const std::string& password);
};

#endif
