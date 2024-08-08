#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

const int BUFFER_SIZE = 1024;

std::vector<std::string> parseHeaderString(std::string &buffer,
                                           char delimiter) {
  std::stringstream ss(buffer);
  std::vector<std::string> linesVec;

  std::string line;
  while (std::getline(ss, line, delimiter)) {
    if (!line.empty()) {
      linesVec.push_back(line);
    }
  }

  return linesVec;
}

int main(int argc, char **argv) {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }
  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  std::cout << "Waiting for a client to connect...\n";

  int client = accept(server_fd, (struct sockaddr *)&client_addr,
                      (socklen_t *)&client_addr_len);
  std::cout << "Client connected\n";

  char buffer[BUFFER_SIZE];
  read(client, buffer, sizeof(buffer));

  std::string bufferStr = buffer;
  std::vector<std::string> headerLines = parseHeaderString(bufferStr, '\r');

  std::vector<std::string> headerRowVec =
      parseHeaderString(headerLines[0], ' ');

  std::string path = headerRowVec[1];

  std::string message;
  if (path == "/") {
    message = "HTTP/1.1 200 OK\r\n\r\n";
  } else if (path.find("/echo/") == 0) {
    std::string echo = path.substr(6);
    message =
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " +
        std::to_string(echo.length()) + "\r\n\r\n" + echo;
  } else {
    message = "HTTP/1.1 404 Not Found\r\n\r\n";
  }

  send(client, message.c_str(), message.length(), 0);

  close(server_fd);

  return 0;
}
