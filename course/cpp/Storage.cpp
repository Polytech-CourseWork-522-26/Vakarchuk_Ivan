#include "Storage.h"
#include <fstream>
#include <iostream> 
using namespace std;

// КОРИСТУВАЧІ

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
    if (!f) return false;
    f.write((const char*)&user, sizeof(UserData));
    return true;
}

bool Storage::getUser(const string& username, UserData& user) {
    ifstream f("users.dat", ios::binary);
    if (!f) return false;
    while (f.read((char*)&user, sizeof(UserData))) {
        if (username == user.username) return true;
    }
    return false;
}

// ІНГРЕДІЄНТИ (Додано)
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
    if (f) f.write((const char*)&r, sizeof(RecipeData));
}
void Storage::saveDishRelation(const DishIngredient& rel) {
    ofstream f("relations.dat", ios::binary | ios::app);
    f.write((const char*)&rel, sizeof(DishIngredient));
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
void Storage::updateRecipe(const RecipeData& updatedRecipe) {
    fstream f("recipes.dat", ios::binary | ios::in | ios::out);
    RecipeData r;
    while (f.read((char*)&r, sizeof(RecipeData))) {
        if (r.id == updatedRecipe.id) {
            // Повертаємо покажчик назад на розмір структури
            f.seekp(-(long long)sizeof(RecipeData), ios::cur);
            f.write((char*)&updatedRecipe, sizeof(RecipeData));
            break;
        }
    }
    f.close();
}
vector<DishIngredient> Storage::getRelationsByDish(int dishId) {
    vector<DishIngredient> list;
    ifstream f("relations.dat", ios::binary);
    DishIngredient rel;
    while (f.read((char*)&rel, sizeof(DishIngredient))) {
        if (rel.dish_id == dishId) list.push_back(rel);
    }
    return list;
}

void Storage::deleteRecipe(int dishId) {
    // 1. Оновлюємо файл рецептів
    ifstream f("recipes.dat", ios::binary);
    if (f) { // Перевіряємо, чи файл взагалі існує
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

    // ТЕПЕР ЦЕ ПРАЦЮВАТИМЕ:
    cout << "\n=========================================" << endl;
    cout << "✅ Рецепт та його інгредієнти успішно видалено!" << endl;
    cout << "=========================================" << endl;

    system("pause");
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
UserData Storage::getUserByName(const std::string& name) {
    auto users = getAllUsers();
    for (auto& u : users) {
        if (std::string(u.username) == name) return u;
    }
    UserData empty{};
    memset(&empty, 0, sizeof(UserData));
    return empty;
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
