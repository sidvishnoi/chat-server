#ifndef SRC_SERVER_SERVER_H_
#define SRC_SERVER_SERVER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <iostream>
#include <stdexcept>
#include <cerrno>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include "../db/database.h"
#include "../db/User.h"

using std::cout; using std::cin; using std::endl;
using std::string;
using std::vector;
using FdList = std::set<int>;
using FdToName = std::map<int, string>;
using ChatroomToFdList = std::map<string, FdList>;

namespace cmd {
  enum Commands {
    NOT_CMD,
    INVALID,
    JOIN,
    LEAVE,
    LIST_CHATROOMS,
    LIST_PEOPLE
  };
}

namespace color {
  const std::string red("\033[0;31m");
  const std::string green("\033[1;32m");
  const std::string yellow("\033[1;33m");
  const std::string cyan("\033[0;36m");
  const std::string magenta("\033[0;35m");
  const std::string reset("\033[0m");
}

int Socket();
void Bind(int sockfd, int port);
void Listen(int sockfd, int backlog);
int Accept(int sockfd);

// print the errno codes
void printError();

// login a user
string login(const Database<User> &db, const string &uname, const string &pass);

// logout a user
bool logout(const Database<User> &db, const string &uname);

// main chat function
int serverChat(int sockfd);

// add a client to chat system (with login)
bool addClient(
  const int currentClientFd,
  fd_set *master,
  const std::string &credentials,
  const Database<User> &db,
  FdToName &names
);

// get the msg type - regular msg or some command
cmd::Commands getMessageType(const std::string &);

// response for `/list rooms`
std::string getChatroomsList(const ChatroomToFdList &chatRooms);

// response for `/list people`
std::string getPeopleList(
  const std::string &chatRoomName,
  const FdToName &names,
  const ChatroomToFdList &chatRooms
);

// broadcast msg to all `clients` except currentClientFd (sender)
void broadcast(
  const FdList &clients,
  const int currentClientFd,
  const string &msg
);

// handle private messages
void privateChat(
  const FdToName &clients,
  int currentClientFd,
  const string &msg,
  string clientName
);

// add a user to a chatroom `/join #chatroom`
void joinChatRoom(
  const std::string chatRoomName,
  const int clientFd,
  FdToName &clients,
  ChatroomToFdList &chatRooms
);

// remove a user from a chatroom `/leave #chatroom`
void leaveChatRoom(
  const std::string chatRoomName,
  const int clientFd,
  const FdToName &names,
  ChatroomToFdList &chatRooms
);

// handle lost connection request
// - Remove connection from active connections
// - Notify chatroom mates.
void handleLostConnection(
  const int currentClientFd,
  ChatroomToFdList &chatRooms,
  FdToName &clients,
  fd_set &master,
  Database<User> &db
);

// handle incomming msg :
//  if private send it to the particular client else broadcast it to chatroom.
void handleMsg(
  const int currentClientFd,
  const ChatroomToFdList &chatRooms,
  const FdToName &clients,
  string msg,
  std::pair <bool, string> &receiverDetails
);

// get the `ip:port` for a given client
std::string getPeerName(const int sockfd);

const int BUFFER_SIZE = 1024;
const string DELIM("$$$");

vector<string> split(const string &str, const string &delim, const int limit);

#endif  // SRC_SERVER_SERVER_H_
