#include <iostream>

#include "chat_server.h"


void run(net::io_context& ctx)
{
    while (true)
    {
        try
        {
            ctx.run();
            return;
        }
        catch (const std::exception & e)
        {
            std::cout << "An error occurred - " << e.what() << "\r\n";
        }
    }
}

void log_event(const yegcpp::chat_server::server_event& e)
{
    switch (e.event)
    {
    case yegcpp::chat_server::events::joined:
        std::cout << e.message << " has joined the chat.\r\n";
        break;
    case yegcpp::chat_server::events::left:
        std::cout << e.message << " has left the chat.\r\n";
        break;
    }
}

int main()
{
    std::cout << "<<< YEG CPP Chat Server >>>" << std::endl;
    net::io_context ctx;
    yegcpp::chat_server server(ctx, log_event);

    server.start(2020);

    run(ctx);
}