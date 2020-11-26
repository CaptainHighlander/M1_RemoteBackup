#pragma once

#include "FileSystemWatcher/file_system_watcher.h"
#include "../Common/SharedList/shared_list.hpp"

#include <boost/asio.hpp>
#include <string>

using namespace boost::asio;
using namespace boost::asio::ip;
using std::string;
using std::pair;
using std::unordered_map;

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
    list<ip::tcp::socket> clientSocket;
    string address;
    uint16_t port;
    bool bIsAuthenticated;
    string mexToSend;
    string receivedMex;
    string pathToWatch;
    //More threads can access to these lists respecting the RAII paradigm.
    SharedList<string> filesToDeleteList;
    SharedList<string> filesToCreateList;
    SharedList<string> filesToModifyList;

    #pragma region Private members:
    void DoLogin(void);
    [[nodiscard]] std::unordered_map<string,string> GetDigestsFromServer(void);
    #pragma endregion
};
