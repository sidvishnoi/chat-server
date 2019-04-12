#include "./server.h"

void handleMsg(
  const int currentClientFd,
  const ChatroomToFdList &chatRooms,
  const FdToName &clients,
  string msg,
  std::pair <bool, string> &receiverDetail
) {
  auto tokens = split(msg, DELIM, 2);
  // If complete msg is not received in one recv call
  if(tokens.size() != 2) {
    // Personal msg
    if (receiverDetail.first) {
      auto receiverItr = find_if(
        clients.begin(),
        clients.end(),
        [&receiverDetail](auto const &itr) -> bool {
          return itr.second == receiverDetail.second;
        });
      send(receiverItr->first, msg.c_str(), msg.size(), 0);
    }
    else {
      broadcast(chatRooms.at(receiverDetail.second), currentClientFd, msg);
    }
    return;
  }
  // details have chatroom name and personal msg receiver's name
  auto details = tokens[0];
  string receiverName;
  string chatRoomName;
  //If msg is personal
  auto index = details.find('@');
  if(index != string::npos) {
    receiverName = details.substr(index + 1, details.substr(index + 1).find(' '));
    chatRoomName = details.substr(0, index); 
  }
  // Not personal msg
  else {
    receiverName = "";
    chatRoomName = tokens[0];
  }
  auto text = tokens[1];
  if (chatRooms.count(chatRoomName) != 0) {
    if (chatRooms.at(chatRoomName).count(currentClientFd) != 0) {
      auto msgToSend = "MSG" + DELIM
        + clients.at(currentClientFd) + "#" + chatRoomName + DELIM
        + text;
      // send private message.
      if (receiverName != "") {
        auto receiverItr = find_if(
          clients.begin(),
          clients.end(),
          [&receiverName](auto const &itr) -> bool {
            return itr.second == receiverName;
          });
        // Send msg to the receiver if it exist in the chatroom.
        if (receiverItr != clients.end()
          && chatRooms.at(chatRoomName).count(receiverItr->first) != 0) {
          receiverDetail.first = true;
          receiverDetail.second = receiverName;
          send(receiverItr->first, msgToSend.c_str(), msgToSend.size(), 0);
          return;
        }
      }
      // If not private msg, broadcast it to the chatroom.
      receiverDetail.first = false;
      receiverDetail.second = chatRoomName;
      broadcast(chatRooms.at(chatRoomName), currentClientFd, msgToSend);
    }
    else {
      string errorMsg = "ERROR" + DELIM + "SERVER" + DELIM +
        "#" + chatRoomName + " must join chat-room first.";
      send(currentClientFd, errorMsg.c_str(), errorMsg.size(), 0);
    }
  } else {
    string errorMsg = "ERROR" + DELIM + "SERVER" + DELIM +
      "#" + chatRoomName + " doesn't exist.";
    send(currentClientFd, errorMsg.c_str(), errorMsg.size(), 0);
  }
}
