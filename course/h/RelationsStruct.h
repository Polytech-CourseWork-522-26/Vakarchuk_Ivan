#pragma once
#include <cstring>
struct DishIngredient {
    int dish_id;
    int ingredient_id;
    float quantity;
    char source[30];     // залишимо, але будемо використовувати
};

struct FavoriteDish {
    int user_id;
    int dish_id;
};