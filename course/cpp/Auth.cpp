#include "../h/Auth.h"
#include "../h/Storage.h"
#include "../h/IDGenerator.h"
#include <cstring>
#include <iostream>
#include "../h/SessionManager.h"

using namespace std;

bool Auth::registerUser(const std::string& u,
    const std::string& p,
    const std::string& email,
    const std::string& diet,
    const std::string& allergies) {
    if (u.empty() || p.empty() || email.empty()) {
        cout << "❌ Логін, пароль та email не можуть бути порожніми!\n";
        return false;
    }

    // Перевіряємо, чи користувач вже існує
    UserData existing;
    if (Storage::getUser(u, existing)) {
        return false; // вже є
    }

    UserData newUser{};
    newUser.id = std::stoi(IDGenerator::generate("user_id.txt"));
    strncpy_s(newUser.username, u.c_str(), sizeof(newUser.username) - 1);
    strncpy_s(newUser.password, p.c_str(), sizeof(newUser.password) - 1);
    strncpy_s(newUser.email, email.c_str(), sizeof(newUser.email) - 1);
    strncpy_s(newUser.diet, diet.c_str(), sizeof(newUser.diet) - 1);
    strncpy_s(newUser.allergies, allergies.c_str(), sizeof(newUser.allergies) - 1);

    newUser.role = 0; // звичайний користувач
	if (newUser.id == 1) {
        newUser.role = 1; // перший користувач - адмін
    }

    return Storage::saveUser(newUser);
}

bool Auth::login(const std::string& username, const std::string& password) {
    UserData user;
    if (Storage::getUser(username, user)) {
        return (password == user.password);
    }
    return false;
}