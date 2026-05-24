#include "client.hpp"
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

void run_client(const std::string& host, unsigned short port) {
    boost::asio::io_context io;
    tcp::resolver resolver(io);
    tcp::socket socket(io);

    auto endpoints = resolver.resolve(host, std::to_string(port));
    boost::asio::connect(socket, endpoints);
    std::cout << "[Client] Connected to " << host << ":" << port << "\n";

    std::string message;
    std::cout << "[Client] Enter message: ";
    std::getline(std::cin, message);

    boost::asio::write(socket, boost::asio::buffer(message + "\n"));
    std::cout << "[Client] Sent: " << message << "\n";

    boost::asio::streambuf buf;
    boost::system::error_code ec;
    boost::asio::read_until(socket, buf, '\n', ec);

    if (ec && ec != boost::asio::error::eof) {
        std::cerr << "[Client] Read error: " << ec.message() << "\n";
        return;
    }

    std::istream is(&buf);
    std::string response;
    std::getline(is, response);
    std::cout << "[Client] Server response: " << response << "\n";
}

int main() {
    try {
        run_client("127.0.0.1", 12345);
    } catch (std::exception& e) {
        std::cerr << "[Client] Exception: " << e.what() << "\n";
        return 1;
    }
}
