#include <gtest/gtest.h>

#include "../src/unix_socket_server/unix_socket_server.hpp"

TEST(UnixSocketServer, Init) {
  std::unique_ptr<mathboard::UnixSocketServer> server(
      mathboard::UnixSocketServer::Instance());

  EXPECT_TRUE(server->Init("/tmp/mathboard.sock"));
}

TEST(UnixSocketServer, Read) {
  std::unique_ptr<mathboard::UnixSocketServer> server(
      mathboard::UnixSocketServer::Instance());

  EXPECT_TRUE(server->Init("/tmp/mathboard.sock"));

  server->Listen();

  std::string expected_output = "[TEST](UnixSocketServer, Read)";

  std::string command = "echo '" + expected_output +
                        "' | socat - "
                        "UNIX-CONNECT:/tmp/mathboard.sock";

  system(command.c_str());

  std::int32_t socket_cli_fd;
  void *cli_addr;

  EXPECT_TRUE(server->Accept(socket_cli_fd, &cli_addr));

  std::vector<std::uint8_t> buffer(256);

  EXPECT_TRUE(server->Read(socket_cli_fd, buffer));

  std::string output(buffer.begin(), buffer.begin() + expected_output.size());

  EXPECT_STREQ(output.c_str(), expected_output.c_str());
}

// TODO
// Test write functionality.
// Currently it returns SIGPIPE in write test since the client (socat) exits
// early. This works in normal executable, but not in tests:
/*
  std::unique_ptr<mathboard::UnixSocketServer> server(
      mathboard::UnixSocketServer::Instance());

  server->Init("/tmp/mathboard.sock");

  server->Listen();

  system("echo 'TEST' | socat - "
         "UNIX-CONNECT:/tmp/mathboard.sock");

  std::int32_t socket_cli_fd;
  void* cli_addr{};

  server->Accept(socket_cli_fd, &cli_addr);

  std::vector<std::uint8_t> buffer(256);

  server->Read(socket_cli_fd, buffer);

  std::string output(buffer.begin(), buffer.end());

  std::cout << output << std::endl;

  server->WriteString(socket_cli_fd, "Hello, world!\n");
*/
