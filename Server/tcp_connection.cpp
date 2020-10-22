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
TCP_Connection::~TCP_Connection(void)
{
    std::cout << "[DEBUG] Distruzione connessione" << std::endl;
    this->Disconnect();
}

tcp::socket& TCP_Connection::GetSocket(void)
{
    return this->socketServer;
}

void TCP_Connection::Start(void)
{
    const string loginRequestMsg = "AUTH REQUEST\n";
    this->outgoingMessage = loginRequestMsg;

    // When initiating the asynchronous operation, and if using boost::bind(),
    // we must specify only the arguments that match the handler's parameter list.
    // Call boost::asio::async_write() to serve the data to the client.
    // We are using boost::asio::async_write(), rather than ip::tcp::socket::async_write_some(),
    // to ensure that the entire block of data is sent.
    boost::asio::async_write(this->socketServer, boost::asio::buffer(this->outgoingMessage),
                             boost::bind(&TCP_Connection::HandleWrite, shared_from_this(),
                             boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}
#pragma endregion

#pragma region Private members:
TCP_Connection::TCP_Connection(boost::asio::io_context& io_context) : socketServer(io_context), failedLoginAttempts(0)
{
}

void TCP_Connection::Disconnect(void)
{
    this->socketServer.close();
}

void TCP_Connection::HandleWrite(const boost::system::error_code& error, size_t bytes_transferred)
{
    std::cout << "[DEBUG] Write operation completed; transferred " << bytes_transferred << "bytes" << std::endl;
    if (this->associatedUserID.empty() == true && this->failedLoginAttempts < this->MAX_NUMBER_OF_FAILED_LOGINS)
    {
        boost::asio::async_read_until(this->socketServer, boost::asio::dynamic_buffer(incomingMessage), '\n',
                          boost::bind(&TCP_Connection::HandleRead, shared_from_this(),
                                      boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
        return;
    }
}

void TCP_Connection::HandleRead(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (this->associatedUserID.empty() == true)
    {
        // Popping last character "\n"
        this->incomingMessage.pop_back();
    }
    std::cout << this->incomingMessage << std::endl;
    std::cout << "[DEBUG] Read operation completed; transferred " << bytes_transferred << "bytes" << std::endl;
}
#pragma endregion
