#include "tcp_server.h"

#include <boost/bind.hpp>

TCP_Server::TCP_Server(boost::asio::io_context& io_context, tcp version, uint16_t portNumber)
        : io_contextServer(io_context), acceptorServer(io_context, tcp::endpoint(version, portNumber))
{
    this->StartAccept();
}

void TCP_Server::StartAccept(void)
{
    //Creates a socket to manage this client connection
    TCP_Connection::pointer new_connection = TCP_Connection::Create(this->io_contextServer);

    //Initiates an asynchronous accept operation to wait for a new connection from another client.
    this->acceptorServer.async_accept(new_connection->GetSocket(), boost::bind(&TCP_Server::HandleAccept,
                                      this, new_connection, boost::asio::placeholders::error));
}

void TCP_Server::HandleAccept(const TCP_Connection::pointer& new_connection, const boost::system::error_code& error)
{
    if (!error)
    {
        //Start the connection with the client
        new_connection->Start();
    }

    //Call StartAccept() to initiate the next accept operation.
    this->StartAccept();
}

