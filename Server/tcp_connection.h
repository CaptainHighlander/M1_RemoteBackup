#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string.h>

using std::string;
using boost::asio::ip::tcp;

//Using shared_ptr and enable_shared_from_this
//because we want to keep the TCP_Connection object alive
//as long as there is an operation that refers to it.
class TCP_Connection : public boost::enable_shared_from_this<TCP_Connection>
{
public:
    typedef boost::shared_ptr<TCP_Connection> pointer;
    TCP_Connection(const TCP_Connection& other) = delete;
    void operator = (const TCP_Connection& other) = delete;
    ~TCP_Connection(void);

    static pointer Create(boost::asio::io_context& io_context);

    [[nodiscard]] tcp::socket& GetSocket(void);

    void Start(void);
private:
    explicit TCP_Connection(boost::asio::io_context& io_context);

    void Disconnect(void);

    ///HandleWrite() is responsible for any further actions for this client connection.
    void HandleWrite(const boost::system::error_code& error, size_t bytes_transferred);

    ///HandleRead() is responsible for any further actions for this client connection.
    void HandleRead(const boost::system::error_code& error, size_t bytes_transferred);

    tcp::socket socketServer;
    ///The data to be sent is stored in the class member outgoingMessage as we need to keep the data valid until the asynchronous operation is complete.
    string outgoingMessage;
    ///The data to be received is stored in the class member incomingMessage as we need to keep the data valid until the asynchronous operation is complete.
    string incomingMessage;
    ///The user who has logged into this connection.
    string associatedUserID;
    uint8_t failedLoginAttempts;
    const uint8_t MAX_NUMBER_OF_FAILED_LOGINS = 3;
};
