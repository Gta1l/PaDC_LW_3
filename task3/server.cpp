#include "server.hpp"
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

Session3::Session3(tcp::socket socket, boost::asio::io_context& io)
    : socket_(std::move(socket)), io_(io), timer_(io) {}

void Session3::start() { do_read(); }

void Session3::do_read() {
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

void Session3::handle_request(const std::string& request) {
    std::cout << "[Server] Received: " << request << "\n";

    int seconds = 3;
    if (request.rfind("ping ", 0) == 0) {
        try { seconds = std::stoi(request.substr(5)); } catch (...) {}
    } else if (request == "ping") {
        seconds = 3;
    } else {
        do_write("Unknown command. Use: ping [N]\n");
        return;
    }

    std::cout << "[Server] Scheduling pong in " << seconds << " seconds\n";
    schedule_pong(seconds);
}

void Session3::schedule_pong(int seconds) {
    auto self = shared_from_this();
    timer_.expires_after(boost::asio::chrono::seconds(seconds));
    timer_.async_wait([this, self, seconds](boost::system::error_code ec) {
        if (!ec) {
            std::string msg = "pong (after " + std::to_string(seconds) + "s)\n";
            std::cout << "[Server] Sending: " << msg;
            do_write(msg);
        }
    });
}

void Session3::do_write(const std::string& response) {
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

class Acceptor3 : public std::enable_shared_from_this<Acceptor3> {
public:
    Acceptor3(boost::asio::io_context& io, unsigned short port)
        : io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port)) {}

    void start() { do_accept(); }

private:
    void do_accept() {
        auto self = shared_from_this();
        acceptor_.async_accept(
            [this, self](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << "[Server] Client connected: " << socket.remote_endpoint() << "\n";
                    std::make_shared<Session3>(std::move(socket), io_)->start();
                }
                do_accept();
            });
    }

    boost::asio::io_context& io_;
    tcp::acceptor acceptor_;
};

void run_server(boost::asio::io_context& io, unsigned short port) {
    std::make_shared<Acceptor3>(io, port)->start();
    std::cout << "[Server] Listening on port " << port << "\n";
}

int main() {
    try {
        boost::asio::io_context io;
        run_server(io, 12347);
        io.run();
    } catch (std::exception& e) {
        std::cerr << "[Server] Exception: " << e.what() << "\n";
        return 1;
    }
}
