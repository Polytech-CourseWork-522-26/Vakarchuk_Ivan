#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <string>
#include <unordered_map>
#include <ctime>

struct Session {
    std::string username;
    int role = 0;           // 0 - user, 1 - admin
    time_t created_at = 0;
};

class SessionManager {
public:
    static std::string createSession(const std::string& username, int role);
    static Session* getSession(const std::string& sid);
    static std::string extractSessionId(const std::string& cookie_header);
    static void removeSession(const std::string& sid);

private:
    static std::unordered_map<std::string, Session> sessions;
};

#endif