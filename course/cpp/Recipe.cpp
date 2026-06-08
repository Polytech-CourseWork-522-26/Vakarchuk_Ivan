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
#include <cstring>

using namespace std;

/**
 * @brief Безпечне копіювання рядка в char-масив
 * Запобігає переповненню буфера в бінарних структурах.
 */
void safe_copy_str(char* dest, const std::string& src, size_t size) {
    if (!dest || size == 0) return;
    strncpy(dest, src.c_str(), size - 1);
    dest[size - 1] = '\0';
}

// ============================================================================
// РОБОТА КОРИСТУВАЧІВ ТА АДМІНІВ З РЕЦЕПТАМИ
// ============================================================================

/**
 * @brief Консольний інтерфейс створення рецепта
 * Дозволяє користувачу покроково ввести дані про страву.
 */
void Recipe::createFlow(const std::string& user) {
    RecipeData r;
    memset(&r, 0, sizeof(RecipeData)); // Очищення пам'яті для бінарного запису
    
    r.id = std::stoi(IDGenerator::generate("recipe_id.txt")); // Генерація унікального ID[cite: 11]

    if (cin.peek() == '\n') cin.ignore(); 

    cout << "Назва страви: ";
    string temp;
    getline(cin, temp);
    safe_copy_str(r.name, temp, sizeof(r.name));

    cout << "Опис: ";
    getline(cin, temp);
    safe_copy_str(r.description, temp, sizeof(r.description));

    cout << "Інструкції (кроки): ";
    getline(cin, temp);
    safe_copy_str(r.instructions, temp, sizeof(r.instructions));

    cout << "Калорії: ";
    while (!(cin >> r.calories)) {
        cout << "Помилка. Введіть число: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    safe_copy_str(r.author, user, sizeof(r.author));

    Storage::saveRecipe(r); // Збереження у файл через Storage[cite: 17]
    cout << "✅ Рецепт успішно збережено!\n";
}

/**
 * @brief Створення рецепта через веб-запит
 * Приймає карту параметрів (наприклад, з HTTP-форми) та реєструє автора.
 */
void Recipe::createFromWeb(const std::map<std::string, std::string>& params, const std::string& author) {
    RecipeData r;
    memset(&r, 0, sizeof(RecipeData));

    r.id = std::stoi(IDGenerator::generate("recipe_id.txt"));
    
    safe_copy_str(r.name, params.at("name"), sizeof(r.name));
    safe_copy_str(r.description, params.at("description"), sizeof(r.description));
    safe_copy_str(r.instructions, params.at("instructions"), sizeof(r.instructions));
    safe_copy_str(r.author, author, sizeof(r.author));
    
    if (params.count("youtube_url")) {
       safe_copy_str(r.video_url, params.at("video_url"), sizeof(r.video_url));
    }

    r.calories = std::stoi(params.at("calories"));
    
    Storage::saveRecipe(r);
}

// ============================================================================
// ПОШУК, ФІЛЬТРАЦІЯ ТА ВІДОБРАЖЕННЯ
// ============================================================================

/**
 * @brief Головне меню перегляду страв
 * Реалізує фільтрацію за алергіями та дієтою, а також адмін-дії.
 */
void Recipe::showAll(bool isAdmin, const string& currentUser) {
    auto recipes = Storage::getAllRecipes(); // Завантаження всіх рецептів[cite: 17]
    auto allIngredients = Storage::getAllIngredients();
    UserData user = Storage::getUserByName(currentUser);

    if (recipes.empty()) {
        cout << "\nРецептів поки немає.\n";
        return;
    }

    cout << "\n========== СПИСОК СТРАВ ==========\n";
    for (int i = 0; i < (int)recipes.size(); i++) {
        bool hasAllergy = false;
        bool dietMismatch = false;
        auto relations = Storage::getRelationsByDish(recipes[i].id);

        // Перевірка на алергени та відповідність дієті
        for (auto& rel : relations) {
            for (auto& ing : allIngredients) {
                if (ing.id == rel.ingredient_id) {
                    // Перевірка алергії
                    if (string(user.allergies).find(ing.name) != string::npos) {
                        hasAllergy = true;
                    }
                    // Перевірка вегетаріанської дієти (пошук м'яса)
                    if (string(user.diet).find("vegetarian") != string::npos && 
                        string(ing.category).find("meat") != string::npos) {
                        dietMismatch = true;
                    }
                }
            }
        }

        // Візуальне маркування в списку
        if (hasAllergy) cout << i + 1 << ". [⚠️ МІСТИТЬ АЛЕРГЕН] ";
        else if (dietMismatch) cout << i + 1 << ". [❌ НЕ ПІДХОДИТЬ] ";
        else cout << i + 1 << ". ";
        
        cout << recipes[i].name << endl;
    }

    // Детальний перегляд обраного рецепта
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

        // Дії над конкретним рецептом
        cout << "\n-----------------------------------------";
        cout << "\n1. Додати в кошик\n2. Додати в улюблені\n3. РЕДАГУВАТИ РЕЦЕПТ";
        if (isAdmin) cout << "\n4. ВИДАЛИТИ РЕЦЕПТ";
        cout << "\n0. Назад\n>> ";

        int action; cin >> action;
        if (action == 1) addToShoppingList(res.id);
        else if (action == 2) addToFavorites(currentUser, res.id);
        else if (action == 3) editRecipeFlow(res.id);
        else if (action == 4 && isAdmin) Storage::deleteRecipe(res.id); // Тільки для адміністраторів[cite: 17]
    }
}

