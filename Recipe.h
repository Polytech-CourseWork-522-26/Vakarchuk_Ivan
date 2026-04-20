#ifndef RECIPE_H
#define RECIPE_H

#include <string>

class Recipe {
public:
    void showAll();
    void createFlow(const std::string& user);
    void editFlow(const std::string& user);
    void deleteFlow(const std::string& user);
};

#endif