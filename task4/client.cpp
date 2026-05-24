#include "client.hpp"
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

void run_client(const std::string& host, unsigned short port) {
    boost::asio::io_context io;
    tcp::socket socket(io);
    boost::asio::connect(socket, tcp::resolver(io).resolve(host, std::to_string(port)));
    std::cout << "[Client] Connected to " << host << ":" << port << "\n";
    std::cout << "[Client] Send integers to compute factorial. Type 'exit' to quit.\n";

    for (;;) {
        std::string input;
        std::cout << "[Client] Enter number: ";
        if (!std::getline(std::cin, input) || input == "exit") break;

        boost::system::error_code ec;
        boost::asio::write(socket, boost::asio::buffer(input + "\n"), ec);
        if (ec) { std::cerr << "[Client] Write error: " << ec.message() << "\n"; break; }

        boost::asio::streambuf buf;
        boost::asio::read_until(socket, buf, '\n', ec);
        if (ec && ec != boost::asio::error::eof) {
            std::cerr << "[Client] Read error: " << ec.message() << "\n"; break;
        }

        std::istream is(&buf);
        std::string response;
        std::getline(is, response);
        std::cout << "[Client] Server: " << response << "\n";
    }
}

int main() {
    try {
        run_client("127.0.0.1", 12348);
    } catch (std::exception& e) {
        std::cerr << "[Client] Exception: " << e.what() << "\n";
        return 1;
    }
}
