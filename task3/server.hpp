#pragma once

#include <boost/asio.hpp>
#include <string>
#include <memory>

class Session3 : public std::enable_shared_from_this<Session3> {
public:
    explicit Session3(boost::asio::ip::tcp::socket socket,
                      boost::asio::io_context& io);
    void start();

private:
    void do_read();
    void handle_request(const std::string& request);
    void do_write(const std::string& response);
    void schedule_pong(int seconds);

    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_context& io_;
    boost::asio::steady_timer timer_;
    char data_[4096]{};
};

void run_server(boost::asio::io_context& io, unsigned short port);
