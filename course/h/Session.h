#pragma once
#include <string>
#include "crow_all.h"
enum Role {
    GUEST = 0,
    USER = 1,
    ADMIN = 2
};

struct Session {
    std::string username;
    Role role;
    std::string id;
};