#pragma once

#include "../Common/Utils/utils.h"

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string.h>
#include <unordered_map>

using std::string;
using std::unordered_map;
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

    #pragma region Static members:
    static pointer Create(tcp::socket io_context, const uint64_t id);
    [[nodiscard]] static unordered_map<uint64_t, pointer> GetConnectionsMap(void);
    [[nodiscard]] static size_t GetActiveConnectionsNumber(void);
    #pragma endregion

    #pragma region Public members:
    void Start(void);
    void Close(void);
    #pragma endregion
private:
    TCP_Connection(tcp::socket io_context, const uint64_t _id);

    void Disconnect(void);
    void ManageConnection(void);
    void DoLogin(void);
    [[nodiscard]] bool CheckLoginCredentials(void);
    void CheckSynchronization(void);
    void ManageCommunicationWithClient(void);

    const string USERS_PATH = "./Users/";
    ///The connection id
    uint64_t id;
    ///The socket used for network comunications.
    tcp::socket socketServer;
    ///A buffer of byte
    std::array<char, BUFFER_SIZE> byteBuffer{};
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

    static unordered_map<uint64_t, pointer> connectionsMap;
    static std::mutex m_connectionsMap;
};
