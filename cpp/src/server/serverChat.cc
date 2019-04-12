#include "./server.h"

int serverChat(int sockfd) {
  Database<User> db("./data/users/", "name");
  int maxfd = sockfd;
  char buffer[BUFFER_SIZE];
  FdToName clients;
  ChatroomToFdList chatRooms;
  fd_set master;
  std::pair <bool, string> receiverDetail (false, ""); 

  while (true) {
    FD_ZERO(&master);
    FD_SET(sockfd, &master);
    for (auto &p : clients) {
      FD_SET(p.first, &master);
      if (p.first > maxfd) maxfd = p.first;
    }
    select(maxfd + 1, &master, NULL, NULL, NULL);

    if (FD_ISSET(sockfd, &master)) {
      const int newClient = Accept(sockfd);
      if (newClient < 0) {
        cout << color::red << "[CONNECTION:ERROR] " << color::reset
          << "Cannot Accept from newClient = " << newClient << endl;
        continue;
      }
      cout << color::yellow << "[CONNECTION:INFO] " << color::reset
        << "New client joined from <" << getPeerName(newClient) << ">" << endl;
      FD_SET(newClient, &master);
      clients[newClient] = "";
      continue;
    }

    for (auto &p : clients) {
      const int currentClientFd = p.first;
      if (!FD_ISSET(currentClientFd, &master)) continue;

      std::fill_n(buffer, BUFFER_SIZE, '\0');
      // sizeof(buffer) - 1 to avoid garbage values
      const int bytesRead = recv(currentClientFd, buffer, sizeof(buffer)-1, 0);

      bool connectionLost = (bytesRead <= 0);
      if (connectionLost) {
        handleLostConnection(currentClientFd, chatRooms, clients, master, db);
        continue;
      }

      if (clients[currentClientFd].empty()) {
        addClient(currentClientFd, &master, string(buffer), db, clients);
        continue;
      }

      // process messages
      string msg(buffer);
      auto type = getMessageType(msg);
      if (type == cmd::NOT_CMD) {
        handleMsg(currentClientFd, chatRooms, clients, msg, receiverDetail);
        continue;
      }

      switch (type) {
        case cmd::JOIN: {
          auto chatRoomName = split(msg, DELIM, 2)[1];
          cout << color::green << "[CLIENT:JOIN] " << color::reset
            << "<" << clients[currentClientFd] << "> requested to join #"
            << chatRoomName << endl;
          joinChatRoom(chatRoomName, currentClientFd, clients, chatRooms);
          continue;
        }

        case cmd::LEAVE: {
          auto chatRoomName = split(msg, DELIM, 2)[1];
          cout << color::green << "[CLIENT:LEAVE] " << color::reset
            << "<" << clients[currentClientFd] << "> left #"
            << chatRoomName << endl;
          leaveChatRoom(chatRoomName, currentClientFd, clients, chatRooms);
          continue;
        }

        case cmd::LIST_CHATROOMS: {
          auto response = getChatroomsList(chatRooms);
          send(currentClientFd, response.c_str(), response.size(), 0);
          continue;
        }

        case cmd::LIST_PEOPLE: {
          auto chatRoomName = split(msg, DELIM, 2)[1];
          auto lst = getPeopleList(chatRoomName, clients, chatRooms);
          auto response = "INFO" + DELIM +
            "PEOPLE#" + chatRoomName + DELIM + lst;
          send(currentClientFd, response.c_str(), response.size(), 0);
          continue;
        }

        case cmd::INVALID:
        default: continue;
      }
    }   // end:range-for-clients
  }   // end:while
  return 0;
}
