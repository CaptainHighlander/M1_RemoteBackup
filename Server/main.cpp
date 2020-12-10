#include <iostream>
#include <boost/asio.hpp>
#include "tcp_server.h"

int main()
{
    try
    {
        // The io_context object provides I/O services, such as sockets, that the server object will use.
        boost::asio::io_context io_context;

        //Creation of a server object in order to accept incoming IPv4 client connections on port 1996.
        TCP_Server server(io_context, tcp::v4(), 1996);

        //Run the io_context object to perform asynchronous operations. It stops main function execution.
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}