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

    TCP_Connection(TCP_Connection const&) = delete;
    TCP_Connection& operator=(TCP_Connection const&) = delete;
    ~TCP_Connection(void);

    static pointer Create(boost::asio::io_context& io_context);
    [[nodiscard]] tcp::socket& GetSocket(void);
    void Start(void);
private:
    explicit TCP_Connection(boost::asio::io_context& io_context);

    void Disconnect(void);
    void ManageConnection(void);
    void CheckSynchronization(void);
    void DoLogin(void);
    [[nodiscard]] bool CheckLoginCredentials(void);

    const string USERS_PATH = "./Users/";
    ///The socket used for network comunications.
    tcp::socket socketServer;
    ///The data to be sent.
    string outgoingMessage;
    ///The data to be received.
    string incomingMessage;
    ///The user who has logged into this connection.
    string associatedUserID;
    ///The folder associated to the logged user.
    string userFolder;

    uint8_t failedLoginAttempts;
    const uint8_t MAX_NUMBER_OF_FAILED_LOGINS = 2;
};
