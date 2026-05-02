#ifndef RECIPE_STRUCT_H
#define RECIPE_STRUCT_H
struct FavoriteRecipe {
    char username[50]; // Переконайся, що розмір достатній
    int recipe_id;
};
struct RecipeData {
    int id;
    int author_id;
    char author[50];
    char name[100];
    char description[255];
    char ingredients[1000];
    char instructions[5000];
    int calories;
    char type[50];
    char tags[100];
};

#endif