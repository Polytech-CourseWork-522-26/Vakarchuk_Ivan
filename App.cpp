#include "App.h"
#include "Auth.h"
#include "Recipe.h"
#include "IDGenerator.h"
#include "IngredientManager.h"
#include <iostream>
#include <limits>
#include "Storage.h"

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
            string u, p, e, d, a;
            cout << "\n--- РЕЄСТРАЦІЯ ---" << endl;
            cout << "Логін: "; cin >> u;
            cout << "Пароль: "; cin >> p;
            cout << "Email: "; cin >> e;

            // Використовуємо getline, щоб можна було вводити фрази (напр. "Без обмежень")
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Дієта (напр. Vegan, Keto): ";
            getline(cin, d);
            if (d.empty()) d = "None";

            cout << "Алергії (через кому, напр. Томати, Арахіс): ";
            getline(cin, a);
            if (a.empty()) a = "None";

            Auth auth;
            if (auth.registerUser(u, p, e, d, a)) // Передаємо всі 5 параметрів
                cout << "\n✔ Реєстрація успішна! Тепер увійдіть.\n";
            else
                cout << "\n❌ Користувач вже існує.\n";
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
    Auth auth;
    if (auth.login(currentUserName, p)) {
        // Отримуємо дані користувача, щоб знати його роль
        user = Storage::getUserByName(currentUserName);
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

        if (!(cin >> c)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (c) {
        case 1: r.showAll(false, currentUserName); break;
        case 2: r.searchByIngredient(); break;
        case 3: r.showFavorites(currentUserName); break;
        case 4: r.createFlow(currentUserName); break;
        case 5: r.shoppingListFlow(currentUserName); break;
        }
    } while (c != 6);
}

void App::adminMenu() {
    int choice;
    do {
        cout << "\n--- ПАНЕЛЬ КЕРУВАННЯ АДМІНІСТРАТОРА (" << currentUserName << ") ---\n";
        cout << "1. ➕ Додати інгредієнт\n";
        cout << "2. 🍳 Створити новий рецепт\n";
        cout << "3. 📝 Редагувати / Видалити рецепти\n";
        cout << "4. 👥 Модерація користувачів\n";
        cout << "5. 📊 Статистика системи\n";
        cout << "0. Вихід\n>> ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
        case 1: {
            IngredientManager im;
            im.addIngredientFlow();
            break;
        }
        case 2: {
            RecipeData newR;
            newR.id = std::stoi(IDGenerator::generate("recipe_id.txt"));

            system("cls");
            cout << "=== АДМІНІСТРАТОР: СТВОРЕННЯ РЕЦЕПТУ ===\n";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Назва страви: ";
            cin.getline(newR.name, 100);

            cout << "Короткий опис: ";
            cin.getline(newR.description, 255);

            cout << "Покрокова інструкція: ";
            cin.getline(newR.instructions, 500);

            cout << "Калорійність: ";
            cin >> newR.calories;

            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Теги (напр. Vegan, Fast): ";
            cin.getline(newR.tags, 100);

            strncpy_s(newR.author, "Admin", _TRUNCATE);

            Recipe r;
            r.adminAddRecipe(newR);
            break;
        }
        case 3: {
            Recipe r;
            r.showAll(true, currentUserName);
            break;
        }
        case 4: {
            manageUsers();
            break;
        }
        case 5: {
            Storage::showGlobalStatistics();
            break;
        }
        }
    } while (choice != 0);
}

void App::manageUsers() {
    system("cls");
    auto users = Storage::getAllUsers();

    cout << "===== КЕРУВАННЯ КОРИСТУВАЧАМИ =====\n\n";
    for (int i = 0; i < (int)users.size(); i++) {
        cout << i + 1 << ". " << users[i].username
            << " | Дієта: " << users[i].diet
            << " | Алергії: " << users[i].allergies
            << " | Email: " << users[i].email
            << " | Роль: " << (users[i].role == 1 ? "Адмін" : "Користувач")
            << endl;
    }

    cout << "\nОберіть дію:\n1. Видалити користувача\n2. Змінити роль\n0. Назад\n>> ";
    int action; cin >> action;
    if (action == 1) {
        cout << "Введіть ім'я користувача для видалення: ";
        string uname; cin >> uname;
        Storage::deleteUser(uname);
    }
    else if (action == 2) {
        cout << "Введіть ім'я користувача для зміни ролі: ";
        string uname; cin >> uname;
        auto users = Storage::getAllUsers();
        for (auto& u : users) {
            if (uname == u.username) {
                u.role = (u.role == 0 ? 1 : 0); // переключаємо роль
                Storage::updateUser(u); 
                cout << "✅ Роль змінено!\n";
                break;
            }
        }
    }

    system("pause");
}

