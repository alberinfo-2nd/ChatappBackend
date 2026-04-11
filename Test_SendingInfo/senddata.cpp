#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 
#endif

#include <winsock2.h>
#include <httplib.h>
#include <iostream>
#include <string>
#include <random>
#include <../include/json.hpp>

using json = nlohmann::json;

std::string generate_random_key(size_t length) {
    const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, (int)characters.size() - 1);

    std::string random_string;
    for (size_t i = 0; i < length; ++i) {
        random_string += characters[distribution(generator)];
    }
    return random_string;
}

int main() {
    httplib::Client cli("http://localhost:8080");
    std::string username, password;

    // --- 1. User Input ---
    std::cout << "Enter Username (optional): ";
    std::getline(std::cin, username);
    std::cout << "Enter Password (optional): ";
    std::getline(std::cin, password);

    // --- 2. Construct Login JSON ---
    json login_json;
    if (!username.empty()) login_json["username"] = username;
    
    // Only add password key if it's not empty
    if (!password.empty()) {
        login_json["password"] = password;
        std::cout << "[DEBUG] Password entered, adding to payload." << std::endl;
    } else {
        std::cout << "[DEBUG] No password entered, omitting from payload." << std::endl;
    }
    
    login_json["public_key"] = generate_random_key(16);

    std::cout << "[DEBUG] Login Post: " << login_json.dump() << std::endl;

    std::string auth_token;
    auto res = cli.Post("/login", login_json.dump(), "application/json");

    if (res && res->status == 200) {
        try {
            auto response_data = json::parse(res->body);
            auth_token = response_data.value("authorizationToken", "");
            std::cout << "[DEBUG] Login Success. Token retrieved." << std::endl;
        } catch (...) {
            std::cerr << "[ERROR] JSON parse failed." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "[ERROR] Login failed. Status: " << (res ? std::to_string(res->status) : "Timeout") << std::endl;
        return 1;
    }

    // --- 3. Construct Message JSON ---
    json msg_json;
    if (!username.empty()) msg_json["user"] = username;
    
    msg_json["authorizationToken"] = auth_token;
    msg_json["message"] = "Hello World";
    msg_json["recipient"] = "user123";

    std::cout << "[DEBUG] Message Post: " << msg_json.dump() << std::endl;

    auto msg_res = cli.Post("/sendMessage", msg_json.dump(), "application/json");
    
    if (msg_res && msg_res->status == 200) {
        std::cout << "[DEBUG] Send Success: " << msg_res->body << std::endl;
    } else {
        std::cerr << "[ERROR] Send Failed." << std::endl;
    }

    return 0;
}/*
curl -X POST 127.0.0.1:8080/login -H "Content-Type: application/json" -d '{"username": "x", "public_key": "x"}'
curl -X POST 127.0.0.1:8080/login -H "Content-Type: application/json" -d '{"username": "jack", "public_key": "jack"}'


curl -X POST 127.0.0.1:8080/sendMessage -H "Content-Type: application/json" -H "authorizationToken: ..." -d '{"authorizationToken":"UCyHYEHFdxl0CF9ylQej7r6SBecH2T97","username": "x", "recipient" : "jack", "message":"hi" }'

curl -X POST 127.0.0.1:8080/sendMessage -H "Content-Type: application/json" -H "{"authorizationToken":"UCyHYEHFdxl0CF9ylQej7r6SBecH2T97"}" -d '{"username": "x", "recipient" : "jackl", "message":"hi" }'


*/