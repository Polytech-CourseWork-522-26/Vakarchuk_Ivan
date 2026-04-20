#include "Auth.h"
#include "Storage.h"
#include "IDGenerator.h"
#include <cstring>

using namespace std;

bool Auth::registerUser(const string& u, const string& p) {

    UserData user;

    if (Storage::getUser(u, user))
        return false;

    user.id = stoi(IDGenerator::generate("user_id.txt"));

    strcpy_s(user.username, u.c_str());
    strcpy_s(user.password, p.c_str());

    return Storage::saveUser(user);
}

bool Auth::login(const string& u, const string& p) {
    UserData user;

    if (!Storage::getUser(u, user))
        return false;

    return p == user.password;
}