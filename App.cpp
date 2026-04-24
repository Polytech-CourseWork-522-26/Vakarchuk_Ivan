#include "App.h"
#include "Auth.h"
#include "Recipe.h"
#include "IngredientManager.h" // Потрібно буде створити на Кроці 2
#include <iostream>
#include <limits>

using namespace std;

void App::run() {
    startMenu();

    if (currentUserName.empty()) return;

    // Розгалуження в залежності від ролі
    if (currentUserRole == 1) {
        adminMenu();
    }
    else {
        userMenu();
    }
}

void App::startMenu() {
    int choice;
    while (true) {
        cout << "\n=== ВІТАЄМО У СВІТІ КУЛІНАРІЇ ===\n";
        cout << "1. Реєстрація\n2. Вхід\n3. Вихід\n>> ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 3) exit(0);

        if (choice == 1) {
            string u, p, e, d;
            cout << "Username: "; cin >> u;
            cout << "Password: "; cin >> p;
            cout << "Email: "; cin >> e;
            cout << "Дієта (веган/м'ясоїд/інше): "; cin >> d;

            if (Auth::registerUser(u, p, e, d))
                cout << "✔ Реєстрація успішна! Тепер увійдіть.\n";
            else
                cout << "❌ Користувач вже існує.\n";
        }
        else if (choice == 2) {
            if (login()) return;
            else cout << "❌ Невірний логін або пароль.\n";
        }
    }
}

bool App::login() {
    string p;
    cout << "\n=== ВХІД ===\n";
    cout << "Username: "; cin >> currentUserName;
    cout << "Password: "; cin >> p;

    UserData user;
    if (Auth::login(currentUserName, p)) {
        // Отримуємо дані користувача, щоб знати його роль
        Storage::getUser(currentUserName, user);
        currentUserRole = user.role;
        return true;
    }
    return false;
}

void App::userMenu() {
    Recipe r;
    int c;
    do {
        cout << "\n===== МЕНЮ КОРИСТУВАЧА (" << currentUserName << ") =====\n";
        cout << "1. 📖 Переглянути всі страви\n";
        cout << "2. 🔍 Пошук страв за інгредієнтами\n";
        cout << "3. ⭐ Улюблені страви\n";
        cout << "4. ➕ Створити свій рецепт\n";
        cout << "5. 🛒 Сформувати список покупок\n";
        cout << "6. Вихід\n>> ";

        cin >> c;
        switch (c) {
        case 1: r.showAll(); break;
        case 4: r.createFlow(currentUserName); break;
            // Інші кейси додамо на Кроках 2 та 3
        }
    } while (c != 6);
}

void App::adminMenu() {
    Recipe r;
    int c;
    do {
        cout << "\n===== ПАНЕЛЬ АДМІНІСТРАТОРА (" << currentUserName << ") =====\n";
        cout << "1. 📖 Всі рецепти (Модерація)\n";
        cout << "2. 🍏 Керування інгредієнтами (Додати/Видалити)\n";
        cout << "3. 📊 Статистика системи\n";
        cout << "4. 👥 Керування користувачами\n";
        cout << "5. Вихід\n>> ";

        cin >> c;
        switch (c) {
        case 1: r.showAll(); break;
            // Кейс 2 вестиме до IngredientManager
        }
    } while (c != 5);
}
}
