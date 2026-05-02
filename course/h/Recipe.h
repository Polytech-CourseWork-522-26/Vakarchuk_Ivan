#ifndef RECIPE_H
#define RECIPE_H
#include "RecipeStruct.h"
#include <string>

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


};

#endif
