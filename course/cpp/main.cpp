#include <fstream>
#include <windows.h>
#include <iostream>
#include "App.h"
#include <clocale>
int main() {
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    setlocale(LC_ALL, "Ukrainian");
    // Для коректної роботи з потоками виводу C++
    std::locale::global(std::locale(".UTF-8"));

    App app;
    app.run();
    return 0;
}