#ifndef USER_H
#define USER_H
#include <cstring>
#include <string>
using namespace std;

class User {
private:
    string username;

public:
    User(string name);
    string getName() const;
};

#endif