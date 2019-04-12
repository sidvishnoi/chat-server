#include "./client.h"

// if type is set, msg must be plain `msg: user+DELIM+msg`
void printMessage(const std::string &msg) {
  cout << "\r";
  auto tokens = split(msg, DELIM, 3);
  if (tokens.size() != 3) {
    cout << msg << endl;
    return;
  }
  auto &type = tokens[0], &sender = tokens[1], &text = tokens[2];

  if (type == "INFO") cout << color::yellow;
  else if (type == "ERROR") cout << color::red;

  if (type != "MSG") {
    cout << type << "/" << sender << " ➜ " << color::reset;
  } else {
    cout << color::cyan << sender << " ➜ " << color::reset;
  }
  cout << text << endl;
}
