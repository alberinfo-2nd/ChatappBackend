#pragma once

#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
#include <Message.h>

//ABSTRACT USER CLASS//

class User {
private:
    std::string username;
    std::string publicKey; //Generated with RSA or some ECDSA algo
    std::string authorizationToken; //Generated randomly when the user is created.

protected:
    int strikeCount;
    std::vector<std::shared_ptr<Message>> unreadMessages;

    void setUsername(std::string username);
    void setPublicKey(std::string publicKey);
    void setAuthorizationToken(std::string token);

public:
    std::string getUsername(void) const;
    std::string getPublicKey(void) const;
    std::string getAuthorizationToken(void) const;

    virtual bool report(void);

    nlohmann::json getMessages(void);
    // virtual void pushMessage(std::shared_ptr<Message> msg) const = 0;
    void pushMessage(std::shared_ptr<Message> msg);
    virtual void sendMessage(User* recipient, std::string message) = 0;
};

////////////////////////

//NORMAL ANONYMOUS USER//

class AnonymousUser final : public User {
public:
    AnonymousUser(std::string username, std::string public_key);

    void sendMessage(User* recipient, std::string message) override;
};

////////////////////////

//SIGNED USERS (VIP, ADMINS, ETC)//

class SignedUser : public User {
private:
    std::string password;

protected:
    void setPassword(std::string password);
};

class AdminUser final : public SignedUser {
public:
    AdminUser(std::string username, std::string password, std::string public_key);
    
    bool report(void) override;
    void sendMessage(User* recipient, std::string message) override;
};

////////////////////////

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
            if(x->getUsername() == username) return x;
        }

        return std::nullopt;
    }

    void addUser(T user) {
        try {
            if(searchUser(user->getUsername())) throw std::logic_error("User is already logged in!");
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
                const auto user = *i;
                if (user->getUsername() == username) {
                    this->list.erase(i);
                    return;
                }
            }
        } catch(std::logic_error& e) {
            throw e;
        }
    }

    // used to get the list of currently logged in users
    const std::vector<T>& getUsers() const noexcept {
        return this->list;
    }

    //True if there exists a user with the same username and same authToken. False otherwise
    bool verifyAuthToken(std::string username, std::string authToken) const {
        auto user = searchUser(username);
        if(!user) return false;
        return user->get()->getAuthorizationToken() == authToken;
    }
};