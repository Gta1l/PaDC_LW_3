#pragma once

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <memory>
#include <atomic>

using Strand = boost::asio::strand<boost::asio::io_context::executor_type>;

class Session4 : public std::enable_shared_from_this<Session4> {
public:
    Session4(boost::asio::ip::tcp::socket socket,
             Strand strand,
             std::vector<std::string>& log,
             std::atomic<int>& connection_count);
    void start();

private:
    void do_read();
    void handle_request(const std::string& request);
    void do_write(const std::string& response);
    void arm_timeout();
    void cancel_timeout();

    boost::asio::ip::tcp::socket socket_;
    Strand strand_;
    boost::asio::steady_timer timeout_timer_;
    std::vector<std::string>& log_;
    std::atomic<int>& connection_count_;
    char data_[4096]{};
    static constexpr int TIMEOUT_SEC = 30;
};

class Server4 {
public:
    Server4(boost::asio::io_context& io, unsigned short port, int num_threads);
    void run();

private:
    boost::asio::io_context& io_;
    Strand log_strand_;
    std::vector<std::string> log_;
    std::atomic<int> connection_count_{0};
    int num_threads_;
};

uint64_t compute_factorial(int n);
