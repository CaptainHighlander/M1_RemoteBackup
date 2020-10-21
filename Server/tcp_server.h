#pragma once
#include "tcp_connection.h"

class TCP_Server
{
public:
    ///Constructor: initialises an acceptor to listen connections on a TCP port.
    TCP_Server(boost::asio::io_context& io_context, tcp version, uint16_t portNumber);

private:
    ///Creates a socket and initiates an asynchronous accept operation to wait for a new connection.
    void StartAccept(void);

    ///HandleAccept() is called when the asynchronous accept operation initiated by StartAccept() finishes. It services the client request.
    void HandleAccept(const TCP_Connection::pointer& new_connection, const boost::system::error_code& error);

    boost::asio::io_context& io_contextServer;
    tcp::acceptor acceptorServer;
};
