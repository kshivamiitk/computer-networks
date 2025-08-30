# README: Multi-Threaded Chat Server Implementation

## Introduction
This project implements a multi-threaded chat server supporting user authentication, private messaging, group messaging, and broadcast communication. It is developed as part of **CS425: Computer Networks**.

## Features Implemented
- **User Authentication:** Users must log in with a username and password.
- **Private Messaging:** Users can send direct messages to a specific user.
- **Broadcast Messaging:** Users can send messages to all connected clients.
- **Group Messaging:** Users can create, join, and leave groups, and send messages to group members.
- **Multi-threading:** Each client connection runs in a separate thread for concurrent communication.
- **Synchronization:** Mutex locks ensure thread-safe access to shared resources.

## Implementation Details
### 1. User Authentication
When a client connects, the server prompts for a username and password. The authentication details are stored in `names.txt`:
```cpp
void loadUsers(const str &file) 
{
    ifstream auth(file);
    str line;
    while (getline(auth, line)) 
    {
        size_t sep = line.find(':');
        if (sep != str::npos) 
        {
            users[line.substr(0, sep)] = line.substr(sep + 1);
        }
    }
}
```
If the provided credentials match an entry in `names.txt`, access is granted.

### 2. Broadcasting Messages
To send a message to all users except the sender, the server loops through all connected clients:
```cpp
void bd(const str &msg, int sender = -1) 
{
    wheel lock(mtx);
    for (const auto &c : clients) 
    {
        if (c.first != sender)
            sendMsg(c.first, msg);
    }
}
```
A user can send a broadcast message using `/broadcast <message>`.

### 3. Private Messaging
Users can send direct messages using `/msg <username> <message>`. The server finds the recipient and forwards the message:
```cpp
else if (msg.rfind("/msg ", 0) == 0) 
{
    size_t sp = msg.find(' ', 5);
    if (sp != str::npos) 
    {
        str recv = msg.substr(5, sp - 5);
        str text = msg.substr(sp + 1);
        wheel lock(mtx);
        bool found = false;
        for (const auto &c : clients) 
        {
            if (c.second == recv) 
            {
                sendMsg(c.first, "[Private from " + user + "]: " + text + "\n");
                found = true;
                break;
            }
        }
    }
}
```

### 4. Group Messaging
Users can create or join a group and send messages to its members.
```cpp
else if (msg.rfind("/create_group ", 0) == 0) 
{
    str grp = msg.substr(14);
    wheel lock(mtx);
    groups[grp].insert(sock);
    sendMsg(sock, "Created group " + grp + "\n");
}
```
A user can send a group message using `/group_msg <group_name> <message>`.

## How to Compile and Run
### Server Setup
1. **Compile the server:**
   ```sh
   g++ server.cpp -o server -pthread
   ```
2. **Run the server:**
   ```sh
   ./server
   ```

### Client Setup
1. **Compile the client:**
   ```sh
   g++ client_grp.cpp -o client_grp -pthread
   ```
2. **Run the client:**
   ```sh
   ./client_grp
   ```

## Example Interaction
### Terminal 1: Running the Server

```sh
./server
Server listening on port 12345
```

### Terminal 2: Client 1 (Abhishek)

```sh
./client_grp
Connected to the server.
Enter username: Abhishek
Enter password: pass123
Authentication successful. Welcome!
/broadcast Hello everyone!
/msg Shivam Hi Shivam!
/create_group CS425
/group_msg CS425 Welcome to CS425!
```

### Terminal 3: Client 2 (Shivam)

```sh
./client_grp
Connected to the server.
Enter username: Shivam
Enter password: qwerty456
Authentication successful. Welcome!
/join_group CS425
/group_msg CS425 Glad to be here!
```

### Terminal 4: Client 3 (Vishal)

```sh
./client_grp
Connected to the server.
Enter username: Vishal
Enter password: letmein
Authentication successful. Welcome!
/broadcast Let's get started!
```

### Terminal 5: Client 4 (Kawaljeet)
```sh
./client_grp
Connected to the server.
Enter username: Kawaljeet
Enter password: pass789
Authentication successful. Welcome!
/msg Abhishek Hi, ready for the assignment?
```
## WARNING 


If names.txt is changed manually , then one needs to recompile the code.


## Contributors
- **Vishal Kumar(221201)**
- **Kumar Shivam(220562)**
- **Abhishek Kumar(220041)**




