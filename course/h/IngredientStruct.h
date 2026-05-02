#ifndef INGREDIENT_STRUCT_H
#define INGREDIENT_STRUCT_H

struct IngredientData {
    int id;
    char name[100];
    char manufacturer[100];
    char category[50];     // овочі, молочне, м'ясо
    char unit[20];         // г, кг, мл, шт
};

#endif