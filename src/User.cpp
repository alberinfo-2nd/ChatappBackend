#include <memory>
#include <stdexcept>
#include <string>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <filesystem>

#include <sha256.h>
#include <User.h>

#define authorizationTokenLength 32 //32 Characters

//Generates an authorization token for a new user. ONLY USED ON USER CONSTRUCTOR!!
std::string generateAuthorizationToken(void) {
    const std::string characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    
    std::random_device rd; 
    std::mt19937_64 generator(rd()); //
    std::uniform_int_distribution<> distribution(0, characters.size() - 1);

    std::string authorizationToken = "";

    for (size_t i = 0; i < authorizationTokenLength; ++i) {
        authorizationToken += characters[distribution(generator)];
    }

    return authorizationToken;
}

void User::setUsername(std::string username) {
    this->username = username;
}

void User::setPublicKey(std::string publicKey) {
    this->publicKey = publicKey;
}

void User::setAuthorizationToken(std::string token) {
    if(token.length() == 0) throw std::invalid_argument("Authorization token is empty. Try again!");
    this->authorizationToken = token;
}

std::string User::getUsername(void) const {
    return this->username;
}

std::string User::getPublicKey(void) const {
    return this->publicKey;
}

std::string User::getAuthorizationToken(void) const {
    return this->authorizationToken;
}

nlohmann::json User::getMessages() {
    nlohmann::json message_list = nlohmann::json::array({});

    // Get internal message Pointers
    std::vector<std::shared_ptr<Message>> unreadMessagesPTRs = this->unreadMessages;

    // Extract message
    for(std::shared_ptr<Message> messagePTR : unreadMessagesPTRs) {
        message_list.push_back(messagePTR->toJSON());
    }

    this->unreadMessages.clear();

    return message_list;
}

//Returns false if the user can stay in the session
//Returns true if the user has to be kicked
bool User::report() {
    this->strikeCount++;
    return this->strikeCount >= 3;;
}

void User::pushMessage(std::shared_ptr<Message> msg) {
    this->unreadMessages.push_back(std::move(msg));
}

// ----------------------------------- //

AnonymousUser::AnonymousUser(std::string username, std::string public_key) {
    for (auto &p : std::filesystem::recursive_directory_iterator("./")) {
        if(p.path().extension() != ".cfg") continue;

        std::fstream signed_users_file(p.path().string());
        if(!signed_users_file.is_open()) {
            throw std::logic_error("There was an error opening the when looking up the signed users file!");
        }

        while(!signed_users_file.eof()) {
            std::string file_username, file_salt, file_hash;
            signed_users_file >> file_username >> file_salt >> file_hash;

            if(file_username == username) throw std::invalid_argument("Username belongs to a signed user!");
        }
    }

    setUsername(username);
    setPublicKey(public_key);
    this->strikeCount = 0;
    
    setAuthorizationToken(generateAuthorizationToken());
}

void AnonymousUser::sendMessage(User* recipient, std::string message) {
    recipient->pushMessage(std::make_shared<Message>(
        getUsername(),
        message
    ));
}

// ----------------------------------- //

void SignedUser::setPassword(std::string password) {
    this->password = password;
}

AdminUser::AdminUser(std::string username, std::string password, std::string public_key) {
    std::fstream admin_users("./admins.cfg");
    if(!admin_users.is_open()) {
        throw std::logic_error("There was an error opening the when looking up the administrators!");
    }

    //Check file for user
    admin_users.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); //Ignore comment in admins.cfg file
    while(!admin_users.eof()) {
        std::string file_username, file_salt, file_hash;
        admin_users >> file_username >> file_salt >> file_hash;

        if(file_username != username) continue;

        if(sha256(password.append(file_salt)) == file_hash) {
            setPassword(file_hash);
            setUsername(username);
            setPublicKey(public_key);
            this->strikeCount = 0;

            setAuthorizationToken(generateAuthorizationToken());
            return;
        } else {
            throw std::invalid_argument("Invalid password!");
        }
    }

    throw std::invalid_argument("Administrator not found!");
}

//Returns false all the time, admins cannot be kicked
bool AdminUser::report() {
    this->strikeCount++;
    return false;
}

void AdminUser::sendMessage(User* recipient, std::string message) {
    recipient->pushMessage(std::make_shared<Message>(
        getUsername(),
        message,
        getPublicKey()
    ));
}

//UserList is defined in User.h