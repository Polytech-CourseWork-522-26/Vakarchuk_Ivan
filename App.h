#ifndef APP_H
#define APP_H

#include <string>
#include "Storage.h"

class App {
public:
    void run();

private:
    std::string currentUserName;
    int currentUserRole; // 0 - Користувач, 1 - Адмін
    void manageUsers();
    void startMenu();
    bool login();
    void userMenu();  // Меню для звичайного користувача
    void adminMenu(); // Панель адміністратора
};

#endif