#define WIN32_LEAN_AND_MEAN
#include "../libs/crow_all.h"
#include <windows.h>
#include <iostream>
#include <clocale>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "../h/App.h"
#include "../h/Auth.h"
#include "../h/Storage.h"
#include "../h/Recipe.h"
#include "../h/IDGenerator.h"
#include "../h/IngredientManager.h"
#include "../h/SessionManager.h"

using namespace crow;

// ====================== ГЛОБАЛЬНИЙ СТАН СЕСІЇ ======================
std::string session_username = "";
int session_user_role = 0;      // 0 - Користувач, 1 - Адмін
bool session_logged_in = false;

// ====================== ОНОВЛЕННЯ ГЛОБАЛЬНОЇ СЕСІЇ ======================
std::string trim_str(const std::string& str) {
    std::string s = str;
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
    return s;
}

void updateGlobalSession(const std::string& sid) 
{
    auto session = SessionManager::getSession(sid);
    if (session) {
        session_logged_in = true;
        session_username = session->username;
        session_user_role = session->role;   // int
        
        std::cout << "🔑 Сесія активована: " << session_username 
                  << " (Роль: " << (session_user_role == 1 ? "Адмін" : "Користувач") << ")\n";
    } else {
        std::cout << "⚠️ Не вдалося оновити сесію для SID: " << sid << std::endl;
    }
}
bool isGuest() {
    return session_logged_in && session_username == "Гість";
}
// ========================================================================
// =========================== MAIN =======================================
// ========================================================================
int main() 
{

    std::filesystem::create_directories("static/uploads/photos");
    // ====================== НАЛАШТУВАННЯ ЛОКАЛІ ТА КОНСОЛІ ======================
    std::locale::global(std::locale("C"));
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    std::setlocale(LC_ALL, ".UTF8");

    crow::SimpleApp app;

    std::cout << "==================================================\n";
    std::cout << "🌐 Сервер SmartCook успішно запущено!\n";
    std::cout << "🔗 Адреса: http://127.0.0.1:18080\n";
    std::cout << "==================================================\n\n";

    // ========================================================================
    // 1. СТАТИЧНІ ФАЙЛИ (CSS + JavaScript)
    // ========================================================================

    // CSS
    CROW_ROUTE(app, "/static/css/style.css").methods(crow::HTTPMethod::Get)([]() {
        std::ifstream file("static/css/style.css");
        if (!file.is_open()) {
            return crow::response(404, "CSS файл не знайдено");
        }
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        crow::response res(content);
        res.set_header("Content-Type", "text/css");
        return res;
    });

    // JS файли
    CROW_ROUTE(app, "/static/js/<string>")([](std::string filename) {
        std::ifstream file("static/js/" + filename);
        if (!file.is_open()) {
            return crow::response(404, "JS файл не знайдено");
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.set_header("Content-Type", "application/javascript");
        return res;
    });

    // ========================================================================
    // 2. АВТОРИЗАЦІЯ ТА РЕЄСТРАЦІЯ
    // ========================================================================

    // Сторінка входу
   CROW_ROUTE(app, "/login")([]() {
    return crow::mustache::load("login.html").render();
});


CROW_ROUTE(app, "/logout")
([&](const crow::request& req) {
    std::string sid = SessionManager::extractSessionId(req.get_header_value("Cookie"));
    SessionManager::removeSession(sid);
    
    crow::response res;
    res.redirect("/login");
    res.add_header("Set-Cookie", "sid=; Max-Age=0; Path=/");
    return res;
});

CROW_ROUTE(app, "/guest-login").methods(crow::HTTPMethod::Post)
([&](const crow::request& req) {
    std::string sid = SessionManager::createSession("Гість", 0);
    
    session_logged_in = true;
    session_username = "Гість";
    session_user_role = 0;

    crow::response res(R"({"status":"success"})");
    res.add_header("Set-Cookie", "sid=" + sid + "; Path=/; HttpOnly; SameSite=Lax");
    return res;
});

    // API - Вхід
    // ==================== API ВХОДУ (Користувач + Адмін) ====================
CROW_ROUTE(app, "/api/login").methods(crow::HTTPMethod::Post)
([](const crow::request& req) {
    auto json = crow::json::load(req.body);
    if (!json) 
        return crow::response(400, R"({"status":"error","message":"Невірний формат даних"})");

    std::string username = json["username"].s();
    std::string password = json["password"].s();

    UserData user;
    if (!Storage::getUser(username, user) || password != user.password) {
        return crow::response(401, R"({"status":"error","message":"Невірний логін або пароль"})");
    }

    // Змінено: тепер просто int
    int role = user.role;
    std::string sid = SessionManager::createSession(username, role);

    updateGlobalSession(sid);

    crow::response res(R"({"status":"success","redirect":"/"})");
    res.add_header("Set-Cookie", "sid=" + sid + "; Path=/; HttpOnly; SameSite=Lax");
    
    std::cout << "✅ Успішний вхід: " << username 
              << " | Роль: " << (role == 1 ? "Адмін" : "Користувач") << std::endl;
    return res;
});


    // Сторінка та обробка реєстрації
    CROW_ROUTE(app, "/register").methods(crow::HTTPMethod::Get, crow::HTTPMethod::Post)
    ([](const crow::request& req) {
        if (req.method == crow::HTTPMethod::Get) {
            return crow::response(crow::mustache::load("register.html").render());
        }

        crow::response res;
        res.set_header("Content-Type", "application/json");

        try {
            auto params = req.get_body_params();

            std::string username  = params.get("username")  ? params.get("username")  : "";
            std::string password  = params.get("password")  ? params.get("password")  : "";
            std::string email     = params.get("email")     ? params.get("email")     : "";
            std::string diet      = params.get("diet")      ? params.get("diet")      : "-";
            std::string allergies = params.get("allergies") ? params.get("allergies") : "-";

            if (username.empty() || password.empty() || email.empty()) {
                res.code = 400;
                res.body = R"({"status": "error", "message": "Обов'язкові поля порожні!"})";
                return res;
            }

            if (Auth::registerUser(username, password, email, diet, allergies)) {
                std::cout << "✅ Користувач '" << username << "' успішно зареєстрований!\n";
                res.code = 200;
                res.body = R"({"status": "success", "redirect": "/login"})";
            } else {
                res.code = 400;
                res.body = R"({"status": "error", "message": "Цей логін уже зайнятий!"})";
            }
        } catch (const std::exception& e) {
            std::cerr << "Помилка реєстрації: " << e.what() << std::endl;
            res.code = 500;
            res.body = R"({"status": "error", "message": "Внутрішня помилка сервера"})";
        }
        return res;
    });

    // ========================================================================
    // 3. ГОЛОВНА СТОРІНКА
    // ========================================================================

    CROW_ROUTE(app, "/")([]() {
    crow::mustache::context ctx;

    if (session_logged_in) {
        ctx["session_username"] = session_username;
        ctx["isAdmin"] = (session_user_role == 1);
    }

    auto allRecipes = Storage::getAllRecipes();
    ctx["has_recipes"] = (allRecipes.size() > 0);

    crow::json::wvalue::list recipes_list;
    for (const auto& r : allRecipes) {
        crow::json::wvalue r_json;
        r_json["id"] = r.id;
        r_json["name"] = std::string(r.name);
        r_json["description"] = std::string(r.description);
        r_json["calories"] = r.calories;
        recipes_list.push_back(r_json);
    }
    ctx["recipes"] = std::move(recipes_list);

    return crow::mustache::load("index.html").render(ctx);
});

    // API статусу авторизації
  CROW_ROUTE(app, "/api/auth-status")([]() {
    crow::json::wvalue x;
    x["isLoggedIn"] = session_logged_in;
    x["username"] = session_username;
    x["role"] = session_user_role;        // 0 або 1
    return x;
});
// ====================== ПРОФІЛЬ КОРИСТУВАЧА ======================

CROW_ROUTE(app, "/profile")([]() {
    if (!session_logged_in || isGuest()) {
        crow::response res; res.redirect("/"); return res;
    }

    crow::mustache::context ctx;
    ctx["session_username"] = session_username;
    ctx["isAdmin"] = (session_user_role == 1);

    UserData user = Storage::getUserByName(session_username);
    ctx["email"] = std::string(user.email);
    ctx["diet"] = std::string(user.diet);
    ctx["allergies"] = std::string(user.allergies);

    return crow::response(crow::mustache::load("profile.html").render(ctx));
});

// API - Отримати дані профілю
CROW_ROUTE(app, "/api/profile").methods(crow::HTTPMethod::Get)
([]() {
    if (!session_logged_in) 
        return crow::response(401, R"({"status":"error","message":"Не авторизований"})");

    UserData user = Storage::getUserByName(session_username);

    crow::json::wvalue resp;
    resp["username"] = session_username;
    resp["email"] = std::string(user.email);
    resp["diet"] = std::string(user.diet);
    resp["allergies"] = std::string(user.allergies);
    return crow::response(resp);
});

// API - Оновити профіль
CROW_ROUTE(app, "/api/profile").methods(crow::HTTPMethod::Post)
([](const crow::request& req) {
    if (!session_logged_in) 
        return crow::response(401, R"({"status":"error","message":"Не авторизований"})");

    auto json = crow::json::load(req.body);
    if (!json) 
        return crow::response(400, R"({"status":"error","message":"Невірний формат"})");

    std::string email = json["email"].s();
    std::string diet = json["diet"].s();
    std::string allergies = json["allergies"].s();

    UserData user = Storage::getUserByName(session_username);

    if (!email.empty()) strncpy_s(user.email, email.c_str(), sizeof(user.email)-1);
    if (!diet.empty()) strncpy_s(user.diet, diet.c_str(), sizeof(user.diet)-1);
    if (!allergies.empty()) strncpy_s(user.allergies, allergies.c_str(), sizeof(user.allergies)-1);

    Storage::updateUser(user);

    return crow::response(R"({"status":"success","message":"Профіль успішно оновлено!"})");
});

CROW_ROUTE(app, "/api/search/name").methods(crow::HTTPMethod::Get)
([](const crow::request& req) {
    auto query = crow::query_string(req.url_params);
    std::string q = query.get("q") ? query.get("q") : "";

    auto results = Storage::searchRecipesByName(q);

    bool user_has_allergies = false;
    std::string userAllergies = "";
    if (session_logged_in) {
        UserData user = Storage::getUserByName(session_username);
        userAllergies = user.allergies;
        std::transform(userAllergies.begin(), userAllergies.end(), userAllergies.begin(), 
                       [](unsigned char c){ return std::tolower(c); });
        if (!userAllergies.empty()) {
            user_has_allergies = true;
        }
    }
    auto allIngredients = Storage::getAllIngredients();

    crow::json::wvalue::list list;
    for (const auto& r : results) {
        crow::json::wvalue item;
        item["id"] = r.id;
        item["name"] = std::string(r.name);
        item["description"] = std::string(r.description);
        item["calories"] = r.calories;
        item["photo_path"] = std::string(r.photo_path);
        item["video_url"]  = std::string(r.video_url);
        item["author"]     = std::string(r.author);

        bool hasAllergen = false;
        if (user_has_allergies) {
            auto relations = Storage::getRelationsByDish(r.id);
            for (const auto& rel : relations) {
                for (const auto& ing : allIngredients) {
                    if (ing.id == rel.ingredient_id) {
                        std::string ingName = ing.name;
                        std::transform(ingName.begin(), ingName.end(), ingName.begin(),
                                       [](unsigned char c){ return std::tolower(c); });
                        if (userAllergies.find(ingName) != std::string::npos) {
                            hasAllergen = true;
                            break;
                        }
                    }
                }
                if (hasAllergen) break;
            }
        }
        item["hasAllergen"] = hasAllergen;
        list.push_back(std::move(item));
    }
    return crow::response(crow::json::wvalue{{"recipes", std::move(list)}});
});

CROW_ROUTE(app, "/api/search/ingredients").methods(crow::HTTPMethod::Get)
([](const crow::request& req) {
    auto query = crow::query_string(req.url_params);
    std::string ids_str = query.get("ids") ? query.get("ids") : "";

    std::vector<int> ids;
    std::stringstream ss(ids_str);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (!token.empty()) ids.push_back(std::stoi(token));
    }

    auto results = Storage::searchRecipesByIngredients(ids);

    bool user_has_allergies = false;
    std::string userAllergies = "";
    if (session_logged_in) {
        UserData user = Storage::getUserByName(session_username);
        userAllergies = user.allergies;
        std::transform(userAllergies.begin(), userAllergies.end(), userAllergies.begin(), 
                       [](unsigned char c){ return std::tolower(c); });
        if (!userAllergies.empty()) {
            user_has_allergies = true;
        }
    }
    auto allIngredients = Storage::getAllIngredients();

    crow::json::wvalue::list list;
    for (const auto& r : results) {
        crow::json::wvalue item;
        item["id"] = r.id;
        item["name"] = std::string(r.name);
        item["description"] = std::string(r.description);
        item["calories"] = r.calories;
        item["photo_path"] = std::string(r.photo_path);
        item["video_url"]  = std::string(r.video_url);
        item["author"]     = std::string(r.author);

        bool hasAllergen = false;
        if (user_has_allergies) {
            auto relations = Storage::getRelationsByDish(r.id);
            for (const auto& rel : relations) {
                for (const auto& ing : allIngredients) {
                    if (ing.id == rel.ingredient_id) {
                        std::string ingName = ing.name;
                        std::transform(ingName.begin(), ingName.end(), ingName.begin(),
                                       [](unsigned char c){ return std::tolower(c); });
                        if (userAllergies.find(ingName) != std::string::npos) {
                            hasAllergen = true;
                            break;
                        }
                    }
                }
                if (hasAllergen) break;
            }
        }
        item["hasAllergen"] = hasAllergen;
        list.push_back(std::move(item));
    }
    return crow::response(crow::json::wvalue{{"recipes", std::move(list)}});
});

CROW_ROUTE(app, "/api/ingredients")([](){
    auto ings = Storage::getAllIngredients();
    crow::json::wvalue::list list;
    for (const auto& i : ings) {
        crow::json::wvalue item;
        item["id"] = i.id;
        item["name"] = std::string(i.name);
        item["manufacturer"] = std::string(i.manufacturer);
        item["category"] = std::string(i.category);
        item["unit"] = std::string(i.unit);
        list.push_back(item);
    }
    return crow::response(crow::json::wvalue{{"ingredients", list}});
});

    // ========================================================================
    // 4. РЕЦЕПТИ
    // ========================================================================

CROW_ROUTE(app, "/api/recipes")([]() {
    crow::json::wvalue::list list;

    if (!session_logged_in) {
        // Для гостей — просто рецепти без попереджень
        auto recipes = Storage::getAllRecipes();
        for (const auto& r : recipes) {
            crow::json::wvalue item;
            item["id"] = r.id;
            item["name"] = std::string(r.name);
            item["description"] = std::string(r.description);
            item["calories"] = r.calories;
            item["photo_path"] = std::string(r.photo_path);
            item["video_url"]  = std::string(r.video_url);
            item["author"]     = std::string(r.author);
            item["hasAllergen"] = false;
            list.push_back(item);
        }
    } else {
        UserData user = Storage::getUserByName(session_username);
        auto recipes = Storage::getAllRecipesWithAllergenInfo(user);

        for (const auto& rw : recipes) {
            crow::json::wvalue item;
            item["id"] = rw.recipe.id;
            item["name"] = std::string(rw.recipe.name);
            item["description"] = std::string(rw.recipe.description);
            item["calories"] = rw.recipe.calories;
            item["photo_path"] = std::string(rw.recipe.photo_path);
            item["video_url"]  = std::string(rw.recipe.video_url);
            item["author"]     = std::string(rw.recipe.author);
            item["hasAllergen"] = rw.hasAllergen;
            list.push_back(item);
        }
    }

    return crow::response(crow::json::wvalue{{"recipes", std::move(list)}});
});
 
CROW_ROUTE(app, "/recipe/<int>")([](int id) {
    std::cout << "\n=== RECIPE VIEW DEBUG #" << id << " ===\n";

    auto recipes = Storage::getAllRecipes();
    RecipeData* target = nullptr;
    for (auto& r : recipes) {
        if (r.id == id) {
            target = &r;
            break;
        }
    }

    if (!target) {
        std::cout << "❌ Рецепт не знайдено!\n";
        return crow::response(404);
    }

    auto relations = Storage::getRelationsByDish(id);
    std::cout << "Зв'язків (relations) знайдено: " << relations.size() << std::endl;

    auto all_ing = Storage::getAllIngredients();

    crow::json::wvalue ingredients_list = crow::json::wvalue::list();
    int count = 0;

    for (const auto& rel : relations) {
        for (const auto& ing : all_ing) {
            if (ing.id == rel.ingredient_id) {
                crow::json::wvalue item;
                item["id"] = ing.id;
                item["name"] = std::string(ing.name);
                item["qty"] = (double)rel.quantity;
                item["unit"] = std::string(ing.unit);
                ingredients_list[count++] = std::move(item);
                break;
            }
        }
    }

    // ====================== ОБРОБКА YOUTUBE ======================
    std::string embed_url = "";
    std::string video_url = trim_str(target->video_url);

    if (!video_url.empty()) {
        std::string video_id = "";
        size_t pos = std::string::npos;

        if ((pos = video_url.find("watch?v=")) != std::string::npos) {
            video_id = video_url.substr(pos + 8, 11);
        }
        else if ((pos = video_url.find("youtu.be/")) != std::string::npos) {
            video_id = video_url.substr(pos + 9, 11);
        }
        else if ((pos = video_url.find("embed/")) != std::string::npos) {
            video_id = video_url.substr(pos + 6, 11);
        }
        else if ((pos = video_url.find("shorts/")) != std::string::npos) {
            video_id = video_url.substr(pos + 7, 11);
        }
        else if (video_url.length() == 11) {
            video_id = video_url;
        }

        if (video_id.length() == 11) {
            embed_url = "https://www.youtube.com/embed/" + video_id;
        }
    }

    std::cout << "Відео URL: " << video_url << "\n";
    std::cout << "Embed URL: " << (embed_url.empty() ? "НЕМАЄ" : embed_url) << "\n";

    // ====================== КОНТЕКСТ ======================
    crow::mustache::context ctx;
    ctx["id"] = target->id;
    ctx["name"] = target->name;
    ctx["author"] = target->author;
    ctx["calories"] = target->calories;
    ctx["description"] = target->description;
    ctx["instructions"] = target->instructions;
    ctx["photo_path"] = target->photo_path;
    ctx["video_url"] = embed_url;           // ← Тепер передаємо embed
    ctx["ingredients_json"] = ingredients_list.dump();
    ctx["ingredients_count"] = count;

    if (session_logged_in) {
        ctx["session_username"] = session_username;
    }
 
        // Перевірка прав на редагування
    bool isOwner = (std::string(target->author) == session_username);
    bool isAdmin = (session_user_role == 1);
    ctx["isEditable"] = (isOwner || isAdmin);
    ctx["isDeletable"] = (isOwner || isAdmin);
    ctx["isAdmin"] = isAdmin;

    std::cout << "Всього інгредієнтів для відображення: " << count << "\n";
    std::cout << "========================\n";

    return crow::response(crow::mustache::load("recipe_view.html").render(ctx));
});

    // Створення рецепта (GET + POST)
 // Створення рецепта (GET + POST)
CROW_ROUTE(app, "/create").methods(crow::HTTPMethod::Get, crow::HTTPMethod::Post)
([](const crow::request& req) {
   if (req.method == crow::HTTPMethod::Get) {
    if (!session_logged_in || isGuest()) {
        crow::response res; res.redirect("/"); return res;
    }

        // Правильне оголошення контексту
        crow::mustache::context ctx;
        ctx["session_username"] = session_username;
        ctx["isAdmin"] = (session_user_role == 1);

        auto all_ing = Storage::getAllIngredients();
        crow::json::wvalue ing_list;
        for (size_t i = 0; i < all_ing.size(); ++i) {
            ing_list[i]["id"] = all_ing[i].id;
            ing_list[i]["name"] = std::string(all_ing[i].name);
            ing_list[i]["unit"] = std::string(all_ing[i].unit);
        }
        ctx["all_ingredients"] = std::move(ing_list);

        return crow::response(crow::mustache::load("create.html").render(ctx));
    }

    // ====================== POST ======================
    try {
        crow::multipart::message mp(req);

        std::cout << "\n=== CREATE RECIPE DEBUG ===\n";
        std::cout << "Кількість multipart частин: " << mp.parts.size() << std::endl;

        RecipeData r{};
        memset(&r, 0, sizeof(RecipeData));

        r.id = std::stoi(IDGenerator::generate("recipe_id.txt"));

        // Основні поля
        safe_copy_str(r.name,       mp.get_part_by_name("name").body,        sizeof(r.name));
        safe_copy_str(r.description,mp.get_part_by_name("description").body, sizeof(r.description));
        safe_copy_str(r.instructions,mp.get_part_by_name("instructions").body,sizeof(r.instructions));
        safe_copy_str(r.type,       mp.get_part_by_name("type").body,        sizeof(r.type));
        safe_copy_str(r.video_url,  trim_str(mp.get_part_by_name("video_url").body),   sizeof(r.video_url));
         auto photo_part = mp.get_part_by_name("recipe_photo");
        if (!photo_part.body.empty()) {
            // Визначаємо розширення з Content-Type
            std::string ext = ".jpg";
            std::string ct = photo_part.get_header_object("Content-Type").value;
            if (ct.find("png") != std::string::npos)  ext = ".png";
            else if (ct.find("webp") != std::string::npos) ext = ".webp";
            else if (ct.find("gif") != std::string::npos)  ext = ".gif";
 
            std::string filename = "static/uploads/photos/" + std::to_string(r.id) + ext;
 
            std::ofstream photo_file(filename, std::ios::binary);
            if (photo_file.is_open()) {
                photo_file.write(photo_part.body.data(), photo_part.body.size());
                photo_file.close();
                // Зберігаємо відносний шлях у структуру рецепту
                safe_copy_str(r.photo_path, filename, sizeof(r.photo_path));
                std::cout << "[PHOTO] Збережено: " << filename << std::endl;
            } else {
                std::cerr << "[PHOTO] Не вдалося відкрити файл для запису: " << filename << std::endl;
            }
        }
        safe_copy_str(r.author,     session_username.c_str(),                sizeof(r.author));

        if (!mp.get_part_by_name("calories").body.empty()) {
            r.calories = std::stoi(mp.get_part_by_name("calories").body);
        }

        std::cout << "Рецепт #" << r.id << " | Назва: " << r.name 
                  << " | Автор: " << r.author << std::endl;

        // ==================== ІНГРЕДІЄНТИ ====================
        auto ing_part = mp.get_part_by_name("ingredients_json");
        
        if (!ing_part.body.empty()) {
            auto json_val = crow::json::load(ing_part.body);
            if (json_val) {
                for (const auto& item : json_val) {
                    DishIngredient rel{};
                    rel.dish_id = r.id;

                    if (item.has("id"))  rel.ingredient_id = item["id"].i();
                    if (item.has("qty")) rel.quantity = item["qty"].d();

                    if (rel.ingredient_id > 0 && rel.quantity > 0.0f) {
                        Storage::saveDishRelation(rel);
                    }
                }
            }
        }

        Storage::saveRecipe(r);
        std::cout << "✅ Рецепт #" << r.id << " успішно збережений!\n";

        crow::response res;
        res.code = 303;
        res.set_header("Location", "/");
        return res;

    } catch (const std::exception& e) {
        std::cerr << "[CRITICAL] Помилка створення рецепта: " << e.what() << std::endl;
        return crow::response(500, R"({"status":"error","message":"Внутрішня помилка сервера"})");
    }
});

 CROW_ROUTE(app, "/static/uploads/photos/<string>")([](std::string filename) {
        // Безпека: забороняємо ../ у назві файлу
        if (filename.find("..") != std::string::npos || filename.find('/') != std::string::npos) {
            return crow::response(403);
        }
        std::string path = "static/uploads/photos/" + filename;
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) return crow::response(404);
 
        std::string content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
 
        crow::response res(content);
        // Визначаємо Content-Type
        auto ends_with = [](const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
};

if (ends_with(filename, ".png")) res.set_header("Content-Type", "image/png");
else if (ends_with(filename, ".webp")) res.set_header("Content-Type", "image/webp");
else if (ends_with(filename, ".gif")) res.set_header("Content-Type", "image/gif");
else if (ends_with(filename, ".jpg") || ends_with(filename, ".jpeg")) 
    res.set_header("Content-Type", "image/jpeg");
        return res;
    });
 

