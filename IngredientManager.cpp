#include "IngredientManager.h"
#include "Storage.h"
#include "IDGenerator.h"
#include <iostream>
#include <limits>

using namespace std;

void IngredientManager::addIngredientFlow() {
    IngredientData ing;
    ing.id = stoi(IDGenerator::generate("ing_id.txt"));

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string temp;

    cout << "\n--- ДОДАВАННЯ НОВОГО ІНГРЕДІЄНТА (Адмін) ---\n";
    cout << "Назва продукту: ";
    getline(cin, temp);
    strncpy_s(ing.name, sizeof(ing.name), temp.c_str(), _TRUNCATE);

    cout << "Виробник: ";
    getline(cin, temp);
    strncpy_s(ing.manufacturer, sizeof(ing.manufacturer), temp.c_str(), _TRUNCATE);

    cout << "Категорія (овочі/м'ясо/бакалія...): ";
    getline(cin, temp);
    strncpy_s(ing.category, sizeof(ing.category), temp.c_str(), _TRUNCATE);

    cout << "Одиниці виміру (г, мл, шт): ";
    cin >> ing.unit;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    Storage::saveIngredient(ing);
    cout << "✔ Продукт додано в базу інгредієнтів!\n";
}

void IngredientManager::showAllIngredients() {
    auto list = Storage::getAllIngredients();
    cout << "\n=== Список доступних продуктів ===\n";
    for (const auto& ing : list) {
        cout << ing.id << ". " << ing.name << " (" << ing.category << ") - " << ing.unit << endl;
    }
}
