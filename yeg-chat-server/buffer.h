#ifndef __buffer_h__
#define __buffer_h__
#include <array>
#include <vector>
#include <string>

#include "asio.h"

using buffer = std::vector<unsigned char>;

void push(buffer& b, unsigned char c)
{
    b.emplace_back(c);
}

void push(buffer& b, unsigned short s)
{
    unsigned short ns = net::htons(s);
    push(b, static_cast<unsigned char>((ns & 0x00FF)));
    push(b, static_cast<unsigned char>((ns & 0xFF00) >> 8));

}

void push(buffer& b, const std::string& s)
{
    unsigned short len = s.length();
    push(b, len);
    for (const char c : s)
        push(b, static_cast<unsigned char>(c));
}



void push(buffer& b, const std::initializer_list<char>& list)
{
    for(const char c : list)
    {
        push(b, static_cast<unsigned char>(c));
    }
}


#endif // __buffer_h__