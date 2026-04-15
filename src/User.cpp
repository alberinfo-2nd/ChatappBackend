#include <stdexcept>
#include <string>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>

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

User::User(std::string username , std::string public_key, bool is_admin) {
    if(!is_admin) {
        std::fstream admin_users("./admins.cfg");
        if(!admin_users.is_open()) {
            throw std::logic_error("There was an error opening the when looking up the administrators!");
        }

        while(!admin_users.eof()) {
            std::string file_username, file_salt, file_hash;
            admin_users >> file_username >> file_salt >> file_hash;

            if(file_username == username) throw std::invalid_argument("Username belongs to an admin!");
        }
    }

    this->username = username;
    this->publicKey = public_key;
    this->strikeCount = 0;
    
    this->setAuthorizationToken(generateAuthorizationToken());
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

//Returns false if the user can stay in the session
//Returns true if the user has to be kicked
bool User::report(void) const {
    this->strikeCount++;
    return this->strikeCount >= 3;
}

std::vector<std::shared_ptr<Message>> User::getMessages(void) const {
    // Moves ownership of pointer leaves unreadMsesages empty
    return std::move(this->unreadMessages);
}

void User::pushMessage(std::shared_ptr<Message> msg) const {
    //Validating data if want
    this->unreadMessages.push_back(std::move(msg));
}


// ----------------------------------- //

AdminUser::AdminUser(std::string username, std::string password, std::string public_key) : User(username, public_key, true) {
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
            this->password = file_hash;
            return;
        } else {
            throw std::invalid_argument("Invalid password!");
        }
    }

    throw std::invalid_argument("Administrator not found!");
}

//UserList is defined in User.h