#include "server.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <algorithm>

using boost::asio::ip::tcp;

std::string to_uppercase(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    return s;
}

void run_server(unsigned short port) {
    boost::asio::io_context io;
    tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), port));

    std::cout << "[Server] Listening on port " << port << "\n";

    for (;;) {
        tcp::socket socket(io);
        acceptor.accept(socket);
        std::cout << "[Server] Client connected: " << socket.remote_endpoint() << "\n";

        boost::asio::streambuf buf;
        boost::system::error_code ec;
        boost::asio::read_until(socket, buf, '\n', ec);

        if (ec && ec != boost::asio::error::eof) {
            std::cerr << "[Server] Read error: " << ec.message() << "\n";
            continue;
        }

        std::istream is(&buf);
        std::string line;
        std::getline(is, line);

        std::cout << "[Server] Received: " << line << "\n";
        std::string response = to_uppercase(line) + "\n";

        boost::asio::write(socket, boost::asio::buffer(response), ec);
        if (ec) {
            std::cerr << "[Server] Write error: " << ec.message() << "\n";
        } else {
            std::cout << "[Server] Sent: " << response;
        }
    }
}

int main() {
    try {
        run_server(12345);
    } catch (std::exception& e) {
        std::cerr << "[Server] Exception: " << e.what() << "\n";
        return 1;
    }
}
