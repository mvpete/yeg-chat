#ifndef __console_h__
#define __console_h__

#include "asio.h"
#include <string>
#include <vector>

#ifdef __unix__
#include <ncurses.h>
#endif


namespace yegcpp
{
    using line_handler = std::function<void(const std::string &)>;

    struct pos
    {
        pos(short x, short y)
            : x(x), y(y)
        {
        }
        short x;
        short y;
    };
    namespace impl
    {
#ifdef _WIN32
        using handle = HANDLE;
#else
        using handle = decltype(stdscr);
#endif
        handle get_stdout();
        handle get_stdin();
        void close_handle(handle);
        void disable_echo(handle);
        pos get_cursor_pos(handle);
        void set_cursor_pos(handle, pos);
        void write_console(handle, const char *buf, size_t len, pos p, unsigned long *written);
        char get_char();
    }

    class console
    {
    public:
        console(net::io_context &ctx, const std::string &prompt);
        ~console();
        void async_read_line(line_handler h);
        void write_line(const std::string &s);
        void write(const std::string &s);

    private:
        void write_rbuf_out();
        void clear_rbuf_out();

    private:
        net::io_context &ctx_;
        const std::string prompt_;

    private:
        impl::handle cout_;
        impl::handle cin_;
        std::vector<char> rbuf_;
        int count_;
        bool awaiting_;
    };
};

#endif // __console_h__