// ====================== РЕДАГУВАННЯ ТА ВИДАЛЕННЯ РЕЦЕПТУ ======================

// Сторінка редагування рецепту
// Сторінка редагування рецепту
CROW_ROUTE(app, "/recipe/<int>/edit")([](int id) {
    if (!session_logged_in) {
        crow::response res;
        res.redirect("/login");
        return res;
    }

    // Отримуємо рецепт
    auto recipes = Storage::getAllRecipes();
    RecipeData* target = nullptr;
    for (auto& r : recipes) {
        if (r.id == id) {
            target = &r;
            break;
        }
    }
    if (!target) return crow::response(404);

    // Перевірка прав
    bool isOwner = (std::string(target->author) == session_username);
    bool isAdmin = (session_user_role == 1);

    if (!isOwner && !isAdmin) {
        return crow::response(403, "Немає прав на редагування цього рецепту");
    }

    // Правильне створення контексту
    crow::mustache::context ctx;
    ctx["id"] = target->id;
    ctx["name"] = target->name;
    ctx["description"] = target->description;
    ctx["instructions"] = target->instructions;
    ctx["calories"] = target->calories;
    ctx["video_url"] = target->video_url;
    ctx["session_username"] = session_username;
    ctx["isAdmin"] = (session_user_role == 1);

    return crow::response(crow::mustache::load("edit_recipe.html").render(ctx));
});

