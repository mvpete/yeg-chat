#include "chat_server.h"

#include <iostream>

#include "async_handshake.h"
#include "async_read_message.h"
#include "async_send_error.h"

#include "buffer.h"

using namespace yegcpp;

chat_server::chat_server(net::io_context &ctx, event_handler handler)
    : acceptor_(ctx), handler_(handler)
{
}

void chat_server::start(unsigned short port)
{
    net::tcp::endpoint ep(net::tcp::v4(), port);

    acceptor_.open(ep.protocol());
    acceptor_.bind(ep);
    acceptor_.listen();

    async_accept_client();
}

void chat_server::async_accept_client()
{
    acceptor_.async_accept([this](const net::error_code &ec, net::tcp::socket s) {
        async_handshake(std::move(s), [this](const net::error_code &ec, net::tcp::socket s, function_codes fc, const std::string &username) {
            if (clients_.find(username) != clients_.end())
            {
                async_send_error_and_disconnect(std::move(s), "Username exists.", [](const net::error_code &ec) {});
                return;
            }
            handler_(server_event{ events::joined, username });
            clients_[username] = std::move(s);
            async_process_client(username);
        });
        async_accept_client();
    });
}

void chat_server::async_process_client(const std::string &username)
{
    net::tcp::socket &s = *clients_[username];
    async_read_message(s, [this, username](const net::error_code &ec, function_codes fc, const std::string &msg) {
        if (ec)
        {
            disconnect_client(username);
            return;
        }

        if (dispatch_message(ec, fc, username, msg))
            async_process_client(username);
    });
}

bool chat_server::dispatch_message(const net::error_code &ec, function_codes fc, const std::string &from, const std::string &msg)
{
    switch (fc)
    {
    case function_codes::broadcast:
        async_broadcast_message(from, msg);
        break;
    }
    return true;
}

void chat_server::async_broadcast_message(const std::string &from, const std::string &message)
{
    auto b = std::make_shared<buffer>();
    push(*b, { 'Y', 'E', 'G', 'C' });
    push(*b, static_cast<unsigned short>(function_codes::recv_broadcast));

    unsigned short len = from.size() + 2 + message.size() + 2;
    push(*b, len);
    push(*b, from);
    push(*b, message);

    // foreach user send message
    for (auto &c : clients_)
    {
        if (c.first != from)
        {
            net::tcp::socket &s = *c.second;
            net::async_write(s, net::buffer(*b), [this, b, username = c.first](const net::error_code &ec, size_t bytes) {
                if (ec)
                    disconnect_client(username);
            });
        }
    }
}

void chat_server::disconnect_client(const std::string &username)
{
    if (clients_.find(username) == clients_.end())
        return;

    auto &client = clients_[username];
    if (!client.has_value())
    {
        clients_.erase(username);
        return;
    }

    auto &socket = *client;
    socket.close();

    clients_.erase(username);
    handler_(server_event{ events::left, username });
}
