#pragma once

#include "tcp_connection.h"

class TCP_Server : public boost::enable_shared_from_this<TCP_Connection>
{
public:
    ///Constructor: initialises an acceptor to listen connections on a TCP port.
    TCP_Server(boost::asio::io_context& io_context, tcp version, uint16_t portNumber);
    TCP_Server(TCP_Server const&) = delete;
    TCP_Server& operator=(TCP_Server const&) = delete;
    ~TCP_Server(void);

private:
    ///Creates a socket and initiates an asynchronous accept operation to wait for a new connection.
    void StartAccept(void);

    void SignalsHandler(const boost::system::error_code& errorCode);

    boost::asio::signal_set signalsToIngnore;
    boost::asio::signal_set signals;

    boost::asio::io_context& io_contextServer;
    tcp::acceptor acceptorServer;

    uint64_t nextConnectionId;
};
