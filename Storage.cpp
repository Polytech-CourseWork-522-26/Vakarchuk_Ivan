#include "Storage.h"
#include <fstream>

using namespace std;

// USERS
bool Storage::saveUser(const UserData& user) {
    ofstream f("users.dat", ios::binary | ios::app);
    f.write((char*)&user, sizeof(user));
    return true;
}

bool Storage::getUser(const string& username, UserData& user) {
    ifstream f("users.dat", ios::binary);

    while (f.read((char*)&user, sizeof(user))) {
        if (username == user.username)
            return true;
    }
    return false;
}

// RECIPES
void Storage::saveRecipe(const RecipeData& r) {
    ofstream f("recipes.dat", ios::binary | ios::app);
    f.write((char*)&r, sizeof(r));
}

vector<RecipeData> Storage::getAllRecipes() {
    vector<RecipeData> list;
    ifstream f("recipes.dat", ios::binary);

    RecipeData r;
    while (f.read((char*)&r, sizeof(r))) {
        list.push_back(r);
    }

    return list;
}

bool Storage::updateRecipe(const RecipeData& r) {
    fstream f("recipes.dat", ios::binary | ios::in | ios::out);

    RecipeData temp;

    while (f.read((char*)&temp, sizeof(temp))) {
        if (temp.id == r.id) {
            f.seekp(-static_cast<streamoff>(sizeof(temp)), ios::cur);
            f.write((char*)&r, sizeof(r));
            return true;
        }
    }
    return false;
}

bool Storage::deleteRecipe(int id) {
    auto list = getAllRecipes();

    ofstream f("recipes.dat", ios::binary | ios::trunc);

    for (auto& r : list) {
        if (r.id != id)
            f.write((char*)&r, sizeof(r));
    }
    return true;
}