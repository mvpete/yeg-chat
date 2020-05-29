#include "console.h"
#include <string>

using namespace yegcpp;

#ifdef _WIN32

#include <conio.h>
#include <consoleapi2.h>

#define ENTER '\r'
#define BACKSPACE 0x08

yegcpp::impl::handle yegcpp::impl::get_stdout()
{
   return GetStdHandle(STD_OUTPUT_HANDLE);
}

yegcpp::impl::handle yegcpp::impl::get_stdin()
{
    return GetStdHandle(STD_INPUT_HANDLE);
}

void yegcpp::impl::close_handle(handle h)
{
    CloseHandle(h);
}

void yegcpp::impl::disable_echo(handle h)
{
    DWORD mode;
    GetConsoleMode(h, &mode);

    mode ^= ENABLE_ECHO_INPUT;

    if (!SetConsoleMode(h, mode))
    {
        DWORD err = ::GetLastError();
    }
}

pos yegcpp::impl::get_cursor_pos(handle h)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    GetConsoleScreenBufferInfo(h, &info);
    COORD cpos = info.dwCursorPosition;
    return pos{ cpos.X, cpos.Y };
}

void yegcpp::impl::set_cursor_pos(handle h, pos cpos)
{
    COORD cp{ cpos.x, cpos.y };
    SetConsoleCursorPosition(h, cp);
}

void yegcpp::impl::write_console(handle h, const char *buf, size_t len, pos p, unsigned long *written)
{
    COORD cp{ p.x, p.y };
    WriteConsoleOutputCharacter(h, buf, len, cp, written);
}

char yegcpp::impl::get_char()
{
    return _getch();
}
#else

#include <ncurses.h>

#define ENTER '\n'
#define BACKSPACE 0x7f


yegcpp::impl::handle yegcpp::impl::get_stdout()
{
    initscr();
    return stdscr;
}

yegcpp::impl::handle yegcpp::impl::get_stdin()
{
    // curs_set(0);
    return stdscr;
}

void yegcpp::impl::close_handle(handle h)
{
//   curs_set(1);
  endwin();
}

void yegcpp::impl::disable_echo(handle h)
{
    noecho();
}

pos yegcpp::impl::get_cursor_pos(handle h)
{
    short int x,y;
    getyx(h, y, x);
    return pos{x, y};
}

void yegcpp::impl::set_cursor_pos(handle h, pos cpos)
{
    move(cpos.y, cpos.x);
}

void yegcpp::impl::write_console(handle h, const char *buf, size_t len, pos p, unsigned long *written)
{
    move(p.y, p.x);
    for(int i=0; i<len; ++i) {
      addch(buf[i]);
    }
    addch(' ');
    written += len;
    refresh();
}

char yegcpp::impl::get_char()
{
    return getch();
}
#endif


console::console(net::io_context &ctx, const std::string &prompt)
    : ctx_(ctx), prompt_(prompt), awaiting_(false), count_(0)
{
    cout_ = impl::get_stdout();
    cin_ = impl::get_stdin();
    impl::disable_echo(cin_);
}

console::~console()
{
    impl::close_handle(cin_);
    impl::close_handle(cout_);
}

void console::async_read_line(line_handler h)
{
    net::post([this, h, w = net::io_context::work(ctx_)]() {
        // show the prompt. and read into the buffer.
        count_ = 0;
        awaiting_ = true;
        write_rbuf_out();
        while (true)
        {
            char c = impl::get_char();
            if (c == ENTER)
            {
                clear_rbuf_out();
                break;
            }
            if (c == BACKSPACE)
            {
                if (rbuf_.size() > 0)
                {
                    rbuf_.pop_back();
                    count_--;
                }
            }
            if (c == 0x03)
                exit(0);
            if (std::isprint(c))
            {
                rbuf_.push_back(c);
                ++count_;
            }
            write_rbuf_out();
        }
        std::string s(rbuf_.begin(), rbuf_.begin() + count_);
        rbuf_.clear();
        count_ = 0;
        awaiting_ = false;
        net::post(ctx_, [h, s]() {
            h(s);
        });
    });
}

void console::write_line(const std::string &s)
{
    pos pos = impl::get_cursor_pos(cout_);

    pos.x = 0;
    unsigned long written;
    impl::write_console(cout_, s.c_str(), s.size(), pos, &written);

    pos.x += s.size();
    pos.y += 1;
    pos.x = 0;
    impl::set_cursor_pos(cout_, pos);

    // write the rbuf
    write_rbuf_out();
}

void console::write(const std::string &s)
{
    pos pos = impl::get_cursor_pos(cout_);

    pos.x = 0;
    unsigned long written;
    impl::write_console(cout_, s.c_str(), s.size(), pos, &written);
}

void console::write_rbuf_out()
{
    pos pos = impl::get_cursor_pos(cout_);
    pos.x = 0;
    unsigned long written = 0;

    if (awaiting_)
    {
        impl::write_console(cout_, prompt_.c_str(), prompt_.size(), pos, &written);
        pos.x += written;
        impl::set_cursor_pos(cout_, pos);
    }

    if (count_ == 0)
        return;
    // copy from 0 to x
    const auto rbuf = rbuf_.data();
    impl::write_console(cout_, rbuf, count_, pos, &written);
    pos.x += written;
    impl::set_cursor_pos(cout_, pos);
}

void console::clear_rbuf_out()
{
    if (count_ == 0)
        return;

    pos pos = impl::get_cursor_pos(cout_);

    pos.x = 0;
    // copy from 0 to x
    unsigned long written = 0;
    std::vector<char> cls(count_, ' ');
    impl::write_console(cout_, cls.data(), count_, pos, &written);
}