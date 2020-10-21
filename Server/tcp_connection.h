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

    static pointer Create(boost::asio::io_context& io_context);

    [[nodiscard]] tcp::socket& GetSocket(void);

    void Start(void);

private:
    explicit TCP_Connection(boost::asio::io_context& io_context);

    ///HandleWrite() is responsible for any further actions for this client connection.
    void HandleWrite(const boost::system::error_code& error, size_t bytes_transferred);

    tcp::socket socketServer;
    ///The data to be sent is stored in the class member m_message as we need to keep the data valid until the asynchronous operation is complete.
    string m_message;
};
