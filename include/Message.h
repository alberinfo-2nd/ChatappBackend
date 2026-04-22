#include <string>
#include <chrono> // For handling time
#include <ctime>  // For converting time to readable strings    
#include <json.hpp>

class Message {
private:
    std::string recipientUsername;
    std::string content;
    std::string public_key;
    std::chrono::system_clock::time_point timestamp;

public:
    //Constructors
    Message(std::string recipient, std::string body, std::string public_key = "") 
        : recipientUsername(recipient), content(body), public_key(public_key), timestamp(std::chrono::system_clock::now()){}
    
    void setPublicKey(std::string public_key) {
        this->public_key = public_key;
    }

    // Get Functions
    const std::string& getSender() const{
        return(recipientUsername);
    }
    const std::string& getContent() const {
        return(content);
    }
    const std::string& getPublicKey() const {
        return(public_key);
    }

    // Return String of formatted timestap
    std::string getTimeStamp() const{
        std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
        std::string s = std::ctime(&time);
        if (!s.empty()) s.pop_back();  //removes randomm "\n"
        return s;  
    }

    const nlohmann::json toJSON() const {
        nlohmann::json result = {{"timestamp", getTimeStamp()}, {"sender", getSender()}, {"content", getContent()}};
        if(this->public_key.length() != 0) result.push_back({"public_key", getPublicKey()});
        return result;
    }

};