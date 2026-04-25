#include "Recipe.h"
#include "RecipeStruct.h"
#include "Storage.h"
#include "IDGenerator.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <limits>
#include <map>

using namespace std;

// --- 1. Створення рецепта ---
void Recipe::createFlow(const string& user) {
    RecipeData r;
    // Використовуємо stoi для конвертації згенерованого string ID в int
    r.id = stoi(IDGenerator::generate("recipe_id.txt"));

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string temp;

    cout << "\n=== СТВОРЕННЯ НОВОЇ СТРАВИ ===\n";
    cout << "Назва страви: ";
    getline(cin, temp);
    strncpy_s(r.name, sizeof(r.name), temp.c_str(), _TRUNCATE);

    cout << "Короткий опис: ";
    getline(cin, temp);
    strncpy_s(r.description, sizeof(r.description), temp.c_str(), _TRUNCATE);

    auto allIngredients = Storage::getAllIngredients();

    if (allIngredients.empty()) {
        cout << "\n⚠ База інгредієнтів порожня. Додайте продукти через адмін-панель.\n";
    }
    else {
        char addMore;
        do {
            cout << "\nДоступні інгредієнти:\n";
            for (int i = 0; i < (int)allIngredients.size(); i++) {
                cout << i + 1 << ". " << allIngredients[i].name << " (" << allIngredients[i].unit << ")\n";
            }

            int choice;
            cout << "Оберіть номер продукту (0 - завершити): ";
            cin >> choice;

            if (choice > 0 && choice <= (int)allIngredients.size()) {
                DishIngredient rel;
                rel.dish_id = r.id;
                rel.ingredient_id = allIngredients[choice - 1].id;

                cout << "Введіть кількість (" << allIngredients[choice - 1].unit << "): ";
                cin >> rel.quantity;

                Storage::saveDishRelation(rel);
                cout << "✅ Додано.\n";
            }
            else break;

            cout << "Додати ще? (y/n): ";
            cin >> addMore;
        } while (addMore == 'y' || addMore == 'Y');
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Інструкція: ";
    getline(cin, temp);
    strncpy_s(r.instructions, sizeof(r.instructions), temp.c_str(), _TRUNCATE);

    cout << "Калорії: ";
    cin >> r.calories;
    strncpy_s(r.author, sizeof(r.author), user.c_str(), _TRUNCATE);

    Storage::saveRecipe(r);
    cout << "✔ Збережено!\n";
}

// --- 2. Перегляд усіх та управління рецептом ---
void Recipe::showAll(bool isAdmin, const string& currentUser) {
    auto recipes = Storage::getAllRecipes();
    auto allIngredients = Storage::getAllIngredients();

    // Отримуємо дані поточного користувача
    UserData user = Storage::getUserByName(currentUser);

    if (recipes.empty()) {
        cout << "\nРецептів поки немає.\n";
        return;
    }

    cout << "\n========== СПИСОК СТРАВ ==========\n";
    for (int i = 0; i < (int)recipes.size(); i++) {
        bool hasAllergy = false;
        auto relations = Storage::getRelationsByDish(recipes[i].id);

        // Перевірка кожного інгредієнта страви на алергію
        for (auto& rel : relations) {
            for (auto& ing : allIngredients) {
                if (ing.id == rel.ingredient_id) {
                    if (string(user.allergies).find(ing.name) != string::npos) {
                        hasAllergy = true;
                        break;
                    }
                }
            }
            if (hasAllergy) break;
        }

        // Перевірка дієти (наприклад, якщо користувач вегетаріанець)
        bool dietMismatch = false;
        if (string(user.diet).find("vegetarian") != string::npos) {
            for (auto& rel : relations) {
                for (auto& ing : allIngredients) {
                    if (ing.id == rel.ingredient_id) {
                        if (string(ing.category).find("meat") != string::npos) {
                            dietMismatch = true;
                            break;
                        }
                    }
                }
                if (dietMismatch) break;
            }
        }

        // Вивід назви з мітками
        if (hasAllergy) {
            cout << i + 1 << ". [⚠️ МІСТИТЬ АЛЕРГЕН] " << recipes[i].name << endl;
        }
        else if (dietMismatch) {
            cout << i + 1 << ". [❌ НЕ ПІДХОДИТЬ ДЛЯ ДІЄТИ] " << recipes[i].name << endl;
        }
        else {
            cout << i + 1 << ". " << recipes[i].name << endl;
        }
    }

    int sel;
    cout << "\nНомер для перегляду (0 - назад): ";
    if (!(cin >> sel) || sel == 0) return;

    if (sel > 0 && sel <= (int)recipes.size()) {
        auto& res = recipes[sel - 1];
        system("cls");
        cout << "\n=== " << res.name << " ===\n" << res.description << "\n";

        auto relations = Storage::getRelationsByDish(res.id);
        auto allIng = Storage::getAllIngredients();

        cout << "\n🍎 СКЛАД:";
        for (auto& rel : relations) {
            for (auto& ing : allIng) {
                if (ing.id == rel.ingredient_id)
                    cout << "\n - " << ing.name << ": " << rel.quantity << " " << ing.unit;
            }
        }
        cout << "\n\n👨‍🍳 ІНСТРУКЦІЯ:\n" << res.instructions << "\n";
        cout << "\n🔥 Калорійність: " << res.calories << " ккал\n";

        cout << "\n-----------------------------------------";
        cout << "\n1. Додати в кошик\n2. Додати в улюблені\n3. РЕДАГУВАТИ РЕЦЕПТ";

        if (isAdmin) cout << "\n4. ВИДАЛИТИ РЕЦЕПТ";
        cout << "\n0. Назад\n>> ";

        int action; cin >> action;
        if (action == 1) addToShoppingList(res.id);
        else if (action == 2) addToFavorites(currentUser, res.id);
        else if (action == 3) editRecipeFlow(res.id);
        else if (action == 4 && isAdmin) Storage::deleteRecipe(res.id);
    }
}

// --- 3. Редагування рецепту ---
void Recipe::editRecipeFlow(int dishId) {
    auto allRecipes = Storage::getAllRecipes();
    RecipeData* target = nullptr;

    for (auto& r : allRecipes) {
        if (r.id == dishId) {
            target = &r;
            break;
        }
    }

    if (!target) {
        cout << "❌ Рецепт не знайдено!\n";
        return;
    }

    system("cls");
    cout << "=== РЕДАГУВАННЯ РЕЦЕПТУ (ID: " << dishId << ") ===\n";
    cout << "(Залиште поле порожнім, щоб не змінювати його)\n\n";

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string input;

    cout << "Поточна назва: " << target->name << "\nНова назва: ";
    getline(cin, input);
    if (!input.empty()) strncpy_s(target->name, sizeof(target->name), input.c_str(), _TRUNCATE);

    cout << "Поточний опис: " << target->description << "\nНовий опис: ";
    getline(cin, input);
    if (!input.empty()) strncpy_s(target->description, sizeof(target->description), input.c_str(), _TRUNCATE);

    cout << "Поточна інструкція: " << target->instructions << "\nНова інструкція: ";
    getline(cin, input);
    if (!input.empty()) strncpy_s(target->instructions, sizeof(target->instructions), input.c_str(), _TRUNCATE);

    cout << "Поточні калорії: " << target->calories << "\nНові калорії (0 - без змін): ";
    int newCal;
    if (cin >> newCal && newCal > 0) target->calories = newCal;

    Storage::updateRecipe(*target);
    cout << "\n✅ Рецепт успішно оновлено у базі!\n";
    system("pause");
}

// --- 4. Список покупок та Улюблене ---
void Recipe::addToShoppingList(int dishId) {
    auto relations = Storage::getRelationsByDish(dishId);
    ofstream f("shopping_list.dat", ios::binary | ios::app);
    if (f) {
        for (auto& rel : relations) {
            f.write((const char*)&rel, sizeof(DishIngredient));
        }
        f.close();
        cout << "✅ Додано до списку покупок!\n";
    }
}

void Recipe::addToFavorites(const string& user, int recipeId) {
    FavoriteRecipe fav;
    memset(&fav, 0, sizeof(FavoriteRecipe));
    fav.recipe_id = recipeId;
    strncpy_s(fav.username, sizeof(fav.username), user.c_str(), _TRUNCATE);

    ofstream f("favorites.dat", ios::binary | ios::app);
    if (f) {
        f.write((const char*)&fav, sizeof(FavoriteRecipe));
        f.close();
        cout << "⭐ Додано в обране!\n";
    }
}

void Recipe::favoritesToShoppingList(const std::string& user) {
    ifstream f("favorites.dat", ios::binary);
    if (!f) return;

    vector<DishIngredient> allRelations;
    FavoriteRecipe fav;
    while (f.read((char*)&fav, sizeof(FavoriteRecipe))) {
        if (string(fav.username) == user) {
            auto relations = Storage::getRelationsByDish(fav.recipe_id);
            allRelations.insert(allRelations.end(), relations.begin(), relations.end());
        }
    }
    f.close();
    ofstream out("shopping_list.dat", ios::binary | ios::app);
    for (auto& rel : allRelations) {
        out.write((const char*)&rel, sizeof(DishIngredient));
    }
    out.close();
}


void Recipe::showFavorites(const string& user) {
    ifstream f("favorites.dat", ios::binary);
    if (!f) {
        cout << "\n⭐ Список обраного порожній.\n";
        return;
    }

    FavoriteRecipe fav;
    auto allRecipes = Storage::getAllRecipes();
    bool found = false;

    cout << "\n⭐ ВАШІ УЛЮБЛЕНІ РЕЦЕПТИ:\n";
    while (f.read((char*)&fav, sizeof(FavoriteRecipe))) {
        if (string(fav.username) == user) {
            for (auto& r : allRecipes) {
                if (r.id == fav.recipe_id) {
                    cout << " ✅ " << r.name << endl;
                    found = true;
                }
            }
        }
    }
    if (!found) cout << "😔 У списку поки порожньо.\n";
    f.close();
}

// --- 5. Пошук та адмін-функції ---
void Recipe::searchByIngredient() {
    string query;
    cout << "\nПошук за продуктом: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, query);

    auto allIng = Storage::getAllIngredients();
    int targetId = -1;
    for (const auto& ing : allIng) {
        if (string(ing.name).find(query) != string::npos) {
            targetId = ing.id;
            break;
        }
    }

    if (targetId == -1) { cout << "❌ Не знайдено.\n"; return; }

    ifstream f("relations.dat", ios::binary);
    DishIngredient rel;
    vector<int> foundIds;
    while (f.read((char*)&rel, sizeof(DishIngredient))) {
        if (rel.ingredient_id == targetId) foundIds.push_back(rel.dish_id);
    }
    f.close();

    if (foundIds.empty()) cout << "😔 Страв не знайдено.\n";
    else {
        auto allR = Storage::getAllRecipes();
        for (int id : foundIds) {
            for (auto& r : allR) if (r.id == id) cout << "✅ " << r.name << endl;
        }
    }
}

void Recipe::shoppingListFlow(const std::string& currentUser) {
    favoritesToShoppingList(currentUser);

    ifstream f("shopping_list.dat", ios::binary);
    if (!f) {
        cout << "\n🛒 Кошик порожній.\n";
        return;
    }

    map<int, float> totals;
    DishIngredient rel;
    while (f.read((char*)&rel, sizeof(DishIngredient)))
        totals[rel.ingredient_id] += rel.quantity;
    f.close();

    auto allIng = Storage::getAllIngredients();
    cout << "\n🛒 СПИСОК ПОКУПОК:\n";
    cout << "=== Інгредієнти улюблених рецептів ===\n";
    for (auto const& [id, qty] : totals) {
        for (auto& ing : allIng) {
            if (ing.id == id)
                cout << " • " << ing.name << ": " << qty << " " << ing.unit << endl;
        }
    }

    cout << "\n=== Інгредієнти додані вами ===\n";
    // тут можна показати ті, що користувач додавав вручну

    cout << "\nОчистити кошик? (1 - Так, 0 - Ні): ";
    int clean; cin >> clean;
    if (clean == 1) {
        remove("shopping_list.dat");
        cout << "🗑 Очищено.\n";
    }
}


void Recipe::adminAddRecipe(const RecipeData& r) {
    Storage::saveRecipe(r);
    system("cls");
    cout << "=========================================" << endl;
    cout << "✅ ТЕКСТОВІ ДАНІ УСПІШНО ЗБЕРЕЖЕНО" << endl;
    cout << "=========================================" << endl;
    cout << "🍴 Назва:       " << r.name << endl;
    cout << "📖 Опис:        " << r.description << endl;
    cout << "👨‍🍳 Інструкція:  " << r.instructions << endl;
    cout << "🔥 Калорії:     " << r.calories << endl;
    cout << "=========================================" << endl;

    auto allIngredients = Storage::getAllIngredients();
    if (allIngredients.empty()) {
        cout << "\n⚠️ Попередження: База інгредієнтів порожня!\n";
        system("pause");
        return;
    }

    cout << "\n🍏 ВИБІР ІНГРЕДІЄНТІВ ДЛЯ СТРАВИ:\n";
    char addMore;
    do {
        for (int i = 0; i < (int)allIngredients.size(); i++) {
            cout << i + 1 << ". " << allIngredients[i].name << " (" << allIngredients[i].unit << ")\n";
        }
        int choice;
        cout << "\nОберіть номер (0 - завершити): ";
        cin >> choice;
        if (choice > 0 && choice <= (int)allIngredients.size()) {
            DishIngredient rel;
            rel.dish_id = r.id;
            rel.ingredient_id = allIngredients[choice - 1].id;
            cout << "Введіть кількість (" << allIngredients[choice - 1].unit << "): ";
            cin >> rel.quantity;
            Storage::saveDishRelation(rel);
            cout << "✅ Додано.\n";
        }
        else break;
        cout << "Додати ще продукт? (y/n): ";
        cin >> addMore;
    } while (addMore == 'y' || addMore == 'Y');

    cout << "\n✨ РЕЦЕПТ ПОВНІСТЮ ЗБЕРЕЖЕНО!\n";
    system("pause");
}