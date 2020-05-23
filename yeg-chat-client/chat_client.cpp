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
    // implement broadcast message
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
    // implement receive message
}

void chat_client::process_read_buffer(function_codes fc, size_t bytes)
{
    // implment process buffer.
}