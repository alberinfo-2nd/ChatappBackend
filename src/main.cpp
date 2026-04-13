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

    /********************************************** 
    Todo ---- Authenticates users and validates administrative privileges 
    Intercepts all API requests.
    **********************************************/
    server.set_pre_request_handler([&userList, &adminUserList](const httplib::Request &request, httplib::Response& response) {
        auto user_data = nlohmann::json{};
        nlohmann::json response_message = {
            {"status", "Success"},
            {"message", "User created successfully"}
        };

        try {
            // Check if contains data
            if(request.body.size() == 0 ) {
                throw std::invalid_argument("Body has size zero!");
            }
            
            // Extract info from request using Json 
            user_data = nlohmann::json::parse(request.body);

            //Verify that required values are present
            if (!user_data.contains("username")) {
                throw std::invalid_argument("username field was not provided!");
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


        // Test for login
        if(request.path == "/login") return httplib::Server::HandlerResponse::Unhandled;
            
        std::string username = user_data["username"];
        std::string authToken = request.get_header_value("authorizationToken");
        if(userList.verifyAuthToken(username, authToken) || adminUserList.verifyAuthToken(username, authToken)) {
            return httplib::Server::HandlerResponse::Unhandled; //Continue to normal routing
        }

        // Nothing works send Error code
        response.status = 403;
        response_message["status"] = "Failed";
        response_message["message"] = "Invalid authorization token or username";

        response.set_content(response_message.dump(), "application/json");
        return httplib::Server::HandlerResponse::Handled;  // User was not authorized!
    });

    // Base response for GET
    server.Get("/", [](const httplib::Request &, httplib::Response &res){
        res.set_content("Hello World!\n", "text/plain");
    });

    /*****************************
    Login in / out requests from Front End
    ******************************/
    server.Post("/login", [&userList, &adminUserList](const httplib::Request &request, httplib::Response &response){
        response.status = 200;
        // Format for response message
        nlohmann::json response_message = {
            {"status", "Success"},
            {"message", "User created successfully"}
        };
        
        // Extract info from request using Json + check for requrments
        auto user_data = nlohmann::json::parse(request.body);
        try {
            if (!user_data.contains("public_key")) {
                throw std::invalid_argument("public_key field was not provided!");
            }

            if(!user_data.contains("password")) { 
                // If Normal user
                User newUser = User(user_data["username"], user_data["public_key"]);
                userList.addUser(newUser);
                response_message["authorization_token"] = newUser.getAuthorizationToken();
            } else {
                // If Admin user
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
        // Set response 
        response.set_content(response_message.dump(), "application/json");
    });

    server.Post("/logout", [&userList, &adminUserList](const httplib::Request &request, httplib::Response &response) {
        response.status = 200;
        // Format for response message
        nlohmann::json response_message = {
            {"status", "Success"},
            {"message", "User deleted successfully"}
        };
        
        // Extract info from request using Json + check for requrments
        auto user_data = nlohmann::json::parse(request.body);
        try {
            userList.removeUser(user_data["username"]);
        } catch(std::logic_error& e) {
            try {
                adminUserList.removeUser(user_data["username"]);
            } catch(std::logic_error& e) {
                response.status = 400;
                response_message["status"] = "Failed";
                response_message["message"] = e.what();
            }
        }

        response.set_content(response_message.dump(), "application/json");
    });


    /*****************
    get request to send array of active users to the frontend
    *******************/
    server.Get("/active-users", [&userList] (const httplib::Request &, httplib::Response &response){
        nlohmann::json users = nlohmann::json::array({});
        nlohmann::json response_message;
        
        for (const auto& user : userList.getUsers()) {
            users.push_back({
                {"username", user.getUsername()},
                {"public_key", user.getPublicKey()}    
            });
        }    
        response.status = 200;
        response_message["status"] = "success";
        response_message["users"] = users;
        response.set_content(response_message.dump(), "application/json");
    }); 

    /* 
    Send Message from User to User
    */
    server.Post("/sendMessage", [&userList, &adminUserList](const httplib::Request &request, httplib::Response &response){
        response.status = 200;
        nlohmann::json response_message = {{"status", "Success"}, {"message", "message sent!"}};
        // Extract info from request using Json + check for requirments
        nlohmann::json user_data{};
        try{
            auto user_data = nlohmann::json::parse(request.body);
            if (!user_data.contains("message") )    throw std::invalid_argument("message field was not provided!");
            if (!user_data.contains("recipient"))   throw std::invalid_argument("recipient field was not provided!");
            if (!user_data["message"].is_string())  throw std::invalid_argument("message was not a string");
            if (!user_data["recipient"].is_string())  throw std::invalid_argument("recipient was not a string");
 
            if(user_data["username"] == user_data["recipient"]) throw std::invalid_argument("User cannot send messages to itself!");

            //*** Finding user to message ***/            
            // Check on userList then admin list
            auto userSearchResult = userList.searchUser(user_data["recipient"]);
            if (!userSearchResult) userSearchResult = adminUserList.searchUser(user_data["recipient"]);
            if (!userSearchResult) throw std::invalid_argument("Recipient Not Found");

            // Pushes message to corrisponding user 
            userSearchResult->get().pushMessage(std::make_shared<Message>(
                user_data["username"], // Sender
                user_data["message"]   // Message
            ));
        } catch (std::invalid_argument &e){
            response.status = 400;
            response_message["status"] = "Failed";
            response_message["message"] = e.what();
        } catch(std::logic_error& e) {
            response.status = 400;
            response_message["status"] = "Failed";
            response_message["message"] = e.what();
        }
        
        response.set_content(response_message.dump(), "application/json");
    });

    
    /* 
    Read Message from User
    */
    server.Get("/readMessages", [&userList, &adminUserList](const httplib::Request &request, httplib::Response &response) {
        response.status = 200;
        
        nlohmann::json response_message = {
            {"status", "Success"},
            {"messages", nlohmann::json::array({})}
        };

        nlohmann::json message_list{};
        try {
            // Extract info from request using Json + check for requrments        
            auto user_data = nlohmann::json::parse(request.body);

            // Check user list --- Then admin
            std::optional<std::reference_wrapper<const User>> userSearchResult = userList.searchUser(user_data["username"]);
            if (!userSearchResult) userSearchResult = adminUserList.searchUser(user_data["username"]);

            // Get internal message Pointers
            std::vector<std::shared_ptr<Message>> unreadMessagesPTRs = userSearchResult->get().getMessages();

            // Extract message
            for(std::shared_ptr<Message> messagePTR : unreadMessagesPTRs) {
                message_list.push_back(messagePTR->toJSON());
            }

            response_message["messages"] = message_list;
        } catch(std::invalid_argument &e){
            response.status = 400;
            response_message["status"] = "Failed";
            response_message["message"] = e.what();
        }

        response.set_content(response_message.dump(), "application/json");
    });

    server.Post("/report", [&userList, &adminUserList](const httplib::Request &request, httplib::Response &response) {
        response.status = 200;
        
        nlohmann::json response_message = {
            {"status", "Success"},
            {"message", "User reported successfully"}
        };

        try {
            auto user_data = nlohmann::json::parse(request.body);

            if(!user_data.contains("reportedUser")) throw std::invalid_argument("reportedUser field was not provided!");

            if(user_data["username"] == user_data["reportedUser"]) throw std::invalid_argument("user cannot report itself!");

            auto reportedUser = userList.searchUser(user_data["reportedUser"]);
            if(!reportedUser) reportedUser = adminUserList.searchUser(user_data["reportedUser"]);
            if(!reportedUser) throw std::invalid_argument("Reported username does not exist!!");

            bool kickUser = reportedUser->get().report();
            if(kickUser) {
                try {
                    userList.removeUser(user_data["reportedUser"]);
                } catch(...) {
                    adminUserList.removeUser(user_data["reportedUser"]);
                }
            }
        } catch(std::invalid_argument& e) {
            response.status = 400;
            response_message["status"] = "Failed";
            response_message["message"] = e.what();
        }

        response.set_content(response_message.dump(), "application/json");
    });

    server.Post("/kick", [&userList](const httplib::Request &request, httplib::Response &response) {
        response.status = 200;
        
        nlohmann::json response_message = {
            {"status", "Success"},
            {"message", "User kicked successfully"}
        };
        
        try {
            auto user_data = nlohmann::json::parse(request.body);

            //Since we know username is in either userList or adminUserList, it showing up on userList means its not an admin user.
            if(userList.searchUser(user_data["username"])) throw std::invalid_argument("User kicking has to be an admin_user");
            if(!user_data.contains("kickedUser")) throw std::invalid_argument("kickedUser field was not provided!");
            if(user_data["username"] == user_data["kickedUser"]) throw std::invalid_argument("user cannot kick itself!");

            try {
                userList.removeUser(user_data["kickedUser"]);
            } catch(std::logic_error& e) {
                response.status = 400;
                response_message["status"] = "Failed";
                response_message["message"] = e.what();
            }
        } catch(std::invalid_argument& e) {
            response.status = 400;
            response_message["status"] = "Failed";
            response_message["message"] = e.what();
        }

        response.set_content(response_message.dump(), "application/json");
    });

    // TODO: Switch between debug and production for localhost and 0.0.0.0 respectively
    server.listen("127.0.0.1", 8080);
}
