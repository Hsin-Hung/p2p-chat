#include "p2p.h"

chat_room room;

awaitable<void> listener(tcp::acceptor acceptor)
{

    for (;;)
    {
        // std::cout << "New Connection" << std::endl;
        std::make_shared<chat_session>(
            co_await acceptor.async_accept(use_awaitable),
            room)
            ->start();
    }
}

void *p2p_server_init(unsigned short port)
{

    try
    {

        boost::asio::io_context io_context(1);

        co_spawn(io_context,
                 listener(tcp::acceptor(io_context, {tcp::v4(), port})),
                 detached);

        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto)
                           { io_context.stop(); });

        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

bool p2p_connect(std::string name, std::string peer, unsigned short port)
{

    try
    {

        boost::asio::io_context io_context;

        std::shared_ptr<socket_handler> socket_hdl = std::make_shared<socket_handler>(tcp::socket(io_context));

        socket_hdl->get_socket().connect(tcp::endpoint(boost::asio::ip::address::from_string(peer.c_str()), port));

        std::make_shared<chat_session>(
            socket_hdl,
            room)
            ->start();
    }
    catch (boost::system::system_error &e)
    {

        std::cerr << "Exception: " << e.what() << "\n";
    }

    return true;
}
bool broadcast(std::string name, std::string msg)
{
    msg = name + ": " + msg + "\n";
    room.deliver(msg);
}
