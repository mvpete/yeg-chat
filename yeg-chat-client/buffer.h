#ifndef __buffer_h__
#define __buffer_h__
#include <array>
#include <string>
#include <vector>

#include "asio.h"

namespace yegcpp
{

    using buffer = std::vector<unsigned char>;

    inline void push(buffer &b, unsigned char c)
    {
        b.emplace_back(c);
    }

    inline void push(buffer &b, unsigned short s)
    {
        unsigned short ns = net::htons(s);
        push(b, static_cast<unsigned char>((ns & 0x00FF)));
        push(b, static_cast<unsigned char>((ns & 0xFF00) >> 8));
    }

    inline void push(buffer &b, const std::string &s)
    {
        unsigned short len = s.length();
        push(b, len);
        for (const char c : s)
            push(b, static_cast<unsigned char>(c));
    }


    inline void push(buffer &b, const std::initializer_list<char> &list)
    {
        for (const char c : list)
        {
            push(b, static_cast<unsigned char>(c));
        }
    }

    inline std::string read_string(buffer::iterator &begin, buffer::iterator end)
    {
        // get the len
        auto len = net::ntohs(*reinterpret_cast<unsigned short *>(&*begin));
        if (end - begin < len)
            throw std::runtime_error("bad message");
        auto slen = sizeof(unsigned short);
        std::string s(begin + slen, begin + slen + len);
        begin += slen + len;
        return s;
    }

};

#endif // __buffer_h__