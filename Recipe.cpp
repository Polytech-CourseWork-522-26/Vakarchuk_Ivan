#include "Recipe.h"
#include "Storage.h"
#include "IDGenerator.h"
#include <iostream>
#include <limits>

using namespace std;

void Recipe::showAll() {
    auto list = Storage::getAllRecipes();
    if (list.empty()) {
        cout << "\nРецептів поки немає.\n";
        return;
    }

    cout << "\n=== СПИСОК РЕЦЕПТІВ ===\n";
    for (int i = 0; i < list.size(); i++) {
        cout << i + 1 << ". " << list[i].name << " (Автор: " << list[i].author << ")\n";
    }

    int sel;
    cout << "\nОберіть номер для перегляду (0 - назад): ";
    if (!(cin >> sel) || sel == 0) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    if (sel > 0 && sel <= list.size()) {
        auto& res = list[sel - 1];
        cout << "\n-----------------------------------";
        cout << "\nНАЗВА: " << res.name;
        cout << "\nОПИС: " << res.description;
        cout << "\nКАЛОРІЇ: " << res.calories << " ккал";
        cout << "\nІНГРЕДІЄНТИ:\n" << res.ingredients;
        cout << "\nІНСТРУКЦІЯ:\n" << res.instructions;
        cout << "\n-----------------------------------\n";
        cout << "Натисніть Enter, щоб повернутися...";
        cin.ignore();
        cin.get();
    }
}
// =======================
// ➕ СТВОРЕННЯ
// =======================
void Recipe::createFlow(const string& user) {
    RecipeData r;
    r.id = stoi(IDGenerator::generate("recipe_id.txt"));

    // КРОК 1: Очищаємо буфер перед getline
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string temp;
    cout << "\n=== СТВОРЕННЯ РЕЦЕПТА ===\n";

    cout << " Назва: ";
    getline(cin, temp);
    strncpy_s(r.name, sizeof(r.name), temp.c_str(), _TRUNCATE);

    cout << " Опис: ";
    getline(cin, temp);
    strncpy_s(r.description, sizeof(r.description), temp.c_str(), _TRUNCATE);

    cout << " Інгредієнти (текстом): ";
    getline(cin, temp);
    strncpy_s(r.ingredients, sizeof(r.ingredients), temp.c_str(), _TRUNCATE);

    cout << " Спосіб приготування: ";
    getline(cin, temp);
    strncpy_s(r.instructions, sizeof(r.instructions), temp.c_str(), _TRUNCATE);

    cout << " Калорії: ";
    cin >> r.calories;

    // КРОК 2: Очищаємо буфер ПІСЛЯ введення числа
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    strncpy_s(r.author, sizeof(r.author), user.c_str(), _TRUNCATE);
    r.author_id = 0; // Можна додати пошук ID автора за потребою

    Storage::saveRecipe(r);

    cout << "\n✔ Рецепт збережено! Натисніть Enter для повернення в меню...";
    cin.get();
}
void Recipe::editFlow(const string& user) {
    auto list = Storage::getAllRecipes();

    if (list.empty()) {
        cout << "Немає рецептів\n";
        return;
    }

    cout << "\n=== РЕДАГУВАННЯ ===\n";

    for (int i = 0; i < list.size(); i++) {
        cout << i + 1 << ". " << list[i].name << endl;
    }

    int sel;
    cout << "Оберіть рецепт: ";
    cin >> sel;

    if (sel <= 0 || sel > list.size()) return;

    auto r = list[sel - 1];

    if (string(r.author) != user) {
        cout << "❌ Ви не автор\n";
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string temp;

    cout << "👉 Новий опис: ";
    getline(cin, temp);
    strcpy_s(r.description, temp.c_str());

    cout << "👉 Нові калорії: ";
    cin >> r.calories;

    Storage::updateRecipe(r);

    cout << "✔ Оновлено\n";
}

void Recipe::deleteFlow(const string& user) {
    auto list = Storage::getAllRecipes();

    if (list.empty()) {
        cout << "Немає рецептів\n";
        return;
    }

    cout << "\n=== ВИДАЛЕННЯ ===\n";

    for (int i = 0; i < list.size(); i++) {
        cout << i + 1 << ". " << list[i].name << endl;
    }

    int sel;
    cout << "Оберіть рецепт: ";
    cin >> sel;

    if (sel <= 0 || sel > list.size()) return;

    auto r = list[sel - 1];

    if (string(r.author) != user) {
        cout << "❌ Ви не автор\n";
        return;
    }

    Storage::deleteRecipe(r.id);

    cout << "✔ Видалено\n";
}
