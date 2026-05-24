#include "client.hpp"
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

void run_client(const std::string& host, unsigned short port) {
    boost::asio::io_context io;
    tcp::socket socket(io);
    boost::asio::connect(socket, tcp::resolver(io).resolve(host, std::to_string(port)));
    std::cout << "[Client] Connected. Commands: 'ping' or 'ping N' (N = seconds)\n";

    for (;;) {
        std::string cmd;
        std::cout << "[Client] Enter command: ";
        if (!std::getline(std::cin, cmd) || cmd == "exit") break;

        boost::asio::write(socket, boost::asio::buffer(cmd + "\n"));
        std::cout << "[Client] Sent: " << cmd << "\n";
        std::cout << "[Client] Waiting for server response...\n";

        boost::asio::streambuf buf;
        boost::system::error_code ec;
        boost::asio::read_until(socket, buf, '\n', ec);

        if (ec) { std::cerr << "[Client] Error: " << ec.message() << "\n"; break; }

        std::istream is(&buf);
        std::string response;
        std::getline(is, response);
        std::cout << "[Client] Server: " << response << "\n";
    }
}

int main() {
    try {
        run_client("127.0.0.1", 12347);
    } catch (std::exception& e) {
        std::cerr << "[Client] Exception: " << e.what() << "\n";
        return 1;
    }
}
