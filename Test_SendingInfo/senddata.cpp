#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 // Targets Windows 10
#endif

#include <winsock2.h>       // Include networking headers first
#include <httplib.h>
#include <iostream>
#include <string>
/*
windows run command 
   g++ main.cpp -o main.exe -I./include -lws2_32
*/
int main() {
    // 1. Initialize the client with your server address
    httplib::Client cli("http://localhost:8080");

    // 2. Prepare the JSON data exactly like your curl -d flag
    // The R"(...)" syntax allows you to write " without escaping it
    std::string json_data = R"({
        "username": "admin",
        "public_key": "",
        "password": "12345"
    })";

    // 3. Send the POST request
    // Matches: -X POST -H "Content-Type: application/json" -d "..."
    if (auto res = cli.Post("/login", json_data, "application/json")) {
        if (res->status == 200) {
            std::cout << "Success: " << res->body << std::endl;
        } else {
            std::cout << "Server returned error: " << res->status << std::endl;
        }
    } else {
        auto err = res.error();
        std::cerr << "Connection failed. Error code: " << (int)err << std::endl;
    }

    return 0;
}