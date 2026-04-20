#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#include <iostream>
#include <string>
//include from BackendProject
#include "../include/httplib.h"
#include "../include/json.hpp"

using json = nlohmann::json;
using namespace std;

int main() {
    httplib::Client cli("http://127.0.0.1:8080");
    string username, authToken;
    bool loggedIn = false;

    cout << "=== Welcome to Messenger ===\n";

    while (true) {
        cout << "\nChoose an option:\n";
        if (!loggedIn) {
            cout << "1. Login\n";
        } else {
            cout << "1. Send Message\n";
            cout << "2. Read Messages\n";
        }
        cout << "0. Exit\n";
        cout << "Selection: ";

        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        if (choice == 1 && !loggedIn) {
            string pubKey;
            cout << "Enter Username: "; cin >> username;
            cout << "Enter Public Key: "; cin >> pubKey;

            json login_json = {{"username", username}, {"public_key", pubKey}};
            auto res = cli.Post("/login", login_json.dump(), "application/json");

            if (res && res->status == 200) {
                auto response_data = json::parse(res->body);
                authToken = response_data.value("authorization_token", "");
                loggedIn = true;
                cout << ">> Login Successful!\n";
            } else if (res && res->status == 400){
                auto response_data = json::parse(res->body);

            } else {
                cout << ">> Login Failed.\n";
            }
        } 
        else if (choice == 2 && loggedIn) {
            string recipient, message;
            cout << "To: "; cin >> recipient;
            cout << "Message: "; cin.ignore(); getline(cin, message);

            json msg_json = {{"username", username}, {"recipient", recipient}, {"message", message}};
            httplib::Headers headers = {{"authorizationToken", authToken}};
            
            auto res = cli.Post("/sendMessage", headers, msg_json.dump(), "application/json");
            if (res && res->status == 200) cout << ">> Sent!\n";
            else cout << ">> Failed to send.\n";
        } 
        else if (choice == 3 && loggedIn) {
            json read_json = {{"username", username}};
            httplib::Headers headers = {{"authorizationToken", authToken}};
            
            httplib::Request req;
            req.method = "GET";
            req.path = "/readMessages";
            req.headers = headers;
            req.body = read_json.dump();
            req.set_header("Content-Type", "application/json");

            auto res = cli.send(req);
            if (res && res->status == 200) {
                try {
                    auto res_json = json::parse(res->body);
                    
                    cout << "\n--- Your Inbox ---" << endl;
                    
                    // Access the "messages" array from the JSON
                    if (res_json.contains("messages") && res_json["messages"].is_array()) {
                        for (const auto& msg : res_json["messages"]) {
                            string sender = msg.value("sender", "Unknown");
                            string content = msg.value("content", "");
                            string time = msg.value("timestamp", "N/A");

                            cout << "[" << time << "] " << sender << ": " << content << endl;
                        }
                    } else {
                        cout << "No messages found." << endl;
                    }
                    cout << "------------------\n" << endl;
                } catch (json::parse_error& e) {
                    cout << ">> Error parsing messages: " << e.what() << endl;
                }
            } else {
                cout << ">> Could not retrieve messages. Status: " << (res ? to_string(res->status) : "Error") << endl;
            }
        }
        else {
            cout << "Invalid choice or login required.\n";
        }
    }

    return 0;
}