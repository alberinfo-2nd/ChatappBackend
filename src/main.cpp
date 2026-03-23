#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#include <httplib.h>
//TODO: Make it HTTPS
httplib::Server server;

int main() {
    server.Get("/", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!\n", "text/plain"); //Change to JSON in the future
    });

    //TODO: Switch between debug and production for localhost and 0.0.0.0 respectively
    server.listen("127.0.0.1", 8080);
}
