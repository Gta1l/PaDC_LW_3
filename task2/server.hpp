#pragma once

#include <boost/asio.hpp>
#include <string>
#include <memory>

class Session2 : public std::enable_shared_from_this<Session2> {
public:
    explicit Session2(boost::asio::ip::tcp::socket socket,
                      boost::asio::io_context& io);
    void start();

private:
    void do_read();
    void handle_request(const std::string& request);
    void do_write(const std::string& response);

    boost::asio::ip::tcp::socket socket_;
    boost::asio::io_context& io_;
    char data_[4096]{};
};

void run_server(boost::asio::io_context& io, unsigned short port);
long long compute_sum(const std::string& line);
