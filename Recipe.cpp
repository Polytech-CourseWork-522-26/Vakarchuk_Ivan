#include "Recipe.h"
#include "Storage.h"
#include "IDGenerator.h"
#include <iostream>
#include <limits>

using namespace std;

void Recipe::showAll() {
    auto list = Storage::getAllRecipes();

    if (list.empty()) {
        cout << "Немає рецептів\n";
        return;
    }

    while (true) {
        cout << "\n=== СПИСОК РЕЦЕПТІВ ===\n";
        cout << "1. 🔙 Назад\n";

        for (int i = 0; i < list.size(); i++) {
            cout << i + 2 << ". " << list[i].name << endl;
        }

        int sel;
        cout << "Оберіть: ";
        cin >> sel;

        if (sel == 1) return;

        if (sel >= 2 && sel <= list.size() + 1) {
            auto& r = list[sel - 2];

            cout << "\n===== РЕЦЕПТ: " << r.name << " =====\n";
            cout << "Автор: " << r.author << endl;
            cout << "Опис: " << r.description << endl;
            cout << "-----------------------------------\n";
            cout << "🍎 ІНГРЕДІЄНТИ:\n" << r.ingredients << endl;
            cout << "-----------------------------------\n";
            cout << "👨‍🍳 ПРИГОТУВАННЯ:\n" << r.instructions << endl;
            cout << "-----------------------------------\n";
            cout << "🔥 Калорійність: " << r.calories << " ккал" << endl;

            cout << "\nНатисніть Enter, щоб повернутися...";
            cin.ignore();
            cin.get();
        }
    }
}

// =======================
// ➕ СТВОРЕННЯ
// =======================
void Recipe::createFlow(const string& user) {
    RecipeData r;
    r.id = stoi(IDGenerator::generate("recipe_id.txt"));

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string temp;

    cout << "\n=== СТВОРЕННЯ РЕЦЕПТА ===\n";

    cout << " Назва: ";
    getline(cin, temp);
    strcpy_s(r.name, temp.c_str());

    cout << " Короткий опис: ";
    getline(cin, temp);
    strcpy_s(r.description, temp.c_str());

    cout << " Інгредієнти: ";
    getline(cin, temp);
    strcpy_s(r.ingredients, temp.c_str());

    cout << " Спосіб приготування: ";
    getline(cin, temp);
    strcpy_s(r.instructions, temp.c_str());

    cout << " Калорії: ";
    cin >> r.calories;

    strcpy_s(r.author, user.c_str());

    Storage::saveRecipe(r);
    cout << "✔ Рецепт збережено!\n";
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