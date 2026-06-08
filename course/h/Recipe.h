#ifndef RECIPE_H
#define RECIPE_H
#include <map>
#include <string>
#include "RecipeStruct.h"
#include <string>
#include <cstring>
void safe_copy_str(char* dest, const std::string& src, size_t size);
class Recipe {
public:
    void shoppingListFlow();
    void searchByIngredient();
    void createFlow(const std::string& user);
    void editRecipeFlow(int dishId);
    void adminAddRecipe(const RecipeData& r);
    void favoritesToShoppingList(const std::string& user);
    void addToShoppingList(int dishId);
    void addToFavorites(const std::string& user, int recipeId);
    void showAll(bool isAdmin, const std::string& currentUser);
    void showFavorites(const std::string& user);
    void shoppingListFlow(const std::string& currentUser);
    void createFromWeb(const std::map<std::string, std::string>& params, const std::string& author);
    
};

#endif
