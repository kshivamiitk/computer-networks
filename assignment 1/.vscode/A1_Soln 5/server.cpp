#include <bits/stdc++.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 12345
#define BUF 1024
#define str string
using namespace std;

// using wheel instead of lock_guard<mutex> 
#define wheel lock_guard<mutex>

// the client list 
unordered_map<int, str> clients;
// the users lists, who can add the user
unordered_map<str, str> users;
// the list of the groups
unordered_map<str, unordered_set<int>> groups;
mutex mtx;

// the function to load the users is implemented here
void loadUsers(const str &file)
{
    ifstream auth(file);
    if (!auth)
    {
        //if the file is not getting opened or the name is wrong it will lead to error
        cerr << "Error: Unable to open " << file << " file." << endl;
        exit(1);
    }
    str line;
    while (getline(auth, line))
    {
        size_t sep = line.find(':');
        if (sep != str::npos)
        {
            users[line.substr(0, sep)] = line.substr(sep + 1);
        }
    }
    auth.close();
    // This is used for the loading of the users, like I have defined a file named users.txt
}
// An explicit messaging function in order to avoid the excessive redefinition
void sendMsg(int sock, const str &msg)
{
    send(sock, msg.c_str(), msg.size(), 0);
}
// function for broadcasting the message to all of the users
void bd(const str &msg, int sender = -1)
{
    wheel lock(mtx);
    for (const auto &c : clients)
    {
        if (c.first != sender)
            sendMsg(c.first, msg);
    }
}

// making the function handleclient to include in the threading part
void handleClient(int sock)
{
    char buf[BUF];
    str user, pass;
    // taking the authentication, allowing access if the name and password matches

    sendMsg(sock, "Enter username: ");

    fill(buf, buf + BUF, 0);

    if (recv(sock, buf, BUF - 1, 0) <= 0)
        return;

    
    user = str(buf);
    user.erase(remove(user.begin(), user.end(), '\n'), user.end());

    sendMsg(sock, "Enter password: ");

    fill(buf, buf + BUF, 0);

    if (recv(sock, buf, BUF - 1, 0) <= 0)
        return;

    pass = str(buf);

    pass.erase(remove(pass.begin(), pass.end(), '\n'), pass.end());

    if (users.count(user) && users[user] == pass)
    { sendMsg(sock, "Authentication successful. Welcome!\n");}
    else
    {
        sendMsg(sock, "Authentication failed\n");
        close(sock);
        return;
    }
    {
        wheel lock(mtx);
        // locking the system when adding a new user, if authentication successful
        clients[sock] = user;
    }

    bd("[" + user + "] joined.\n", sock);
    cout << "[" << user << "] joined.\n";

    // here comes the part where the user communciate with other users
    while (true)
    {
        fill(buf, buf + BUF, 0);
        int bytes = recv(sock, buf, BUF - 1, 0);
        if (bytes <= 0)
            break;

        str msg(buf);
        msg.erase(remove(msg.begin(), msg.end(), '\n'), msg.end());

        if (msg == "/exit")
            break;
        else if (msg.find("/msg ", 0) == 0)
        {
                // mssging the person mentioned

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
                if (!found)
                    sendMsg(sock, "User not found.\n");
            }
            else
            {
                sendMsg(sock, "Invalid command. Use: /msg <user> <msg>\n");
            }
        }
        else if (msg.find("/broadcast ", 0) == 0)
        {
            // broadcasting case
            bd("[Broadcast from " + user + "]: " + msg.substr(11) + "\n", sock);
        }
        else if (msg.find("/join_group ", 0) == 0)
        {
            // joining the group
            str grp = msg.substr(12);
            wheel lock(mtx);
            groups[grp].insert(sock);
            sendMsg(sock, "Joined group " + grp + "\n");
        }
        else if (msg.find("/create_group ", 0) == 0)
        {
            // creating the group 
            str grp = msg.substr(14);
            wheel lock(mtx);
            groups[grp].insert(sock);
            sendMsg(sock, "Created group " + grp + "\n");
        }
        else if (msg.find("/leave_group ", 0) == 0)
        {
            // leaving the group
            str grp = msg.substr(13);
            wheel lock(mtx);
            if (groups.count(grp))
            {
                groups[grp].erase(sock);
                sendMsg(sock, "Left group " + grp + "\n");
            }
            else
                sendMsg(sock, "Group not found.\n");
        }
        else if (msg.find("/group_msg ", 0) == 0)
        {
            // messaging the group 
            size_t sp = msg.find(' ', 11);
            if (sp != str::npos)
            {
                str grp = msg.substr(11, sp - 11);
                str text = msg.substr(sp + 1);
                wheel lock(mtx);
                if (groups.count(grp))
                {
                    for (int s : groups[grp])
                    {
                        if (s != sock)
                            sendMsg(s, "[Group " + grp + " from " + user + "]: " + text + "\n");
                    }
                }
                else
                    sendMsg(sock, "Group not found.\n");
            }
            else
                sendMsg(sock, "Invalid command. Use: /group_msg <group> <msg>\n");
        }
        else
            sendMsg(sock, "Unknown command.\n");
    }

    {
        wheel lock(mtx);
        clients.erase(sock);
    }
    bd("[" + user + "] left.\n", sock);
    cout << "[" << user << "] left.\n";
    close(sock);
}
int main()
{
    int server, newSock;
    struct sockaddr_in addr;
    int opt = 1;
    socklen_t addrLen = sizeof(addr);
    // loading the users, need to recompile the code if the names.txt is updated manually
    loadUsers("names.txt");
    if ((server = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        return -1;
    }

    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Setsockopt failed");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    // binding the server

    if (::bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Bind failed");
        return -1;
    }

    // listening of the server started
    if (listen(server, 5) < 0)
    {
        perror("Listen failed");
        return -1;
    }

    cout << "Server listening on port " << PORT << endl;
    // here the while loop will help in interaction of the users
    while (true)
    {
        newSock = accept(server, (struct sockaddr *)&addr, &addrLen);
        if (newSock < 0)
        {
            perror("Accept failed");
            continue;
        }
        cout << "New client connected" << endl;
        // thread to handle the client as per the defined functions above, and the server is not going to stop until ctr+C is not pressed
        
        thread(handleClient, newSock).detach();
    }

    close(server);
    return 0;
}