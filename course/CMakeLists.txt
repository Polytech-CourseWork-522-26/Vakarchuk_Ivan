cmake_minimum_required(VERSION 3.10)
project(crow_server)

set(CMAKE_CXX_STANDARD 17)

add_executable(crow_server
    cpp/main.cpp
    cpp/App.cpp
    cpp/Auth.cpp
    cpp/IDGenerator.cpp
    cpp/IngredientManager.cpp
    cpp/Recipe.cpp
    cpp/Storage.cpp
    cpp/User.cpp
)

target_include_directories(crow_server PRIVATE
    ${CMAKE_SOURCE_DIR}/cpp
    ${CMAKE_SOURCE_DIR}/h
    ${CMAKE_SOURCE_DIR}/libs/crow/include
    ${CMAKE_SOURCE_DIR}/libs/asio
)
