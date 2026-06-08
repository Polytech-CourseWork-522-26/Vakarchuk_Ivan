#include "../h/User.h"

User::User(string name) {// Конструктор класу User, який приймає ім'я користувача
    username = name;
}


string User::getName() const {// Метод для отримання імені користувача
    return username;
}