#include "Storage.h"
#include <fstream>
#include <iostream> 
#include <cstring>
#include <algorithm>   // для std::transform та std::sort
#include <cctype>      // для ::tolower
#include <set>         // для std::set
using namespace std;

// КОРИСТУВАЧІ
bool Storage::getUser(const std::string& username, UserData& user) {
    auto users = getAllUsers();
    for (const auto& u : users) {
        if (std::string(u.username) == username) {
            user = u;
            return true;
        }
    }
    return false;
}
void Storage::deleteUser(const std::string& username) {
    // 1. Відкриваємо файл з користувачами
    std::ifstream f("users.dat", std::ios::binary);
    if (!f) {
        std::cout << "❌ Помилка: Файл користувачів не знайдено.\n";
        return;
    }

    // 2. Створюємо тимчасовий файл
    std::ofstream temp("temp_u.dat", std::ios::binary);
    UserData u;
    bool found = false;

    // 3. Переписуємо всіх, крім того, кого видаляємо
    while (f.read((char*)&u, sizeof(UserData))) {
        if (std::string(u.username) != username) {
            temp.write((char*)&u, sizeof(UserData));
        }
        else {
            found = true;
        }
    }

    f.close();
    temp.close();

    // 4. Замінюємо старий файл новим
    std::remove("users.dat");
    std::rename("temp_u.dat", "users.dat");

    if (found) {
        std::cout << "✅ Користувача '" << username << "' успішно видалено.\n";
    }
    else {
        std::cout << "⚠️ Користувача з таким іменем не знайдено.\n";
    }
    system("pause");
}
vector<UserData> Storage::getAllUsers() {
    vector<UserData> users;
    ifstream f("users.dat", ios::binary);
    if (!f) return users;
    UserData u;
    while (f.read((char*)&u, sizeof(UserData))) {
        users.push_back(u);
    }
    f.close();
    return users;
}
bool Storage::saveUser(const UserData& user) {
    ofstream f("users.dat", ios::binary | ios::app);
    if (!f.is_open()) {
        cout << "❌ Помилка відкриття файлу users.dat" << endl;
        return false;
    }
    f.write((const char*)&user, sizeof(UserData));
    f.flush(); // Примусово штовхаємо дані на диск
    f.close();
    return true;
}




// Залиште тільки цей один екземпляр функції:
UserData Storage::getUserByName(const std::string& name) {
    auto users = getAllUsers();
    for (auto& u : users) {
        if (std::string(u.username) == name) return u;
    }
    UserData empty;
    memset(&empty, 0, sizeof(UserData));
    empty.id = -1; 
    return empty;
}

// ІНГРЕДІЄНТИ 
void Storage::saveIngredient(const IngredientData& ing) {
    ofstream f("ingredients.dat", ios::binary | ios::app);
    f.write((const char*)&ing, sizeof(IngredientData));
}

vector<IngredientData> Storage::getAllIngredients() {
    vector<IngredientData> list;
    ifstream f("ingredients.dat", ios::binary);
    IngredientData ing;
    while (f.read((char*)&ing, sizeof(IngredientData))) list.push_back(ing);
    return list;
}

