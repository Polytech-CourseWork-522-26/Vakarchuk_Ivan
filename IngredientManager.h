#ifndef INGREDIENT_MANAGER_H
#define INGREDIENT_MANAGER_H

#include "Storage.h"
#include <string>

class IngredientManager {
public:
    static void deleteIngredient(int id);
    static void editIngredient(int id);
    static void addIngredientFlow(); // Для адміна
    static void showAllIngredients();
};

#endif