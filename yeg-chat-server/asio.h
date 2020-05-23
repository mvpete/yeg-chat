#ifndef __asio_h__
#define __asio_h__
#define WINDOWS 1
#ifdef WINDOWS
#include <WinSock2.h>
#endif

#include <boost/asio.hpp>
namespace net
{
    using namespace boost::asio;
    using namespace boost::asio::ip;
    using io_context = boost::asio::io_context;
    using error_code = boost::system::error_code;
    

    inline unsigned short htons(unsigned short hs)
    {
        return ::htons(hs);
    }

    inline unsigned short ntohs(unsigned short ns)
    {
        return ::ntohs(ns);
    }
}
#endif
