#include "server.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <sstream>

using boost::asio::ip::tcp;

long long compute_sum(const std::string& line) {
    std::istringstream iss(line);
    long long val, sum = 0;
    while (iss >> val) sum += val;
    return sum;
}

Session2::Session2(tcp::socket socket, boost::asio::io_context& io)
    : socket_(std::move(socket)), io_(io) {}

void Session2::start() { do_read(); }

void Session2::do_read() {
    auto self = shared_from_this();
    socket_.async_read_some(boost::asio::buffer(data_),
        [this, self](boost::system::error_code ec, std::size_t len) {
            if (!ec) {
                std::string request(data_, len);
                while (!request.empty() && (request.back() == '\n' || request.back() == '\r'))
                    request.pop_back();
                handle_request(request);
            } else if (ec != boost::asio::error::eof) {
                std::cerr << "[Server] Read error: " << ec.message() << "\n";
            }
        });
}

void Session2::handle_request(const std::string& request) {
    auto self = shared_from_this();
    std::cout << "[Server] Received: " << request << "\n";
    boost::asio::post(io_, [this, self, request]() {
        long long sum = compute_sum(request);
        std::string response = "Sum: " + std::to_string(sum) + "\n";
        std::cout << "[Server] Computed sum: " << sum << "\n";
        do_write(response);
    });
}

void Session2::do_write(const std::string& response) {
    auto self = shared_from_this();
    boost::asio::async_write(socket_, boost::asio::buffer(response),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                do_read();
            } else {
                std::cerr << "[Server] Write error: " << ec.message() << "\n";
            }
        });
}

class Acceptor2 : public std::enable_shared_from_this<Acceptor2> {
public:
    Acceptor2(boost::asio::io_context& io, unsigned short port)
        : io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port)) {}

    void start() { do_accept(); }

private:
    void do_accept() {
        auto self = shared_from_this();
        acceptor_.async_accept(
            [this, self](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << "[Server] Client connected: " << socket.remote_endpoint() << "\n";
                    std::make_shared<Session2>(std::move(socket), io_)->start();
                } else {
                    std::cerr << "[Server] Accept error: " << ec.message() << "\n";
                }
                do_accept();
            });
    }

    boost::asio::io_context& io_;
    tcp::acceptor acceptor_;
};

void run_server(boost::asio::io_context& io, unsigned short port) {
    std::make_shared<Acceptor2>(io, port)->start();
    std::cout << "[Server] Listening on port " << port << "\n";
}

int main() {
    try {
        boost::asio::io_context io;
        run_server(io, 12346);
        io.run();
    } catch (std::exception& e) {
        std::cerr << "[Server] Exception: " << e.what() << "\n";
        return 1;
    }
}
