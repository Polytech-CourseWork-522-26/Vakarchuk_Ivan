#ifndef RECIPE_STRUCT_H
#define RECIPE_STRUCT_H

struct RecipeData {
    int id;
    char author[50];
    char name[100];         
    char description[255];
    char ingredients[500];  
    char instructions[100000]; 
    int calories;
};

#endif