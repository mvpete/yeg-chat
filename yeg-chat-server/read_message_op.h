#ifndef __read_message_op_h__
#define __read_message_op_h__

#include <array>
#include "asio.h"
#include "function_codes.h"

namespace yegcpp
{
    template <typename Socket, typename Handler>
    class read_message_op
    {
        enum states { begin, read_username, read_username_complete, send_ack, ack_complete };
        struct impl
        {
            impl(Socket& s, Handler&& h)
                :socket(s), h(std::forward<Handler>(h)), state(states::begin)
            {
            }
            std::array<char, 8> buf;
            std::vector<char> mbuf;
            Socket &socket;
            Handler h;
            states state;

        };
    public:
        read_message_op(Socket& s, Handler&& h, bool confirm = false)
            :impl_(std::make_shared<impl>(s, std::forward<Handler>(h))), confirm_(confirm)
        {

        }

        void operator()(net::error_code ec = {}, size_t bytes = 0)
        {
            if (ec)
            {
                complete(ec, function_codes::error, ec.message());
                return;
            }

            net::tcp::socket& socket = impl_->socket;
            auto& buf = this->buffer();

            switch (impl_->state)
            {
            case states::begin:
            {
                impl_->state = states::read_username;
                net::async_read(socket, boost::asio::buffer(impl_->buf), std::move(*this));
                return;
            }
            case states::read_username:
            {
                if (ec || bytes < 8)
                {
                    const std::string message = ec.message();
                    complete(ec, function_codes::error, message);
                    return;
                }

                // compare the first 4 bytes to YEGC
                if (buf[0] != 'Y' || buf[1] != 'E' || buf[2] != 'G' || buf[3] != 'C')
                {
                    complete(net::error::make_error_code(boost::asio::error::connection_refused), function_codes::error, "");
                    return;
                }

                auto fc = net::ntohs(*reinterpret_cast<unsigned short*>(&buf[4]));
                auto len = net::ntohs(*reinterpret_cast<unsigned short*>(&buf[6]));

                // reserve capacity for the message.
                impl_->mbuf.resize(len);

                if (confirm_)
                    impl_->state = states::send_ack;
                else
                    impl_->state = states::ack_complete;

                auto mbuf = boost::asio::buffer(impl_->mbuf);
                net::async_read(socket, mbuf, std::move(*this));

                return;
            }
            case states::send_ack:
            {
                reinterpret_cast<unsigned short&>(buf[6]) = 0;
                impl_->state = states::ack_complete;
                net::async_write(socket, boost::asio::buffer(impl_->buf), std::move(*this));
                return;
            }
            case states::ack_complete:
                const char* s = impl_->mbuf.data();
                auto size = impl_->mbuf.size();
                auto fcn = net::ntohs(*reinterpret_cast<unsigned short*>(&buf[4]));
                function_codes fc = static_cast<function_codes>(fcn);
                const std::string message(s, s + size);
                complete(net::error_code{}, fc, message);
                return;
            }


        }

        void complete(const net::error_code& ec, function_codes fc, const std::string& message)
        {
            impl_->h(ec, fc, message);
        }

        std::array<char, 8>& buffer()
        {
            return impl_->buf;
        }

    private:
        std::shared_ptr<impl> impl_;
        bool confirm_;
    };

};

#endif // __read_message_op_h__