#include <string>
//#include <chrono> // For handling time
//#include <ctime>  // For converting time to readable strings    
#include <json.hpp>

class Message {
private:
    std::string recipientUsername;
    std::string content;
    std::chrono::system_clock::time_point timestamp;

public:
    //Constructors
    //Message() : timestamp(std::chrono::system_clock::now()) {}
    Message(std::string recipient, std::string body) 
        : recipientUsername(recipient), content(body) /*timestamp(std::chrono::system_clock::now()*/{}
    
    /* Returns the raw timestamp
    std::chrono::system_clock::time_point getTimestamp() const {
        return timestamp;
    }*/
    // Get Functions
    const std::string& getSender() const{
        return(recipientUsername);
    }
    const std::string& getContent() const {
        return(content);
    }

    // Return String of formatted timestap
    std::string getTimeStamp() const{
        std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
        return std::ctime(&time);
    }

    const nlohmann::json toJSON() const {
        return {{"sender", getSender()}, {"content", getContent()}};
    }

};