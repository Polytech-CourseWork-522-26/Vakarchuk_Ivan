#include "SessionManager.h"
#include <iostream>
#include <string>

std::unordered_map<std::string, Session> SessionManager::sessions;

// ====================== СТВОРЕННЯ СЕСІЇ ======================
std::string SessionManager::createSession(const std::string& username, int role)
{
    std::string sid = "sid_" + username + "_" + std::to_string(time(nullptr));
    
    Session s;
    s.username = username;
    s.role = role;
    s.created_at = time(nullptr);
    
    sessions[sid] = s;
    
    std::cout << "[Session] Створено сесію для " << username 
              << " | SID: " << sid << std::endl;
    return sid;
}

// ====================== ОТРИМАННЯ СЕСІЇ ======================
Session* SessionManager::getSession(const std::string& sid)
{
    auto it = sessions.find(sid);
    if (it != sessions.end()) {
        return &it->second;
    }
    return nullptr;
}

// ====================== ВИТЯГ SID З COOKIE ======================
std::string SessionManager::extractSessionId(const std::string& cookie_header)
{
    if (cookie_header.empty()) {
        std::cout << "[DEBUG] Cookie Header: (порожній)" << std::endl;
        return "";
    }

    std::cout << "[DEBUG] Cookie Header: " << cookie_header << std::endl;

    size_t pos = cookie_header.find("sid=");
    if (pos != std::string::npos) {
        pos += 4;
        size_t end = cookie_header.find(';', pos);
        if (end == std::string::npos) 
            end = cookie_header.length();

        std::string sid = cookie_header.substr(pos, end - pos);
        
        // Прибираємо пробіли
        sid.erase(0, sid.find_first_not_of(" \t"));
        sid.erase(sid.find_last_not_of(" \t") + 1);

        std::cout << "[DEBUG] Extracted SID: '" << sid << "'" << std::endl;
        return sid;
    }

    std::cout << "[DEBUG] SID не знайдено в cookie" << std::endl;
    return "";
}

// ====================== ВИДАЛЕННЯ СЕСІЇ ======================
void SessionManager::removeSession(const std::string& sid)
{
    sessions.erase(sid);
}