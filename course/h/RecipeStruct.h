#ifndef RECIPE_STRUCT_H
#define RECIPE_STRUCT_H
#include <cstring>
struct FavoriteRecipe {
    char username[50];
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
    char photo_path[200];
    char video_url[200];
};



#endif