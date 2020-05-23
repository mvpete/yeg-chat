#ifndef __async_handshake_h__
#define __async_handshake_h__

#include "read_message_op.h"
#include "function_codes.h"

namespace yegcpp
{
    template <typename Socket, typename Handler>
    struct socket_handler
    {
        socket_handler(Socket&& s, Handler&& h)
            :socket(std::forward<Socket>(s)), handler(std::forward<Handler>(h))
        {
        }

        Socket socket;
        Handler handler;

    };

    template <typename Socket, typename Handler>
    void async_handshake(Socket&& s, Handler&& h)
    {
        auto sh = std::make_unique<socket_handler<Socket,Handler>>(std::forward<Socket>(s), std::forward<Handler>(h));

        auto& socket = sh->socket;
        auto hshandler = [sh = std::move(sh)](const net::error_code ec, function_codes fc, const std::string& message)
        {
            sh->handler(ec, std::move(sh->socket), fc, message);
        };
        read_message_op<Socket, decltype(hshandler)> op(socket, std::move(hshandler), true);
        op();
    }

};

#endif // __async_handshake_h__