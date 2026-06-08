#ifndef INGREDIENT_STRUCT_H
#define INGREDIENT_STRUCT_H
#include <cstring>
struct IngredientData {
    int id;
    char name[100];
    char manufacturer[100];
    char category[50];     
    char unit[20];         
};

#endif