// API: Оновлення рецепту
CROW_ROUTE(app, "/api/recipe/<int>").methods(crow::HTTPMethod::Post)
([](const crow::request& req, int id) {
    if (!session_logged_in) return crow::response(401);

    auto json = crow::json::load(req.body);
    if (!json) return crow::response(400);

    auto recipes = Storage::getAllRecipes();
    RecipeData updated;
    bool found = false;

    for (auto& r : recipes) {
        if (r.id == id) {
            updated = r;
            found = true;
            break;
        }
    }
    if (!found) return crow::response(404);

    bool isOwner = (std::string(updated.author) == session_username);
    bool isAdmin = (session_user_role == 1);

    if (!isOwner && !isAdmin) return crow::response(403);

    // Оновлюємо дані
    if (json.has("name")) safe_copy_str(updated.name, json["name"].s(), sizeof(updated.name));
    if (json.has("description")) safe_copy_str(updated.description, json["description"].s(), sizeof(updated.description));
    if (json.has("instructions")) safe_copy_str(updated.instructions, json["instructions"].s(), sizeof(updated.instructions));
    if (json.has("calories")) updated.calories = json["calories"].i();
    if (json.has("video_url")) safe_copy_str(updated.video_url, trim_str(json["video_url"].s()), sizeof(updated.video_url));

    if (Storage::updateRecipe(updated)) {
        return crow::response(R"({"status":"success"})");
    }
    return crow::response(500, R"({"status":"error"})");
});