// РЕЦЕПТИ (DishData)
void Storage::saveRecipe(const RecipeData& r) {
    ofstream f("recipes.dat", ios::binary | ios::app);
    if (f.is_open()) {
        f.write((const char*)&r, sizeof(RecipeData));
        f.flush();
        f.close(); // Обов'язково закриваємо
    }
}
// ==================== SAVE ====================
void Storage::saveDishRelation(const DishIngredient& rel) {
    ofstream f("relations.dat", ios::binary | ios::app);
    if (!f.is_open()) {
        std::cerr << "❌ Не вдалося відкрити relations.dat" << std::endl;
        return;
    }

    #pragma pack(push, 1)   // Вимикаємо вирівнювання
    DishIngredient temp = rel;
    #pragma pack(pop)

    f.write((const char*)&temp, sizeof(DishIngredient));
    f.close();

    std::cout << "   [Storage] Збережено relation → dish=" << rel.dish_id 
              << ", ing=" << rel.ingredient_id 
              << ", qty=" << rel.quantity << std::endl;
}
vector<RecipeData> Storage::getAllRecipes() {
  
    vector<RecipeData> list;
    ifstream f("recipes.dat", ios::binary);
    if (!f) return list;
    RecipeData r;
    while (f.read((char*)&r, sizeof(RecipeData))) {
        list.push_back(r);
    }
    return list;
}
bool Storage::updateRecipe(const RecipeData& updatedRecipe) {
    // Відкриваємо файл для читання та запису
    fstream f("recipes.dat", ios::binary | ios::in | ios::out);
    if (!f.is_open()) return false; // Якщо файлу немає — повертаємо false

    RecipeData r;
    bool found = false;

    while (f.read((char*)&r, sizeof(RecipeData))) {
        if (r.id == updatedRecipe.id) {
            // Повертаємо покажчик запису (put pointer) назад
            f.seekp(-(long long)sizeof(RecipeData), ios::cur);
            f.write((char*)&updatedRecipe, sizeof(RecipeData));
            found = true; // Ми реально знайшли і змінили дані
            break;
        }
    }
    
    f.close();
    return found; 
}
// ==================== GET ====================
vector<DishIngredient> Storage::getRelationsByDish(int dishId) {
    vector<DishIngredient> list;
    ifstream f("relations.dat", ios::binary);
    if (!f.is_open()) {
        std::cout << "[Storage] relations.dat не знайдено\n";
        return list;
    }

    #pragma pack(push, 1)
    DishIngredient rel;
    #pragma pack(pop)

    while (f.read((char*)&rel, sizeof(DishIngredient))) {
        if (rel.dish_id == dishId) {
            list.push_back(rel);
            std::cout << "   [Storage] Зчитано: dish=" << rel.dish_id 
                      << ", ing=" << rel.ingredient_id 
                      << ", qty=" << rel.quantity << std::endl;
        }
    }

    f.close();
    std::cout << "[Storage] Для dish=" << dishId << " знайдено " << list.size() << " інгредієнтів\n";
    return list;
}
void Storage::deleteRecipe(int dishId) {
    // 1. Оновлюємо файл рецептів
    ifstream f("recipes.dat", ios::binary);
    if (f) { 
        ofstream temp("temp.dat", ios::binary);
        RecipeData r;
        while (f.read((char*)&r, sizeof(RecipeData))) {
            if (r.id != dishId) temp.write((char*)&r, sizeof(RecipeData));
        }
        f.close();
        temp.close();
        remove("recipes.dat");
        rename("temp.dat", "recipes.dat");
    }

    // 2. Оновлюємо файл зв'язків
    ifstream fRel("relations.dat", ios::binary);
    if (fRel) {
        ofstream tempRel("temp_rel.dat", ios::binary);
        DishIngredient rel;
        while (fRel.read((char*)&rel, sizeof(DishIngredient))) {
            if (rel.dish_id != dishId) tempRel.write((char*)&rel, sizeof(DishIngredient));
        }
        fRel.close();
        tempRel.close();
        remove("relations.dat");
        rename("temp_rel.dat", "relations.dat");
    }
    // Видаляємо також всі скарги на цей рецепт
    Storage::removeReport(dishId);

    // ТЕПЕР ЦЕ ПРАЦЮВАТИМЕ:
    cout << "\n=========================================" << endl;
    cout << "✅ Рецепт та його інгредієнти успішно видалено!" << endl;
    cout << "=========================================" << endl;

}

void Storage::showGlobalStatistics() {
    auto users = getAllUsers();
    auto recipes = getAllRecipes();
    auto ingredients = getAllIngredients();

    cout << "\n=========================================" << endl;
    cout << "   📊 СТАТИСТИКА СИСТЕМИ" << endl;
    cout << "=========================================" << endl;
    cout << "👥 Користувачів:  " << users.size() << endl;
    cout << "📖 Рецептів:      " << recipes.size() << endl;
    cout << "🍎 Інгредієнтів:  " << ingredients.size() << endl;
    cout << "=========================================\n" << endl;
    system("pause");
}
void Storage::addReport(const std::string& username, int recipeId, const std::string& reason) {
    ReportData report{};
    report.recipe_id = recipeId;

    // Безпечне копіюванняget
    strncpy_s(report.username, username.c_str(), sizeof(report.username)-1);

    // ✅ НАЙВАЖЛИВІШЕ ВИПРАВЛЕННЯ
    std::string clean_reason = reason;
    if (clean_reason.length() >= sizeof(report.reason)) {
        clean_reason = clean_reason.substr(0, sizeof(report.reason)-1);
    }
    strncpy_s(report.reason, clean_reason.c_str(), sizeof(report.reason)-1);

    time_t now = time(0);
    strftime(report.report_date, sizeof(report.report_date), "%Y-%m-%d %H:%M", localtime(&now));

    std::ofstream f("reports.dat", std::ios::binary | std::ios::app);
    if (f.is_open()) {
        f.write((char*)&report, sizeof(ReportData));
        f.close();
        std::cout << "✅ Скарга збережена: " << reason << std::endl;
    } else {
        std::cout << "❌ Не вдалося відкрити reports.dat" << std::endl;
    }
}

