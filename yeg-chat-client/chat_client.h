#ifndef __chat_client_h__
#define __chat_client_h__

#include <functional>

#include "asio.h"
#include "buffer.h"
#include "function_codes.h"

namespace yegcpp
{

    using error_handler = std::function<void(net::error_code)>;
    
    enum event_types
    {
        message,
        info
    };

    using event_handler = std::function<void(event_types e, const std::string &, const std::string &)>;

    class chat_client
    {
    public:
        chat_client(net::io_context &ctx, event_handler h);
        void async_connect(const std::string &username, const net::tcp::endpoint &ep, error_handler h);
        void async_broadcast_message(const std::string &message, error_handler handler);
        void async_run(error_handler h);
        void stop();
    private:
        void async_receive_message(error_handler h);
        void async_handshake(const std::string &usernane, error_handler errorhandler);
        void process_read_buffer(function_codes fc, size_t bytes);
    private:
        net::tcp::socket socket_;
        buffer rbuf_;
        buffer wbuf_;
        event_handler handler_;
    };
};

#endif //__chat_client_h__