// ============================================================================
// МОДЕРУВАННЯ ТА ОНОВЛЕННЯ (АДМІН / АВТОР)
// ============================================================================

/**
 * @brief Редагування існуючого рецепта
 * Дозволяє змінити назву, опис або інструкції.
 */
void Recipe::editRecipeFlow(int dishId) {
    auto allRecipes = Storage::getAllRecipes();
    RecipeData target; // Створюємо звичайну локальну структуру
    bool found = false;
    
    for (const auto& r : allRecipes) {
        if (r.id == dishId) { 
            target = r; // Просто копіюємо потрібний рецепт сюди
            found = true; 
            break; 
        }
    }
    
    if (!found) {
        cout << "❌ Рецепт не знайдено.\n";
        return;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string input;

    cout << "Нова назва (залиште порожнім, щоб не змінювати): ";
    getline(cin, input);
    if (!input.empty()) safe_copy_str(target.name, input, sizeof(target.name));

    cout << "Новий опис: ";
    getline(cin, input);
    if (!input.empty()) safe_copy_str(target.description, input, sizeof(target.description));

    cout << "Нова інструкція: ";
    getline(cin, input);
    if (!input.empty()) safe_copy_str(target.instructions, input, sizeof(target.instructions));

    // Передаємо змінений локальний об'єкт у Storage для перезапису у файлі
    if (Storage::updateRecipe(target)) { 
        cout << "✅ Рецепт оновлено!\n";
    } else {
        cout << "❌ Помилка збереження оновлень у файл.\n";
    }
}
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

/**
 * @brief Збереження рецепта у персональний список "Обране".
 * Створює зв'язок між ім'ям користувача та ID рецепта[cite: 14, 15].
 */
void Recipe::addToFavorites(const string& user, int recipeId) {
    FavoriteRecipe fav;
    memset(&fav, 0, sizeof(FavoriteRecipe)); 
    
    fav.recipe_id = recipeId;
    safe_copy_str(fav.username, user, sizeof(fav.username));

    ofstream f("favorites.dat", ios::binary | ios::app);
    if (f.is_open()) {
        f.write((const char*)&fav, sizeof(FavoriteRecipe));
        f.close();
        cout << "❤️ Додано в улюблені!\n";
    } else {
        cout << "❌ Помилка доступу до бази улюблених.\n";
    }
}

/**
 * @brief Перенесення інгредієнтів з усіх улюблених страв у список покупок.
 * Автоматично формує кошик на основі вподобань користувача[cite: 14, 17].
 */
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

/**
 * @brief Відображення списку улюблених страв користувача.
 * Співставляє збережені ID з назвами рецептів із загальної бази[cite: 14, 15, 17].
 */
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

// ============================================================================
// ПОШУК ТА АДМІНІСТРАТИВНЕ КЕРУВАННЯ
// ============================================================================

/**
 * @brief Пошук рецептів за назвою інгредієнта.
 * Знаходить ID продукту, а потім усі страви, де він використовується[cite: 14, 16, 17].
 */
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

    if (targetId == -1) { 
        cout << "❌ Не знайдено.\n"; 
        return; 
    }

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
            for (auto& r : allR) 
                if (r.id == id) cout << "✅ " << r.name << endl;
        }
    }
}

/**
 * @brief Генерація зведеного списку покупок із підрахунком кількості.
 * Сумує однакові інгредієнти та пропонує очистити кошик[cite: 14, 17].
 */
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

    cout << "\nОчистити кошик? (1 - Так, 0 - Ні): ";
    int clean; cin >> clean;
    if (clean == 1) {
        remove("shopping_list.dat");
        cout << "🗑 Очищено.\n";
    }
}

/**
 * @brief Додавання рецепта адміністратором із вибором інгредієнтів.
 * Дозволяє не тільки створити текст рецепта, а й прив'язати продукти з бази[cite: 12, 14, 17].
 */
void Recipe::adminAddRecipe(const RecipeData& r) {
    Storage::saveRecipe(r);
    cout << "\n✅ ТЕКСТОВІ ДАНІ ЗБЕРЕЖЕНО. Оберіть інгредієнти:\n";

    auto allIngredients = Storage::getAllIngredients();
    if (allIngredients.empty()) {
        cout << "⚠️ База продуктів порожня!\n";
        return;
    }

    char addMore;
    do {
        for (int i = 0; i < (int)allIngredients.size(); i++) {
            cout << i + 1 << ". " << allIngredients[i].name << endl;
        }
        int choice;
        cout << "Номер продукту (0 - вихід): "; cin >> choice;
        if (choice > 0 && choice <= (int)allIngredients.size()) {
            DishIngredient rel;
            rel.dish_id = r.id;
            rel.ingredient_id = allIngredients[choice - 1].id;
            cout << "Кількість: "; cin >> rel.quantity;
            Storage::saveDishRelation(rel);
            cout << "✅ Додано.\n";
        } else break;
        cout << "Додати ще? (y/n): "; cin >> addMore;
    } while (addMore == 'y' || addMore == 'Y');

    cout << "✨ Рецепт повністю готовий!\n";
}