void Storage::updateUser(const UserData& updatedUser) {
    fstream f("users.dat", ios::binary | ios::in | ios::out);
    UserData u;
    while (f.read((char*)&u, sizeof(UserData))) {
        if (std::string(u.username) == updatedUser.username) {
            f.seekp(-(long long)sizeof(UserData), ios::cur);
            f.write((char*)&updatedUser, sizeof(UserData));
            break;
        }
    }
    f.close();
}


// ====================== УЛЮБЛЕНЕ ======================
// ====================== УЛЮБЛЕНЕ ======================
void Storage::addFavorite(const std::string& username, int recipeId)
{
    if (isFavorite(username, recipeId)) return;

    std::ofstream f("favorites.txt", std::ios::app);
    if (f.is_open()) {
        f << username << "|" << recipeId << "\n";
        f.close();
    }
}

void Storage::removeFavorite(const std::string& username, int recipeId)
{
    std::ifstream in("favorites.txt");
    std::ofstream out("temp_fav.txt");

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        size_t pos = line.find('|');
        if (pos == std::string::npos) continue;

        std::string user = line.substr(0, pos);
        int rid = std::stoi(line.substr(pos+1));

        if (user != username || rid != recipeId) {
            out << line << "\n";
        }
    }
    in.close();
    out.close();

    std::remove("favorites.txt");
    std::rename("temp_fav.txt", "favorites.txt");
}

bool Storage::isFavorite(const std::string& username, int recipeId)
{
    std::ifstream f("favorites.txt");
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        size_t pos = line.find('|');
        if (pos == std::string::npos) continue;

        std::string user = line.substr(0, pos);
        int rid = std::stoi(line.substr(pos+1));

        if (user == username && rid == recipeId) return true;
    }
    return false;
}

std::vector<RecipeData> Storage::getFavoritesByUsername(const std::string& username)
{
    std::vector<RecipeData> result;
    std::ifstream f("favorites.txt");
    if (!f) return result;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        size_t pos = line.find('|');
        if (pos == std::string::npos) continue;

        std::string user = line.substr(0, pos);
        int recipe_id = std::stoi(line.substr(pos+1));

        if (user == username) {
            auto all = getAllRecipes();
            for (const auto& r : all) {
                if (r.id == recipe_id) {
                    result.push_back(r);
                    break;
                }
            }
        }
    }
    return result;
}

// ====================== КОШИК ======================


// ====================== СПИСОК ПОКУПОК ======================

void Storage::addToShoppingList(int ingredient_id, float quantity, const std::string& source) {
    if (ingredient_id <= 0 || quantity <= 0) return;

    auto allIng = getAllIngredients();
    std::string ing_name = "Невідомий";
    std::string ing_unit = "шт";

    for (const auto& ing : allIng) {
        if (ing.id == ingredient_id) {
            ing_name = ing.name;
            ing_unit = ing.unit;
            break;
        }
    }

    std::vector<ShoppingItem> items;
    std::ifstream f("shopping_list.dat", std::ios::binary);
    if (f) {
        ShoppingItem item;
        while (f.read((char*)&item, sizeof(ShoppingItem))) {
            items.push_back(item);
        }
        f.close();
    }

    bool found = false;
    for (auto& item : items) {
        if (item.ingredient_id == ingredient_id) {
            item.quantity += quantity;
            found = true;
            break;
        }
    }

    if (!found) {
        ShoppingItem newItem{};
        newItem.ingredient_id = ingredient_id;
        newItem.quantity = quantity;
        strncpy_s(newItem.source, source.c_str(), sizeof(newItem.source)-1);
        strncpy_s(newItem.name, ing_name.c_str(), sizeof(newItem.name)-1);
        strncpy_s(newItem.unit, ing_unit.c_str(), sizeof(newItem.unit)-1);
        items.push_back(newItem);
    }

    std::ofstream out("shopping_list.dat", std::ios::binary);
    for (const auto& item : items) {
        out.write((const char*)&item, sizeof(ShoppingItem));
    }
    out.close();

    std::cout << "🛒 Додано в кошик: " << ing_name << " x " << quantity << std::endl;
}

std::vector<ShoppingItem> Storage::getShoppingList() {
    std::vector<ShoppingItem> items;
    std::ifstream f("shopping_list.dat", std::ios::binary);
    if (!f) return items;

    ShoppingItem item;
    while (f.read((char*)&item, sizeof(ShoppingItem))) {
        items.push_back(item);
    }
    f.close();
    return items;
}

void Storage::clearShoppingList() {
    std::remove("shopping_list.dat");
}


