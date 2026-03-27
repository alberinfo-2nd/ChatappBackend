#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#include <httplib.h>
#include <json.hpp>
#include <iostream>

/*TODO:
    Make it HTTPS
    Change Post Base

    Add code that allows server to listen to any (website / ip), such as our front ent  
        set_post_routing_handler([](const auto& req, auto& res) {
        res.set_header("Access-Control-Allow-Origin", "*");

Info Section:
    Code to run on cmd
    curl -X POST 127.0.0.1:8080/login -H "Content-Type: application/json" -d "{\"username\":\"Admin\",\"password\":\"123\"}"

});
*/

int main()
{
    httplib::Server server;

    // Base response for GET
    server.Get("/", [](const httplib::Request &, httplib::Response &res)
               { res.set_content("Hello World!\n", "text/plain"); });

    /* Base post for no specific request ---- !Change Later!*/
    server.Post("/", [](const httplib::Request &request, httplib::Response &respose)
                {
        std::string received_data = respose.body;
        std::string str_Response;

        // Print for validating sent message (User_Side --> Server)  
        std::cout << "Received data: " << received_data << "From user with ip: "<< request.remote_addr << std::endl;

        // Print for validating sent message (Server --> User_Side) 
        str_Response = "{\"status\": \"success\", \"received Data\"= \"" + received_data + "\"}";
        respose.set_content(str_Response, "text/plain"); });

    /*****************************
    Sign in requests from Front End
    ******************************/

    server.Post("/login", [](const httplib::Request &request, httplib::Response &response)
                {
        try {
            // Extract info from request using Json 
            auto user_data = nlohmann::json::parse(request.body);
            std::string username = user_data["username"];
            std::string password = user_data["password"];
            std::string status, message;

            // For Admin Sign-in
            if ((username == "Admin") && (password == "123")){ 
                std::cout << "Admin Successfull logged in";
                status = "Success";
                message = "Logged in";
            } 
            // Invalid Sign-in
            else { 
                status = "Failed";
                message = "Not Logged in";   
            }
            // Confirmation Message -> User
            nlohmann::json response_message = {
                {"status:", status},
                {"message", message}
            };
            response.set_content("Status: ", "application/json");
        } 
        catch (const std::exception &e) {// Invalid Request 
            // Print to Error to terminal 
            std::cerr << "JSON Error: " << e.what() << std::endl;

            // Build response Error code
            nlohmann::json error_response({
                {"status", "Failed"},
                {"message", {e.what()}},
                {"user_id", 101}
            });
            //Sent built response
            response.status = 400; 
            response.set_content(error_response.dump(), "application/json");
        } });

    // TODO: Switch between debug and production for localhost and 0.0.0.0 respectively
    server.listen("127.0.0.1", 8080);
}
