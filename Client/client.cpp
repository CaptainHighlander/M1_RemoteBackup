#include "client.h"
#include <iostream>
#include <boost/asio.hpp>
#include "../Common/Utils/utils.h"

using namespace boost::asio;
using namespace boost::asio::ip;
using std::string;
using std::cout;
using std::cin;
using std::endl;

#pragma region Constructor:
Client::Client(const string& _address, const uint16_t _port)
    : address(_address), port(_port), bIsAuthenticated(false)
{
}
#pragma endregion

#pragma Public members:
void Client::Run(void)
{
    io_service io_service;
    //Socket creation
    this->clientSocket.push_back(ip::tcp::socket(io_service));
    //Try to connect to server
    this->clientSocket.back().connect(tcp::endpoint(address::from_string(this->address), this->port));

    try
    {
        this->DoLogin();
        if (this->bIsAuthenticated == false)
            return; //User isn't logged: client will not be able to continue it'execution. So, it's stopped.

        this->pathToWatch = "./FoldersTest/Riccardo_Client";
        FileSystemWatcher fsw { pathToWatch, this->GetDigestsFromServer() };
        const std::function<void(const std::string&, FileSystemWatcher::FileStatus)> fswActionFunc = std::bind(&Client::NotifyFileChange, this, std::placeholders::_1, std::placeholders::_2);
        fsw.StartWatch(fswActionFunc);
        do
        {
            ;
        }
        while (this->receivedMex != "EXIT" && this->mexToSend != "EXIT");
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception\n\t" << e.what() << endl;
    }
}

void Client::NotifyFileChange(const string& path, const FileSystemWatcher::FileStatus fs)
{
    string debugStr; //TMP string.
    switch(fs)
    {
        case FileSystemWatcher::FileStatus::FS_Created:
            debugStr = "Creato";
            this->filesToCreateList.push_back(path);
            break;
        case FileSystemWatcher::FileStatus::FS_Modified:
            debugStr = "Modificato";
            this->filesToModifyList.push_back(path);
            break;
        case FileSystemWatcher::FileStatus::FS_Erased:
            debugStr = "Cancellato";
            this->filesToDeleteList.push_back(path);
            break;
        default:
            return;
    }
    cout << "[DEBUG] Client::NotifyFileChange --> path = " << path << "\n\tStatus =" << debugStr <<  endl;
    utils::SendFile(this->clientSocket.back(), this->pathToWatch + path);
}
#pragma endregion

#pragma Private members:
void Client::DoLogin(void)
{
    bool bFirstContinuationCondition;
    do
    {
        this->bIsAuthenticated = false;

        //Fetching a mex from the server.
        this->receivedMex = utils::GetDataSynchronously(this->clientSocket.back());
        cout << "[DEBUG] Received message: " << this->receivedMex << endl;

        if (this->receivedMex == "AUTH REQUEST")
            cout << "Insert username and password" << endl;
        else if (this->receivedMex == "AUTH REQUEST RETRY")
            cout << "Incorrect credentials. Please, try again" << endl;
        else if (this->receivedMex == "ACCESS DENIED")
            cout << "Access denied!" << endl;
        else if (this->receivedMex == "AUTHENTICATED")
        {
            cout << "Successful login" << endl;
            this->bIsAuthenticated = true;
        }

        bFirstContinuationCondition = this->receivedMex != "EXIT" && this->receivedMex != "ACCESS DENIED" && this->bIsAuthenticated == false;
        if (bFirstContinuationCondition)
        {
            //Reading new message from input stream
            getline(cin, this->mexToSend);
            //Send mex to server
            utils::SendDataSynchronously(this->clientSocket.back(), this->mexToSend);
        }
        else if (this->bIsAuthenticated == false)
            cout << "Connection terminated" << endl;
    }   while (bFirstContinuationCondition && this->mexToSend != "EXIT");

    //Reset message.
    this->mexToSend = "";
    this->receivedMex = "";
}

std::unordered_map<string,string> Client::GetDigestsFromServer(void)
{
    std::unordered_map<string,string> digestMap;
    this->receivedMex = utils::GetDataSynchronously(this->clientSocket.back());
    while (this->receivedMex != "DIGESTS_LIST_EMPTY")
    {
        pair<string, string> element = utils::SplitString(this->receivedMex, '\n');
        digestMap[std::move(element.first)] = std::move(element.second);
        this->receivedMex = utils::GetDataSynchronously(this->clientSocket.back());
    }
    return digestMap;
}
#pragma endregion
