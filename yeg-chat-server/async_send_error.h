#ifndef __async_send_error_h__
#define __async_send_error_h__

namespace yegcpp
{
    template <typename Socket, typename Handler>
    void async_send_error(Socket&& s, const char *message, Handler &&h)
    {
    }


    template <typename Socket, typename Handler>
    void async_send_error_and_disconnect(Socket&& s, const char* message, Handler&& h)
    {
        async_send_error(std::forward<Socket>(s), message, [h=std::forward<Handler>(h)](const net::error_code& ec, net::tcp::socket s)
        {
            s.close();
            h(net::error_code());
        });
    }

};

#endif // __async_send_error_h__