// Test for Windows
#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#include <stdexcept>
#include <iostream>
#include <string>
#include <httplib.h>
#include <json.hpp>
#include <User.h>

//TODO: Make it HTTPS

int main()
{
    UserList<User> userList = UserList<User>();
    UserList<AdminUser> adminUserList = UserList<AdminUser>();

    httplib::Server server;

    // Base response for GET
    server.Get("/", [](const httplib::Request &, httplib::Response &res){
        res.set_content("Hello World!\n", "text/plain");
    });

    /*****************************
    Sign in requests from Front End
    ******************************/
    server.Post("/login", [&userList, &adminUserList](const httplib::Request &request, httplib::Response &response){
        response.status = 200;
        // Format for response message
        nlohmann::json response_message = {
            {"status", "Success"},
            {"message", "User created successfully"}
        };

        try {
            // Extract info from request using Json 
            auto user_data = nlohmann::json::parse(request.body);
            if(!user_data.contains("username")) {
                throw std::invalid_argument("username field was not provided!");
            }

            if(!user_data.contains("public_key")) {
                throw std::invalid_argument("public_key field was not provided!");
            }

            try {
                if(!user_data.contains("password")) {
                    //Normal user
                    User newUser = User(user_data["username"], user_data["public_key"]);
                    userList.addUser(newUser);
                    response_message["authorization_token"] = newUser.getAuthorizationToken();
                } else {
                    //Admin user
                    AdminUser newUser = AdminUser(user_data["username"], user_data["password"], user_data["public_key"]);
                    adminUserList.addUser(newUser);
                    response_message["authorization_token"] = newUser.getAuthorizationToken();
                }
            } catch(std::invalid_argument &e) {
                response.status = 400;
                response_message["status"] = "Failed";
                response_message["message"] = e.what();
            } catch(std::logic_error &e) {
                response.status = 400;
                response_message["status"] = "Failed";
                response_message["message"] = e.what();
            }
        } catch (std::invalid_argument &e) {// Invalid Request 
            response.status = 400;
            response_message["status"] = "Failed";
            response_message["message"] = e.what();
        }

        response.set_content(response_message.dump(), "application/json");
    });

    // TODO: Switch between debug and production for localhost and 0.0.0.0 respectively
    server.listen("127.0.0.1", 8080);
}