// API: Видалення рецепту
CROW_ROUTE(app, "/api/recipe/<int>").methods(crow::HTTPMethod::Delete)
([&](const crow::request& req, int id) {
    std::string sid = SessionManager::extractSessionId(req.get_header_value("Cookie"));
    Session* session = SessionManager::getSession(sid);
    if (!session) return crow::response(401, R"({"status":"error","message":"Не авторизовано"})");

    auto recipes = Storage::getAllRecipes();
    bool canDelete = false;

    for (const auto& r : recipes) {
        if (r.id == id) {
            if (std::string(r.author) == session->username || session->role == 1) {
                canDelete = true;
            }
            break;
        }
    }

    if (!canDelete) return crow::response(403, R"({"status":"error","message":"Немає прав"})");

    Storage::deleteRecipe(id);
    return crow::response(200, R"({"status":"success"})");
});

    // ========================================================================
    // 5. ОБРАНЕ (Favorites)
    // ========================================================================

    // ========================================================================
// СТОРІНКА ОБРАНИХ РЕЦЕПТІВ
// ========================================================================
CROW_ROUTE(app, "/favorites").methods(crow::HTTPMethod::Get)([]() {
    if (!session_logged_in || isGuest()) {
        crow::response res; res.redirect("/"); return res;
    }

    crow::mustache::context ctx;
    ctx["session_username"] = session_username;
    ctx["isAdmin"] = (session_user_role == 1);

    auto favoriteRecipes = Storage::getFavoritesByUsername(session_username);

    crow::json::wvalue::list fav_list;
    for (const auto& r : favoriteRecipes) {
        crow::json::wvalue item;
        item["id"] = r.id;
        item["name"] = std::string(r.name);
        item["description"] = std::string(r.description);
        item["calories"] = r.calories;
        fav_list.push_back(item);
    }

    ctx["favorites"] = std::move(fav_list);

    return crow::response(crow::mustache::load("favorites.html").render(ctx));
});

    // API Додати в обране
    // ==================== API ДОДАВАННЯ В ОБРАНЕ ====================
