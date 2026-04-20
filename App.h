#ifndef APP_H
#define APP_H

#include <string>

class App {
public:
    void run();

private:
    std::string currentUser;

    void startMenu();
    bool login();
    void menu();
};

#endif