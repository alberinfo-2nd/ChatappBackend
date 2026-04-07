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

        auto user_data = nlohmann::json::parse(request.body);
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

        response.set_content(response_message.dump(), "application/json");
    });

    server.set_pre_request_handler([&userList, &adminUserList](const httplib::Request &request, httplib::Response& response) {
        nlohmann::json response_message = {
            {"status", "Success"},
            {"message", "User created successfully"}
        };

        auto user_data = nlohmann::json{};

        try {
            if(request.body.size() == 0) {
                throw std::invalid_argument("Body has size zero!");
            }

            // Extract info from request using Json 
            user_data = nlohmann::json::parse(request.body);
            if(!user_data.contains("username")) {
                throw std::invalid_argument("username field was not provided!");
            }

            if(!user_data.contains("public_key")) {
                throw std::invalid_argument("public_key field was not provided!");
            }
        
            if(!request.has_header("authorizationToken") && request.path != "/login") {
                throw std::invalid_argument("authorizationToken field was not provided!");
            }
        } catch(std::invalid_argument &e) {
            response.status = 400;
            response_message["status"] = "Failed";
            response_message["message"] = e.what();

            response.set_content(response_message.dump(), "application/json");
            return httplib::Server::HandlerResponse::Handled; //Does not continue to intended routing, exiting early.
        }

        if(request.path == "/login") return httplib::Server::HandlerResponse::Unhandled;

        std::string username = user_data["username"];
        std::string authToken = request.get_header_value("authorizationToken");
        if(userList.verifyAuthToken(username, authToken) || adminUserList.verifyAuthToken(username, authToken)) {
            return httplib::Server::HandlerResponse::Unhandled; //Continue to normal routing
        }

        response.status = 403;
        response_message["status"] = "Failed";
        response_message["message"] = "Invalid authorization token or username";

        response.set_content(response_message.dump(), "application/json");
        return httplib::Server::HandlerResponse::Handled;  // User was not authorized!
    });


    /*****************
    get request to send array of active users to the frontend
    *******************/
    server.Get("/active-users", [&userList, &adminUserList] (const httplib::Request &, httplib::Response &response){
        nlohmann::json users = nlohmann::json::array({});
        nlohmann::json response_message;
        
        try {
            if (!userList.getUsers().empty()) {
                for (const auto& user : userList.getUsers()) {
                    users.push_back({
                        {"username", user.getUsername()},
                        {"type", "user"}
                    });
                } 
            }
            if (!adminUserList.getUsers().empty()) {
                for (const auto& admin : adminUserList.getUsers()) {
                    users.push_back({
                        {"username", admin.getUsername()},
                        {"type", "admin"}
                    });
                }
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

        response.status = 200;
        response_message["status"] = "success";
        response_message["users"] = users;
        response.set_content(response_message.dump(), "application/json");
    }); 

    // TODO: Switch between debug and production for localhost and 0.0.0.0 respectively
    server.listen("127.0.0.1", 8080);
}
