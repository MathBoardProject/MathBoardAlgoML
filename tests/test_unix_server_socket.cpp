#include <gtest/gtest.h>

#include "../src/unix_socket_server.hpp"

TEST(UnixSocketServer, Init) {
  mathboard::UnixSocketServer server{};
  EXPECT_TRUE(server.Init("/tmp/mathboard.sock"));
}

TEST(UnixSocketServer, Read) {
  mathboard::UnixSocketServer server{};
  EXPECT_TRUE(server.Init("/tmp/mathboard.sock"));

  std::int32_t server_socket_fd = server.GetServerSocketFd();

  server.Listen(server_socket_fd);

  std::string expected_output = "[TEST](UnixSocketServer, Read)";

  std::string command = "echo '" + expected_output +
                        "' | socat - "
                        "UNIX-CONNECT:/tmp/mathboard.sock";

  system(command.c_str());

  std::int32_t socket_cli_fd;
  sockaddr_un cli_addr{};

  EXPECT_TRUE(server.Accept(server_socket_fd, socket_cli_fd,
                            reinterpret_cast<sockaddr &>(cli_addr)));

  std::vector<std::uint8_t> buffer(256);

  EXPECT_TRUE(server.Read(socket_cli_fd, buffer));

  std::string output(buffer.begin(), buffer.begin() + expected_output.size());

  EXPECT_STREQ(output.c_str(), expected_output.c_str());
}

// TODO
// Test write functionality.
// Currently it returns SIGPIPE in write test since the client (socat) exits
// early. This works in normal executable, but not in tests:
/*
  mathboard::UnixSocketServer server{};
  server.Init("/tmp/mathboard.sock");

  std::int32_t server_socket_fd = server.GetServerSocketFd();

  server.Listen(server_socket_fd);

  system("echo 'TEST' | socat - "
         "UNIX-CONNECT:/tmp/mathboard.sock");

  std::int32_t socket_cli_fd;
  sockaddr_un cli_addr{};

  server.Accept(server_socket_fd, socket_cli_fd,
                reinterpret_cast<sockaddr &>(cli_addr));

  std::vector<std::uint8_t> buffer(256);

  server.Read(socket_cli_fd, buffer);

  for (char c : buffer) {
    std::cout << c;
  }
  std::cout << std::endl;

  server.WriteString(socket_cli_fd, "Hello, world!\n");
*/
