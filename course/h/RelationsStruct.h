#pragma once
struct DishIngredient {
    int dish_id;
    int ingredient_id;
    float quantity;
};

struct FavoriteDish {
    int user_id;
    int dish_id;
};