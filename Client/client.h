#pragma once

#include <boost/asio.hpp>
#include <string>

using namespace boost::asio;
using namespace boost::asio::ip;
using std::string;

class Client
{
public:
    #pragma region Constructors:
    Client(const string& _address, const uint16_t _port);
    Client(Client const&) = delete;
    Client& operator=(Client const&) = delete;
    #pragma endregion

    #pragma region Public members:
    void Run(void);
    #pragma endregion
private:
    string address;
    uint16_t port;

    #pragma region Private static members:
    [[nodiscard]] static string GetData(tcp::socket& socket);
    static void SendData(tcp::socket& socket, const string& message);
    #pragma endregion
};
