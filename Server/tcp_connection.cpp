#include "tcp_connection.h"
#include <iostream>
#include <boost/bind.hpp>

#pragma region Static public members:
TCP_Connection::pointer TCP_Connection::Create(boost::asio::io_context& io_context)
{
    return pointer(new TCP_Connection(io_context));
}
#pragma endregion

#pragma region Public members:
tcp::socket& TCP_Connection::GetSocket(void)
{
    return this->socketServer;
}

void TCP_Connection::Start(void)
{
    const string loginRequestMsg = "Please, insert <username>and <password>\n";
    this->m_message = loginRequestMsg;

    // When initiating the asynchronous operation, and if using boost::bind(), we must specify only the arguments
    // that match the handler's parameter list. Currently, in this code, both of the argument placeholders
    // (boost::asio::placeholders::error and boost::asio::placeholders::bytes_transferred)
    // could potentially have been removed, since they are not being used in handle_write().
    // Call boost::asio::async_write() to serve the data to the client.
    // We are using boost::asio::async_write(), rather than ip::tcp::socket::async_write_some(), to ensure that the entire block of data is sent.
    boost::asio::async_write(this->socketServer, boost::asio::buffer(this->m_message),
                             boost::bind(&TCP_Connection::HandleWrite, shared_from_this(),
                             boost::asio::placeholders::error,
                             boost::asio::placeholders::bytes_transferred));
}
#pragma endregion

#pragma region Private members:
TCP_Connection::TCP_Connection(boost::asio::io_context& io_context) : socketServer(io_context) { }

void TCP_Connection::HandleWrite(const boost::system::error_code& error, size_t bytes_transferred)
{
    //TMP
    std::cout << "write completed" << std::endl;
    //this->m_message = "test\n";
    /*
    boost::asio::async_write(this->socketServer, boost::asio::buffer(this->m_message),
                             boost::bind(&TCP_Connection::HandleWrite, shared_from_this(),
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
    */
}
#pragma endregion