CROW_ROUTE(app, "/api/add-favorite").methods(crow::HTTPMethod::Post)
([](const crow::request& req) {
    std::string cookie = req.get_header_value("Cookie");
    std::string sid = SessionManager::extractSessionId(cookie);

    std::cout << "[ADD-FAVORITE] Cookie: " << cookie << std::endl;
    std::cout << "[ADD-FAVORITE] Extracted SID: '" << sid << "'" << std::endl;

    if (sid.empty()) {
        return crow::response(403, R"({"status":"error","message":"Сесія не знайдена"})");
    }

    auto session = SessionManager::getSession(sid);
    if (!session) {
        return crow::response(403, R"({"status":"error","message":"Сесія недійсна"})");
    }

    updateGlobalSession(sid);

    // === ПОКРАЩЕНИЙ ПАРСИНГ BODY ===
    std::string body_str = req.body;
    std::cout << "[ADD-FAVORITE] Body: " << body_str << std::endl;

    int recipe_id = 0;
    size_t pos = body_str.find("recipe_id=");
    if (pos != std::string::npos) {
        std::string value = body_str.substr(pos + 10);
        size_t end = value.find('&');
        if (end != std::string::npos) value = value.substr(0, end);
        recipe_id = std::stoi(value);
    }

    if (recipe_id <= 0) {
        std::cout << "❌ recipe_id не знайдено або некоректний!" << std::endl;
        return crow::response(400, R"({"status":"error","message":"Відсутній або некоректний recipe_id"})");
    }

    std::cout << "✅ УСПІШНО додано в улюблене: рецепт #" << recipe_id 
              << " | Користувач: " << session->username << std::endl;
              
    Storage::addFavorite(session->username, recipe_id);

    return crow::response(R"({"status":"success"})");
});
    // API Видалити з обраного
  // ==================== ПЕРЕВІРКА ЧИ В УЛЮБЛЕНИХ ====================