// ==================== ПОШУК ЗА НАЗВОЮ ====================
std::vector<RecipeData> Storage::searchRecipesByName(const std::string& query) {
    if (query.empty()) return getAllRecipes();

    auto all = getAllRecipes();
    std::vector<std::pair<RecipeData, int>> scored;

    std::string q_lower = query;
    std::transform(q_lower.begin(), q_lower.end(), q_lower.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    for (auto& r : all) {
        std::string name_lower = r.name;
        std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        int score = 0;
        if (name_lower == q_lower) score = 100;
        else if (name_lower.find(q_lower) != std::string::npos) score = 50;
        else if (q_lower.length() >= 3 && name_lower.find(q_lower.substr(0, 3)) != std::string::npos) 
            score = 20;

        if (score > 0) {
            scored.emplace_back(r, score);
        }
    }

    std::sort(scored.begin(), scored.end(), [](const auto& a, const auto& b){
        return a.second > b.second;
    });

    std::vector<RecipeData> result;
    for (auto& p : scored) result.push_back(p.first);
    return result;
}
// ==================== ПОШУК ЗА ІНГРЕДІЄНТАМИ ====================
std::vector<RecipeData> Storage::searchRecipesByIngredients(const std::vector<int>& ingredientIds) {
    if (ingredientIds.empty()) return getAllRecipes();

    auto allRecipes = getAllRecipes();
    std::vector<std::pair<RecipeData, int>> scored; // recipe + кількість зайвих інгредієнтів

    for (const auto& recipe : allRecipes) {
        auto relations = getRelationsByDish(recipe.id);
        std::set<int> recipeIngIds;

        for (const auto& rel : relations) {
            recipeIngIds.insert(rel.ingredient_id);
        }

        int matched = 0;
        for (int id : ingredientIds) {
            if (recipeIngIds.count(id)) matched++;
        }

        if (matched == (int)ingredientIds.size()) {
            int extra = (int)recipeIngIds.size() - matched;
            scored.emplace_back(recipe, extra);
        }
    }

    std::sort(scored.begin(), scored.end(), [](const auto& a, const auto& b){
        return a.second < b.second; // менше зайвих = вище в списку
    });

    std::vector<RecipeData> result;
    for (auto& p : scored) result.push_back(p.first);
    return result;
}


std::vector<Storage::RecipeWithAllergen> Storage::getAllRecipesWithAllergenInfo(const UserData& user) {
    auto allRecipes = getAllRecipes();
    auto allIngredients = getAllIngredients();
    std::vector<Storage::RecipeWithAllergen> result;

    std::string userAllergies = user.allergies;
    std::transform(userAllergies.begin(), userAllergies.end(), userAllergies.begin(), 
                   [](unsigned char c){ return std::tolower(c); });

    for (const auto& recipe : allRecipes) {
        Storage::RecipeWithAllergen item;
        item.recipe = recipe;
        item.hasAllergen = false;

        auto relations = getRelationsByDish(recipe.id);

        for (const auto& rel : relations) {
            for (const auto& ing : allIngredients) {
                if (ing.id == rel.ingredient_id) {
                    std::string ingName = ing.name;
                    std::transform(ingName.begin(), ingName.end(), ingName.begin(),
                                   [](unsigned char c){ return std::tolower(c); });

                    if (!userAllergies.empty() && userAllergies.find(ingName) != std::string::npos) {
                        item.hasAllergen = true;
                        break;
                    }
                }
            }
            if (item.hasAllergen) break;
        }

        result.push_back(item);
    }

    return result;
}

std::vector<ReportData> Storage::getAllReports() {
    std::vector<ReportData> reports;
    std::ifstream f("reports.dat", std::ios::binary);
    if (!f.is_open()) return reports;

    ReportData r;
    while (f.read((char*)&r, sizeof(ReportData))) {
        r.username[sizeof(r.username)-1] = '\0';
        r.reason[sizeof(r.reason)-1] = '\0';     // Обрізаємо сміття
        reports.push_back(r);
    }
    f.close();
    return reports;
}
// Видалити всі скарги на конкретний рецепт
void Storage::removeReport(int recipeId) {
    if (recipeId <= 0) return;

    std::ifstream in("reports.dat", std::ios::binary);
    if (!in) return;

    std::ofstream out("temp_reports.dat", std::ios::binary);

    ReportData r;
    int removed = 0;

    while (in.read((char*)&r, sizeof(ReportData))) {
        if (r.recipe_id != recipeId) {
            out.write((char*)&r, sizeof(ReportData));
        } else {
            removed++;
        }
    }

    in.close();
    out.close();

    std::remove("reports.dat");
    std::rename("temp_reports.dat", "reports.dat");

    if (removed > 0) {
        std::cout << "🗑️ Видалено " << removed << " скарг на рецепт #" << recipeId << std::endl;
    }
}

// Видалити одну конкретну скаргу (для кнопки "Перевірено")
void Storage::removeSingleReport(int recipeId) {
    removeReport(recipeId); // поки що однакова логіка
}