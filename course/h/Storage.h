#ifndef STORAGE_H
#define STORAGE_H
#include <cstring>
#include <string>
#include <vector>
#include "RecipeStruct.h"

struct ReportData {
    int id;
    int recipe_id;
    char username[50];
    char reason[512];
    char report_date[20];
};

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
    char source[30];
};

struct ShoppingItem {
    int ingredient_id;
    float quantity;
    char source[20];
    char name[100];
    char unit[20];
};

class Storage {
public:
    // Вкладена структура
    struct RecipeWithAllergen {
        RecipeData recipe;
        bool hasAllergen = false;
    };

    static std::vector<RecipeData> searchRecipesByName(const std::string& query);
    static std::vector<RecipeData> searchRecipesByIngredients(const std::vector<int>& ingredientIds);
    static std::vector<RecipeData> getFavoritesByUsername(const std::string& username);
    static bool saveUser(const UserData& user);
    static bool getUser(const std::string& username, UserData& user);
    static std::vector<UserData> getAllUsers();
    static void deleteUser(const std::string& username);
    static bool isFavorite(const std::string& username, int recipeId);
    static void saveRecipe(const RecipeData& r);
    static std::vector<RecipeData> getAllRecipes();
    static bool updateRecipe(const RecipeData& recipe);
    static void deleteRecipe(int dishId);
    static UserData getUserByName(const std::string& name);
    static void addFavorite(const std::string& username, int recipeId);
    static void saveIngredient(const IngredientData& ing);
    static std::vector<IngredientData> getAllIngredients();
    static void saveDishRelation(const DishIngredient& rel);
    static std::vector<DishIngredient> getRelationsByDish(int dishId);
    static void updateUser(const UserData& user);
    static void removeFavorite(const std::string& username, int recipeId);
    static void showGlobalStatistics();
    static std::vector<DishIngredient> getAllDishRelations(); 
    static void addToShoppingList(int ingredient_id, float quantity, const std::string& source = "manual");
    static std::vector<ShoppingItem> getShoppingList();
    static void clearShoppingList();
    static void addReport(const std::string& username, int recipeId, const std::string& reason);
    static std::vector<ReportData> getAllReports();
    static void removeReport(int recipeId);     
    static void removeSingleReport(int recipeId);
    // Метод для алергенів
    static std::vector<RecipeWithAllergen> getAllRecipesWithAllergenInfo(const UserData& user);
};

#endif