CROW_ROUTE(app, "/api/check-favorite")
([](const crow::request& req) {
    std::string cookie = req.get_header_value("Cookie");
    std::string sid = SessionManager::extractSessionId(cookie);

    if (sid.empty()) 
        return crow::json::wvalue({{"isFavorite", false}});

    auto session = SessionManager::getSession(sid);
    if (!session) 
        return crow::json::wvalue({{"isFavorite", false}});

    auto query = crow::query_string(req.url_params);
    int recipe_id = std::stoi(query.get("recipe_id") ? query.get("recipe_id") : "0");

    bool is_fav = Storage::isFavorite(session->username, recipe_id);

    crow::json::wvalue resp;
    resp["isFavorite"] = is_fav;
    return resp;
});

// ==================== ВИДАЛЕННЯ З УЛЮБЛЕНИХ ====================
// ==================== ВИДАЛЕННЯ З УЛЮБЛЕНИХ ====================
CROW_ROUTE(app, "/api/remove-favorite").methods(crow::HTTPMethod::Post)
([](const crow::request& req) {
    std::string cookie = req.get_header_value("Cookie");
    std::string sid = SessionManager::extractSessionId(cookie);

    std::cout << "[REMOVE-FAVORITE] Cookie: " << cookie << std::endl;
    std::cout << "[REMOVE-FAVORITE] Extracted SID: '" << sid << "'" << std::endl;

    if (sid.empty()) {
        return crow::response(403, R"({"status":"error","message":"Сесія не знайдена"})");
    }

    auto session = SessionManager::getSession(sid);
    if (!session) {
        return crow::response(403, R"({"status":"error","message":"Сесія недійсна"})");
    }

    // === НАДІЙНИЙ ПАРСИНГ recipe_id ===
    std::string body = req.body;
    std::cout << "[REMOVE-FAVORITE] Body: '" << body << "'" << std::endl;

    int recipe_id = 0;
    size_t pos = body.find("recipe_id=");
    if (pos != std::string::npos) {
        std::string value = body.substr(pos + 10);
        size_t end = value.find('&');
        if (end != std::string::npos) value = value.substr(0, end);
        
        try {
            recipe_id = std::stoi(value);
        } catch (...) {
            recipe_id = 0;
        }
    }

    if (recipe_id <= 0) {
        std::cout << "❌ recipe_id не знайдено або некоректний!" << std::endl;
        return crow::response(400, R"({"status":"error","message":"recipe_id відсутній або некоректний"})");
    }

    Storage::removeFavorite(session->username, recipe_id);

    std::cout << "🗑️ Видалено з улюбленого: рецепт #" << recipe_id 
              << " | Користувач: " << session->username << std::endl;

    return crow::response(R"({"status":"success"})");
});
    // ========================================================================
    // 6. ІНШІ API
    // ========================================================================
// Скарга на рецепт
CROW_ROUTE(app, "/api/report-recipe").methods(crow::HTTPMethod::Post)
([&](const crow::request& req) {
    std::string sid = SessionManager::extractSessionId(req.get_header_value("Cookie"));
    Session* session = SessionManager::getSession(sid);
    if (!session) {
        return crow::response(401, R"({"status":"error","message":"Не авторизовано"})");
    }

    auto body = crow::json::load(req.body);
    if (!body) {
        return crow::response(400, R"({"status":"error","message":"Невірний JSON"})");
    }

    int recipe_id = body["recipe_id"].i();
    std::string reason = body["reason"].s();

    std::cout << "[Report] recipe_id=" << recipe_id 
              << ", reason='" << reason << "'" << std::endl;

    if (recipe_id <= 0 || reason.empty()) {
        return crow::response(400, R"({"status":"error","message":"Невірні дані"})");
    }

    Storage::addReport(session->username, recipe_id, reason);
    return crow::response(R"({"status":"success"})");
});
  

