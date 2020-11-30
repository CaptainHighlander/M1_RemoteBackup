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

    //Eexecution...
    try
    {
        //Try to do login.
        this->DoLogin();
        if (this->bIsAuthenticated == false)
            return; //User isn't logged: client will not be able to continue it'execution. So, it's stopped.

        // Init a file watcher checking for the synchronization between client and server
        this->pathToWatch = "./FoldersTest/Riccardo_Client";
        const std::function<void(const std::string&, FileSystemWatcher::FileStatus)> fswActionFunc = std::bind(&Client::NotifyFileChange, this, std::placeholders::_1, std::placeholders::_2);
        FileSystemWatcher fsw { pathToWatch, this->GetDigestsFromServer(), fswActionFunc };
        fsw.StartWatch();

        //TODO - Outgoing files
        do
        {
            //utils::SendFile(this->clientSocket.back(), this->pathToWatch + path);
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
    cout << "[DEBUG] Client::NotifyFileChange --> path = " << path << "\n\tStatus = " << debugStr <<  endl;
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

    //Reset messages
    this->mexToSend = "";
    this->receivedMex = "";
}

std::unordered_map<string,string> Client::GetDigestsFromServer(void)
{
    std::unordered_map<string,string> digestMap;
    this->mexToSend = "ACK_DIGEST";

    //Try to get first digest.
    this->receivedMex = utils::GetDataSynchronously(this->clientSocket.back());
    cout << "[DEBUG] Received message: " << this->receivedMex << endl;

    while (this->receivedMex != "DIGESTS_LIST_EMPTY")
    {
        //Extract the digest and the filename associated with it
        pair<string, string> element = utils::SplitString(this->receivedMex, '\n');
        digestMap[std::move(element.first)] = std::move(element.second);

        //Send ACK
        utils::SendDataSynchronously(this->clientSocket.back(), mexToSend);

        //Try to get an another digest.
        this->receivedMex = utils::GetDataSynchronously(this->clientSocket.back());
        cout << "[DEBUG] Received message: " << this->receivedMex << endl;
    }
    return digestMap;
}
#pragma endregion
