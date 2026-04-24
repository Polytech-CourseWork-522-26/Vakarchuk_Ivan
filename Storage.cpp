#include "Storage.h"
#include <fstream>

using namespace std;

// КОРИСТУВАЧІ
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
    while (f.read((char*)&ing, sizeof(IngredientData))) {
        list.push_back(ing);
    }
    return list;
}

// РЕЦЕПТИ (DishData)
void Storage::saveRecipe(const RecipeData& r) {
    ofstream f("recipes.dat", ios::binary | ios::app);
    if (f) f.write((const char*)&r, sizeof(RecipeData));
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
bool Storage::updateRecipe(const RecipeData& r) {
    fstream f("recipes.dat", ios::binary | ios::in | ios::out);
    if (!f) return false;
    RecipeData temp;
    while (f.read((char*)&temp, sizeof(RecipeData))) {
        if (temp.id == r.id) {
            f.seekp(-static_cast<streamoff>(sizeof(RecipeData)), ios::cur);
            f.write((const char*)&r, sizeof(RecipeData));
            return true;
        }
    }
    return false;
}
bool Storage::deleteRecipe(int id) {
    auto list = getAllRecipes();
    ofstream f("recipes.dat", ios::binary | ios::trunc);
    for (auto& r : list) {
        if (r.id != id) f.write((const char*)&r, sizeof(RecipeData));
    }
    return true;
}
