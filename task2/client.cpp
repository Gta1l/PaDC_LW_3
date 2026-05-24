#include "client.hpp"
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

void run_client(const std::string& host, unsigned short port) {
    boost::asio::io_context io;
    tcp::resolver resolver(io);
    tcp::socket socket(io);

    boost::asio::connect(socket, resolver.resolve(host, std::to_string(port)));
    std::cout << "[Client] Connected to " << host << ":" << port << "\n";

    std::string numbers;
    std::cout << "[Client] Enter numbers separated by spaces: ";
    std::getline(std::cin, numbers);

    boost::asio::write(socket, boost::asio::buffer(numbers + "\n"));
    std::cout << "[Client] Sent: " << numbers << "\n";

    boost::asio::streambuf buf;
    boost::system::error_code ec;
    boost::asio::read_until(socket, buf, '\n', ec);

    std::istream is(&buf);
    std::string response;
    std::getline(is, response);
    std::cout << "[Client] Server response: " << response << "\n";
}

int main() {
    try {
        run_client("127.0.0.1", 12346);
    } catch (std::exception& e) {
        std::cerr << "[Client] Exception: " << e.what() << "\n";
        return 1;
    }
}