CROW_ROUTE(app, "/api/report/<int>").methods(crow::HTTPMethod::Delete)
([](int recipeId) {
    if (!session_logged_in || session_user_role != 1) {
        return crow::response(403);
    }

    Storage::removeSingleReport(recipeId);
    return crow::response(R"({"status":"success"})");
});

    // ====================== СПИСОК ПОКУПОК ======================

// ====================== СПИСОК ПОКУПОК ======================

// Сторінка кошика
// ========================================================================
// СТОРІНКА СПИСКУ ПОКУПОК
// ========================================================================
CROW_ROUTE(app, "/shopping-list").methods(crow::HTTPMethod::Get)([]() {
    if (!session_logged_in || isGuest()) {
        crow::response res; res.redirect("/"); return res;
    }

    crow::mustache::context ctx;
    ctx["session_username"] = session_username;
    ctx["isAdmin"] = (session_user_role == 1);

    // ====================== СИНХРОНІЗАЦІЯ З УЛЮБЛЕНИХ ======================
    auto existingItems = Storage::getShoppingList();
    std::vector<ShoppingItem> manualOnly;
    for (const auto& item : existingItems) {
        if (std::string(item.source) == "manual") {
            manualOnly.push_back(item);
        }
    }

    Storage::clearShoppingList();
    for (const auto& item : manualOnly) {
        Storage::addToShoppingList(item.ingredient_id, item.quantity, "manual");
    }

    auto favRecipes = Storage::getFavoritesByUsername(session_username);
    for (const auto& recipe : favRecipes) {
        auto relations = Storage::getRelationsByDish(recipe.id);
        for (const auto& rel : relations) {
            Storage::addToShoppingList(rel.ingredient_id, rel.quantity, "favorites");
        }
    }

    auto items = Storage::getShoppingList();

    crow::json::wvalue::list json_list;
    for (const auto& item : items) {
        crow::json::wvalue obj;
        obj["ingredient_id"] = item.ingredient_id;
        obj["name"] = std::string(item.name);
        obj["quantity"] = (double)item.quantity;
        obj["unit"] = std::string(item.unit);
        obj["source"] = std::string(item.source);
        json_list.push_back(std::move(obj));
    }

    ctx["shopping_items"] = crow::json::wvalue(std::move(json_list)).dump();
    ctx["has_items"] = !items.empty();

    return crow::response(crow::mustache::load("shopping_list.html").render(ctx));
});
// В main.cpp
// === ПРАВИЛЬНИЙ ВАРІАНТ ===
CROW_ROUTE(app, "/api/shopping-list").methods(crow::HTTPMethod::Get)([]() {
    if (!session_logged_in) {
        return crow::response(401, R"({"status":"error","message":"Не авторизований"})");
    }

    auto items = Storage::getShoppingList();
    
    crow::json::wvalue::list json_list;
    for (const auto& item : items) {
        crow::json::wvalue obj;
        obj["ingredient_id"] = item.ingredient_id;
        obj["name"] = std::string(item.name);
        obj["quantity"] = (double)item.quantity;
        obj["unit"] = std::string(item.unit);
        obj["source"] = std::string(item.source);
        json_list.push_back(std::move(obj));
    }

    crow::json::wvalue response_json;
    response_json = std::move(json_list);   // ← Важливо!

    return crow::response(response_json);
});

// API Додати в кошик
// API Додати в список покупок
CROW_ROUTE(app, "/api/add-to-shopping-list").methods(crow::HTTPMethod::Post)
([](const crow::request& req) {
    if (!session_logged_in) {
        return crow::response(401, R"({"status":"error","message":"Не авторизований"})");
    }

    std::string body = req.body;
    std::cout << "[ADD-TO-SHOPPING] Body: " << body << std::endl;

    int ingredient_id = 0;
    double quantity = 0.0;
    std::string source = "manual";

    // Парсинг form-urlencoded
    size_t pos1 = body.find("ingredient_id=");
    size_t pos2 = body.find("quantity=");

    if (pos1 != std::string::npos) {
        std::string val = body.substr(pos1 + 14);
        size_t end = val.find('&');
        if (end != std::string::npos) val = val.substr(0, end);
        ingredient_id = std::stoi(val);
    }

    if (pos2 != std::string::npos) {
        std::string val = body.substr(pos2 + 9);
        size_t end = val.find('&');
        if (end != std::string::npos) val = val.substr(0, end);
        quantity = std::stod(val);
    }
    size_t pos3 = body.find("source=");
    if (pos3 != std::string::npos) {
        std::string val = body.substr(pos3 + 7);
        size_t end = val.find('&');
        if (end != std::string::npos) val = val.substr(0, end);
        source = val; // буде "favorites" або "manual"
    }
    
    if (ingredient_id > 0 && quantity > 0) {
        Storage::addToShoppingList(ingredient_id, quantity, source);
        std::cout << "[ADD-TO-SHOPPING] Успішно додано: ID=" << ingredient_id << ", qty=" << quantity << std::endl;
        return crow::response(R"({"status":"success"})");
    }

    std::cout << "[ADD-TO-SHOPPING] Помилка парсингу!" << std::endl;
    return crow::response(400, R"({"status":"error","message":"Невірні дані"})");
});
// ========================================================================
// API: Всі інгредієнти з улюблених рецептів користувача
// ========================================================================
CROW_ROUTE(app, "/api/favorite-ingredients").methods(crow::HTTPMethod::Get)
([](const crow::request& req) {
    if (!session_logged_in) {
        return crow::response(401, R"({"status":"error"})");
    }

    // Беремо всі улюблені рецепти
    auto favRecipes = Storage::getFavoritesByUsername(session_username);
    auto allIng     = Storage::getAllIngredients();

    // Збираємо інгредієнти, сумуючи однакові
    std::map<int, double> totals; // ingredient_id → сумарна кількість

    for (const auto& recipe : favRecipes) {
        auto relations = Storage::getRelationsByDish(recipe.id);
        for (const auto& rel : relations) {
            totals[rel.ingredient_id] += rel.quantity;
        }
    }

    // Формуємо JSON-відповідь
    crow::json::wvalue::list result;
    for (const auto& [ing_id, qty] : totals) {
        for (const auto& ing : allIng) {
            if (ing.id == ing_id) {
                crow::json::wvalue obj;
                obj["id"]   = ing.id;
                obj["name"] = std::string(ing.name);
                obj["qty"]  = qty;
                obj["unit"] = std::string(ing.unit);
                result.push_back(std::move(obj));
                break;
            }
        }
    }

    crow::json::wvalue resp;
    resp = std::move(result);
    return crow::response(resp);
});

