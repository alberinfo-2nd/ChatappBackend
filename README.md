# ChatApp Backend

Backend for ChatApp project for COP3003 - Spring 2026

---

## Team Members

- Keila Lopez Sosa
- Kolby Goar
- Alberto Cuch
- James Mill

---

## How to Run

### Running pre-built executable
Download latest executable from Releases, and execute from command line. An HTTP server will open up on 127.0.0.1 and listen for connections from any client on port 8080

### Building from source

#### Requirements

- GNU C++ compiler (preferrably through MinGW)
- GNU Make

#### Steps

1. Clone the respository
   `git clone https://github.com/alberinfo-2nd/ChatappBackend`
2. Open an Adminstrative Powershell then install chocolatey with this command
   `Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))`
3. verify if the install was successful by typing
   `choco --version`
4. Install make by running
   `choco install make -y`
5. Build program by running `make all` from the source directory in the project

---

## Features

- Administrator moderation (selected admin users can disconnect an active user from the server)
- Self-destrcuting messaging (A history system which clears automatically upon exit)
- Anonoymous chatting (Users will obtain a temporary identity via chosen username, and be able to talk with end-to-end encryption)
- Strike system (Admin useres will get notified when a certain user has been reported three times and will be able to disconnect them from the system)
- One-to-one private messaging (Users can select another user from the active user list to start a private conversation)

---

## Class Structure

### UML Diagram

- ![UML Diagram](uml.png)

---

## OOP Concepts Used

### Ecapsulation

- Data members like adminPassword, public_key or username are kept private. Access controlled via public getter and setter methods and only available to each object.

### Inheritance

- There is an abstract class User common to all types of users. From it derives AnonymousUser, which displays typical behaviours from normal users. SignedUser also inherits from User, which parents AdminUser and easily could be extended to other types of user such as VipUser. Each subclass shares common data members and also implements functionality unique to itself.

### Polymorphism

- Implemented through virtual and override functions. I.e., sendMessage changes for each final user (AnonymousUser or AdminUser), or report changes behaviour depending on whether an AnonymousUser or AdminUser is being reported.

### Abstraction

- Functions are coded in a way such that their clients do not need to be aware of their implementation. This is represented in main where actions taken on UserList or on Users is a chain of helper calls instead of direct data manipulation.

---

## Acknowledgments

- **[cpp-httplib](https://github.com/yhirose/cpp-httplib)** - Provides core HTTPS functionality

- **[json](https://github.com/nlohmann/json)** - Provides core json functionality

- **[sha256](https://github.com/kibonga/sha256-cpp)** - Provies hash capabilities for the admin passwords

---