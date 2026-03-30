#include <stdexcept>
#include <string>
#include <fstream>
#include <iostream>
#include <limits>

#include <sha256.h>
#include <User.h>

User::User(std::string username , std::string public_key) {
    this->username = username;
    this->publicKey = public_key;
    this->strikeCount = 0;
}

std::string User::getUsername(void) const {
    return this->username;
}

std::string User::getPublicKey(void) const {
    return this->publicKey;
}

void User::report(void) {
    this->strikeCount++;

    if(this->strikeCount >= 3) {
        //TODO: Kick and send message!
    }
}

// ----------------------------------- //

AdminUser::AdminUser(std::string username, std::string password, std::string public_key) : User(username, public_key) {
    std::fstream admin_users("../admins.cfg");
    if(!admin_users.is_open()) {
        throw "There was an error opening the when looking up the administrators!";
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