// Очистити кошик
CROW_ROUTE(app, "/api/clear-shopping-list").methods(crow::HTTPMethod::Post)
([]() {
    if (!session_logged_in) return crow::response(401);
    Storage::clearShoppingList();
    return crow::response(R"({"status":"success"})");
});

// ====================== АДМІН-ПАНЕЛЬ ======================

CROW_ROUTE(app, "/admin")([]() {
    if (!session_logged_in || session_user_role != 1) {
        crow::response res; 
        res.redirect("/"); 
        return res;
    }

    crow::mustache::context ctx;
    ctx["session_username"] = session_username;
    ctx["isAdmin"] = true;

    return crow::response(crow::mustache::load("admin.html").render(ctx));
});
// Статистика
CROW_ROUTE(app, "/admin/stats")([]() {
    if (!session_logged_in || session_user_role != 1) return crow::response(403);

    crow::mustache::context ctx;
    auto users = Storage::getAllUsers();
    auto recipes = Storage::getAllRecipes();
    auto ingredients = Storage::getAllIngredients();

    int admins = 0;
    for (const auto& u : users) if (u.role == 1) admins++;

    ctx["total_users"] = users.size();
    ctx["total_admins"] = admins;
    ctx["total_recipes"] = recipes.size();
    ctx["total_ingredients"] = ingredients.size();

    return crow::response(crow::mustache::load("admin_stats.html").render(ctx));
});



// Сторінка скарг
CROW_ROUTE(app, "/admin/reports")([]() {
    if (!session_logged_in || session_user_role != 1) {
        crow::response res; res.redirect("/"); return res;
    }

    crow::mustache::context ctx;
    ctx["session_username"] = session_username;
    ctx["isAdmin"] = true;

    auto reports = Storage::getAllReports();
    auto allRecipes = Storage::getAllRecipes();

    std::cout << "[Admin Reports] Завантажено " << reports.size() << " скарг з файлу\n";

    crow::json::wvalue::list reports_list;

    for (const auto& report : reports) {
        crow::json::wvalue item;
        item["recipe_id"] = report.recipe_id;
        item["reporter"] = std::string(report.username);
        item["reason"] = std::string(report.reason);
        item["report_date"] = std::string(report.report_date);

        bool found = false;
        for (const auto& recipe : allRecipes) {
            if (recipe.id == report.recipe_id) {
                item["recipe_name"] = std::string(recipe.name);
                found = true;
                break;
            }
        }

        if (!found) {
            item["recipe_name"] = "(Рецепт видалено або не існує)";
        }

        reports_list.push_back(item);
    }

    ctx["reports"] = std::move(reports_list);
    ctx["has_reports"] = !reports_list.empty();

    std::cout << "[Admin Reports] Підготовлено для відображення: " << reports_list.size() << " записів\n";

    return crow::response(crow::mustache::load("admin_reports.html").render(ctx));
});


// Сторінка
CROW_ROUTE(app, "/admin/ingredients")
([&]() {
    if (!session_logged_in || session_user_role != 1) {
        crow::response res; res.redirect("/"); return res;
    }
    crow::mustache::context ctx;
    ctx["session_username"] = session_username;
    return crow::response(crow::mustache::load("admin_ingredients.html").render(ctx));
});

// Додавання
CROW_ROUTE(app, "/api/ingredients").methods(crow::HTTPMethod::Post)
([&](const crow::request& req) {
    if (!session_logged_in || session_user_role != 1)
        return crow::response(403);

    auto body = crow::json::load(req.body);
    if (!body) return crow::response(400);

    std::string s_name         = std::string(body["name"].s());
    std::string s_manufacturer = std::string(body["manufacturer"].s());
    std::string s_category     = std::string(body["category"].s());
    std::string s_unit         = std::string(body["unit"].s());

    IngredientData ing{};
    ing.id = std::stoi(IDGenerator::generate("ing_id.txt"));
    strncpy_s(ing.name,         sizeof(ing.name),         s_name.c_str(),         _TRUNCATE);
    strncpy_s(ing.manufacturer, sizeof(ing.manufacturer), s_manufacturer.c_str(), _TRUNCATE);
    strncpy_s(ing.category,     sizeof(ing.category),     s_category.c_str(),     _TRUNCATE);
    strncpy_s(ing.unit,         sizeof(ing.unit),         s_unit.c_str(),         _TRUNCATE);

    Storage::saveIngredient(ing);
    return crow::response(R"({"status":"success"})");
});

// Видалення
CROW_ROUTE(app, "/api/ingredients/<int>").methods(crow::HTTPMethod::Delete)
([&](int id) {
    if (!session_logged_in || session_user_role != 1)
        return crow::response(403);

    IngredientManager im;
    im.deleteIngredient(id);
    return crow::response(R"({"status":"success"})");
});

    // ========================================================================
    // ЗАПУСК СЕРВЕРА
    // ========================================================================

    app.port(18080).multithreaded().run();

    return 0;
}