#include <sstream>
#include <string>

#include "chat_client.h"
#include "console.h"


void run(yegcpp::console &con, net::io_context &ctx)
{
    while (true)
    {
        try
        {
            ctx.run();
            return;
        }
        catch (const std::exception &e)
        {
            std::stringstream ss;
            
            ss << "An error occurred - " << e.what() << "\r\n";
            con.write_line(ss.str());
        }
    }
}

void handle_event(yegcpp::console &con, yegcpp::event_types e, const std::string &from, const std::string &message)
{
    switch (e)
    {
    case yegcpp::message:
        std::stringstream ss;
        ss << from << ": " << message;
        con.write_line(ss.str());
        break;
    }
}

void async_read_input(yegcpp::console &con, yegcpp::chat_client &client)
{
    con.async_read_line([&client, &con](const std::string &line) {
        if (line == "quit")
        {
            client.stop();
            return;
        }
        std::stringstream ss;
        ss << "you: " << line;
        con.write_line(ss.str());
        client.async_broadcast_message(line, [&con, &client](const net::error_code &ec) {
            async_read_input(con, client);
        });
    });
}

int main(int argc, const char **argv)
{
    net::io_context ctx;
    yegcpp::console con(ctx, ">");
    con.write_line("<< Welcome to YEG CPP Chat! >>");

    yegcpp::chat_client client(ctx, [&con](yegcpp::event_types e, const std::string &from, const std::string &message) {
        handle_event(con, e, from, message);
    });

    net::tcp::endpoint ep(net::address::from_string(argv[1]), 2020);
    client.async_connect(argv[2], ep, [&client, &con](const net::error_code &ec) {
        con.write_line("Connected to server.");

        async_read_input(con, client);

        client.async_run([&con](const net::error_code &ec) {
            con.write_line("We're done.");
        });
    });


    run(con, ctx);
}