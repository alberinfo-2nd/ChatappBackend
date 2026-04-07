#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

class User {
private:
    std::string username;
    std::string publicKey; //Generated with RSA or some ECDSA algo
    int strikeCount;
    std::string authorizationToken; //Generated randomly when the user is created.

    void setAuthorizationToken(std::string token);

public:
    User(std::string username , std::string public_key);
    std::string getUsername(void) const;
    std::string getPublicKey(void) const;
    std::string getAuthorizationToken(void) const;
    void report(void);
};

class AdminUser : public User {
private:
    std::string password; //SHA245

public:
    AdminUser(std::string username, std::string password, std::string public_key);
};


//Implementation added in User.h because templates are only visible to the current translation unit.
//As such, if the template is separated in definition and implementation in two files, whichever
//code tries to use the function will not see the implementation and will fail at linking
//with an 'undefined reference to x' error. 

template<class T>
class UserList {
private:
    std::vector<T> list;

public:
    UserList() {
        list.clear();
    }

    //returns true if the user already exists, false if it does not.
    std::optional<std::reference_wrapper<const T>> searchUser(std::string username) const {
        for(const auto &x : this->list) {
            if(x.getUsername() == username) return x;
        }

        return std::nullopt;
    }

    void addUser(T user) {
        try {
            if(searchUser(user.getUsername())) throw std::logic_error("User is already logged in!");
            list.push_back(user);
        } catch(std::logic_error& e) {
            throw e;
        }
    }

    // remove user by username
    void removeUser(std::string username) {
        try {
            if(!searchUser(username)) throw std::logic_error("User not found");
            for (auto i = this->list.begin(); i != this->list.end(); ++i ) {
                if (i->getUsername() == username) {
                    this->list.erase(i);
                    return;
                }
            }
        } catch(std::logic_error& e) {
            throw e;
        }
    }

    // used to get the list of currently logged in users
    const std::vector<T>& getUsers() const {
        return this->list;
    }

    //True if there exists a user with the same username and same authToken. False otherwise
    bool verifyAuthToken(std::string username, std::string authToken) const {
        auto user = searchUser(username);
        if(!user) return false;
        return user->get().getAuthorizationToken() == authToken;
    }
};