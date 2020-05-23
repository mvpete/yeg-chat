#ifndef __server_h__
#define __server_h__

#include <optional>
#include <unordered_map>

#include "asio.h"
#include "function_codes.h"


namespace yegcpp
{

    class chat_server
    {
    public:
        enum class events { joined, left };
        struct server_event
        {
            server_event(events e, const std::string& msg)
                :event(e), message(msg)
            {

            }
            const events event;
            const std::string message;
        };
        using event_handler = std::function<void(const server_event&)>;

    public:
        chat_server(net::io_context& context, event_handler h);
        chat_server(const chat_server&) = delete;
        chat_server(chat_server&&) = delete;
        chat_server& operator=(chat_server&) = delete;

        void start(unsigned short port);

    private:

        void async_accept_client();
        void async_process_client(const std::string& username);
        bool dispatch_message(const net::error_code& ec, function_codes fc, const std::string& from, const std::string& msg);
        void async_broadcast_message(const std::string& from, const std::string& message);
        void disconnect_client(const std::string& username);
    private:
        net::tcp::acceptor acceptor_;
        std::unordered_map<std::string, std::optional<net::tcp::socket>> clients_;
        event_handler handler_;
    };
};

#endif // __server_h__