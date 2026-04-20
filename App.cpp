#include "App.h"
#include "Auth.h"
#include "Recipe.h"
#include <iostream>  
using namespace std;

void App::run() {
    startMenu();

    if (!login()) {
        cout << "❌ Невірні дані\n";
        return;
    }

    menu();
}


void App::startMenu() {
    int choice;
    string u, p;

    cout << "1. Реєстрація\n2. Вхід\n>> ";
    cin >> choice;

    if (choice == 1) {
        cout << "Username: ";
        cin >> u;

        cout << "Password: ";
        cin >> p;

        if (Auth::registerUser(u, p))
            cout << "✔ Зареєстровано\n";
        else
            cout << "❌ Помилка\n";
    }
}



bool App::login() {
    string p;

    cout << "\n=== ВХІД ===\n";

    cout << "Username: ";
    cin >> currentUser;

    cout << "Password: ";
    cin >> p;

    return Auth::login(currentUser, p);
}
void App::menu() {
    Recipe r;
    int c;

    do {
        cout << "\n===== МЕНЮ =====\n";
        cout << "1. 📖  Показати рецепти\n";
        cout << "2. ➕  Додати рецепт\n";
        cout << "3. ✏️   Редагувати рецепт\n";
        cout << "4. ❌ Видалити рецепт\n";
        cout << "5. Вихід\n>> ";

        cin >> c;

        switch (c) {
        case 1: r.showAll(); break;
        case 2: r.createFlow(currentUser); break;
        case 3: r.editFlow(currentUser); break;
        case 4: r.deleteFlow(currentUser); break;
        }

    } while (c != 5);
}