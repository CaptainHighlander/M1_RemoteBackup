#pragma once

#include "FileSystemWatcher/file_system_watcher.h"

#include <boost/asio.hpp>
#include <string>

using namespace boost::asio;
using namespace boost::asio::ip;
using std::string;
using std::pair;

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
    void NotifyFileChange(const std::string& path, const FileSystemWatcher::FileStatus);
    #pragma endregion
private:
    string address;
    uint16_t port;
    bool bIsAuthenticated;
    string mexToSend;
    string receivedMex;
    string pathToWatch;
    std::list<ip::tcp::socket> clientSocket;

    #pragma region Private static members:
    [[nodiscard]] static string GetData(tcp::socket& socket);
    static void SendData(tcp::socket& socket, const string& message);
    #pragma endregion

    #pragma region Provate members:
    void DoLogin(tcp::socket& client_socket);
    [[nodiscard]] list<pair<string,string>> GetDigestFromServer(void) const;
    #pragma endregion
};
