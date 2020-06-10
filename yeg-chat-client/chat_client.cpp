#include "chat_client.h"

using namespace yegcpp;

chat_client::chat_client(net::io_context &ctx, event_handler h)
    : socket_(ctx), handler_(h)
{
}

void chat_client::async_connect(const std::string &username, const net::tcp::endpoint &ep, error_handler h)
{
    socket_.async_connect(ep, [this, username, h](const net::error_code &ec) {
        if (ec)
        {
            h(ec);
            return;
        }
        async_handshake(username, [h](const net::error_code &ec) {
            h(ec);
        });
    });
}

void chat_client::async_handshake(const std::string &username, error_handler h)
{
    wbuf_.clear();
    // create a buffer w/ the message, and write it.
    push(wbuf_, { 'Y', 'E', 'G', 'C' });
    push(wbuf_, static_cast<unsigned short>(function_codes::hello_world));
    push(wbuf_, username);

    net::async_write(socket_, net::buffer(wbuf_), [this, h](const net::error_code &ec, size_t bytes) {
        if (ec)
        {
            h(ec);
            return;
        }
        // read the shakeback
        rbuf_.resize(8);
        net::async_read(socket_, net::buffer(rbuf_), [this, h](const net::error_code &ec, size_t bytes) {
            h(ec);
        });
    });
}

void chat_client::async_broadcast_message(const std::string &message, error_handler h)
{
    wbuf_.clear();
    push(wbuf_, { 'Y', 'E', 'G', 'C' });
    push(wbuf_, static_cast<unsigned short>(function_codes::broadcast));
    push(wbuf_, message);

    net::async_write(socket_, net::buffer(wbuf_), [h](const net::error_code &ec, size_t bytes) {
        h(ec);
    });
}

void chat_client::async_run(error_handler h)
{
    async_receive_message(h);
}

void chat_client::stop()
{
    socket_.close();
}

void chat_client::async_receive_message(error_handler h)
{
    rbuf_.clear();
    if (rbuf_.size() < 8)
    {
        rbuf_.resize(8);
    }
    net::async_read(socket_, net::buffer(rbuf_), [this, h](const net::error_code &ec, size_t bytes) {
        if (ec || bytes < 8)
        {
            h(ec);
            return;
        }
        if (rbuf_[0] != 'Y' || rbuf_[1] != 'E' || rbuf_[2] != 'G' || rbuf_[3] != 'C')
        {
            h(net::error::make_error_code(net::error::bad_descriptor));
            return;
        }
        auto fc = net::ntohs(*reinterpret_cast<unsigned short *>(&rbuf_[4]));
        auto len = net::ntohs(*reinterpret_cast<unsigned short *>(&rbuf_[6]));

        if (rbuf_.size() < len)
            rbuf_.resize(len);
        net::async_read(socket_, net::buffer(rbuf_), [this, fc, h](const net::error_code &ec, size_t bytes) {
            process_read_buffer(static_cast<yegcpp::function_codes>(fc), bytes);
            async_receive_message(h);
        });
    });
}

void chat_client::process_read_buffer(function_codes fc, size_t bytes)
{
    switch (fc)
    {
    case function_codes::recv_broadcast:
        auto begin = rbuf_.begin();
        auto end = begin + bytes;
        const auto from = read_string(begin, end);
        const auto message = read_string(begin, end);
        handler_(event_types::message, from, message);
        break;
    }
}