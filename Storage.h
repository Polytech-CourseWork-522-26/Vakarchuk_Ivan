#ifndef STORAGE_H
#define STORAGE_H

#include <string>
#include <vector>
#include "RecipeStruct.h"

struct UserData {
    int id;
    char username[50];
    char password[50];
    char email[100];
    char diet[50];
    char allergies[255];
    int role;
};

struct IngredientData {
    int id;
    char name[100];
    char manufacturer[100];
    char category[50];
    char unit[20];
};

struct DishIngredient {
    int dish_id;
    int ingredient_id;
    float quantity;
};

class Storage {
public:
    static bool saveUser(const UserData& user);
    static bool getUser(const std::string& username, UserData& user);
    static std::vector<UserData> getAllUsers();
    static void deleteUser(const std::string& username);

    static void saveRecipe(const RecipeData& r);
    static std::vector<RecipeData> getAllRecipes();
    static void updateRecipe(const RecipeData& r); // Змінено на void для узгодження з .cpp
    static void deleteRecipe(int dishId);
    static UserData getUserByName(const std::string& name);

    static void saveIngredient(const IngredientData& ing);
    static std::vector<IngredientData> getAllIngredients();
    static void saveDishRelation(const DishIngredient& rel);
    static std::vector<DishIngredient> getRelationsByDish(int dishId);
    static void updateUser(const UserData& user);

    static void showGlobalStatistics();
};

#endif