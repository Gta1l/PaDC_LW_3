#include "server.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <sstream>

using boost::asio::ip::tcp;

uint64_t compute_factorial(int n) {
    if (n < 0) return 0;
    uint64_t result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

Session4::Session4(tcp::socket socket, Strand strand,
                   std::vector<std::string>& log,
                   std::atomic<int>& connection_count)
    : socket_(std::move(socket))
    , strand_(std::move(strand))
    , timeout_timer_(socket_.get_executor())
    , log_(log)
    , connection_count_(connection_count)
{
    ++connection_count_;
    std::cout << "[Server] New session. Active connections: " << connection_count_ << "\n";
}

void Session4::start() {
    arm_timeout();
    do_read();
}

void Session4::arm_timeout() {
    auto self = shared_from_this();
    timeout_timer_.expires_after(boost::asio::chrono::seconds(TIMEOUT_SEC));
    timeout_timer_.async_wait(
        boost::asio::bind_executor(strand_,
            [this, self](boost::system::error_code ec) {
                if (!ec) {
                    std::cerr << "[Server] Session timed out, closing.\n";
                    boost::system::error_code ignored;
                    socket_.close(ignored);
                    --connection_count_;
                }
            }));
}

void Session4::cancel_timeout() {
    timeout_timer_.cancel();
}

void Session4::do_read() {
    auto self = shared_from_this();
    socket_.async_read_some(
        boost::asio::buffer(data_),
        boost::asio::bind_executor(strand_,
            [this, self](boost::system::error_code ec, std::size_t len) {
                if (!ec) {
                    cancel_timeout();
                    std::string request(data_, len);
                    while (!request.empty() && (request.back() == '\n' || request.back() == '\r'))
                        request.pop_back();
                    handle_request(request);
                } else {
                    if (ec != boost::asio::error::eof && ec != boost::asio::error::operation_aborted)
                        std::cerr << "[Server] Read error: " << ec.message() << "\n";
                    --connection_count_;
                }
            }));
}

void Session4::handle_request(const std::string& request) {
    auto self = shared_from_this();
    std::cout << "[Server] Thread " << std::this_thread::get_id()
              << " processing: " << request << "\n";

    boost::asio::post(strand_, [this, self, request]() {
        int n = 0;
        try { n = std::stoi(request); } catch (...) {
            do_write("Error: send an integer\n");
            return;
        }

        uint64_t result = compute_factorial(n);
        std::string response = "Factorial(" + std::to_string(n) + ") = "
                               + std::to_string(result) + "\n";

        boost::asio::post(strand_, [this, self, response]() {
            std::ostringstream oss;
            oss << std::this_thread::get_id();
            log_.push_back("[Thread " + oss.str() + "] " + response);
            std::cout << "[Server] Log entries: " << log_.size() << "\n";
        });

        do_write(response);
    });
}

void Session4::do_write(const std::string& response) {
    auto self = shared_from_this();
    boost::asio::async_write(socket_,
        boost::asio::buffer(response),
        boost::asio::bind_executor(strand_,
            [this, self](boost::system::error_code ec, std::size_t) {
                if (!ec) {
                    arm_timeout();
                    do_read();
                } else {
                    if (ec != boost::asio::error::operation_aborted)
                        std::cerr << "[Server] Write error: " << ec.message() << "\n";
                    --connection_count_;
                }
            }));
}

class Acceptor4 : public std::enable_shared_from_this<Acceptor4> {
public:
    Acceptor4(boost::asio::io_context& io, unsigned short port,
              Strand strand, std::vector<std::string>& log,
              std::atomic<int>& connection_count)
        : io_(io)
        , acceptor_(io, tcp::endpoint(tcp::v4(), port))
        , strand_(std::move(strand))
        , log_(log)
        , connection_count_(connection_count) {}

    void start() { do_accept(); }

private:
    void do_accept() {
        auto self = shared_from_this();
        acceptor_.async_accept(
            [this, self](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::cout << "[Server] Accepted from " << socket.remote_endpoint() << "\n";
                    std::make_shared<Session4>(
                        std::move(socket), strand_, log_, connection_count_)->start();
                } else {
                    std::cerr << "[Server] Accept error: " << ec.message() << "\n";
                }
                do_accept();
            });
    }

    boost::asio::io_context& io_;
    tcp::acceptor acceptor_;
    Strand strand_;
    std::vector<std::string>& log_;
    std::atomic<int>& connection_count_;
};

Server4::Server4(boost::asio::io_context& io, unsigned short port, int num_threads)
    : io_(io)
    , log_strand_(io.get_executor())
    , num_threads_(num_threads)
{
    std::make_shared<Acceptor4>(io, port, log_strand_, log_, connection_count_)->start();
    std::cout << "[Server] Started on port " << port
              << " with " << num_threads << " thread(s)\n";
}

void Server4::run() {
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads_; ++i)
        threads.emplace_back([this]() {
            std::cout << "[Server] Worker thread: " << std::this_thread::get_id() << "\n";
            io_.run();
        });

    for (auto& t : threads) t.join();

    std::cout << "\n[Server] Final log (" << log_.size() << " entries):\n";
    for (const auto& entry : log_)
        std::cout << "  " << entry;
}

int main(int argc, char* argv[]) {
    try {
        int num_threads = 4;
        if (argc > 1) {
            try { num_threads = std::stoi(argv[1]); } catch (...) {}
        }

        boost::asio::io_context io;
        Server4 server(io, 12348, num_threads);
        server.run();
    } catch (std::exception& e) {
        std::cerr << "[Server] Exception: " << e.what() << "\n";
        return 1;
    }
}
