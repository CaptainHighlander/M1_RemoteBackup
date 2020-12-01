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

    //Execution...
    try
    {
        //Try to do login.
        this->DoLogin();
        if (this->bIsAuthenticated == false)
            return; //User isn't logged: client will not be able to continue it'execution. So, it's stopped.

        //Init a file watcher checking for the synchronization between client and server
        this->pathToWatch = "./FoldersTest/Riccardo_Client";
        const FileSystemWatcher::notificationFunc fswActionFunc = std::bind(&Client::NotifyFileChange, this, std::placeholders::_1, std::placeholders::_2);
        FileSystemWatcher fsw { pathToWatch, this->GetDigestsFromServer(), fswActionFunc };
        fsw.StartWatch();

        //TODO - Outgoing files
        do
        {
            //Communicates the paths to be deleted
            while (this->filesToDeleteSet.IsEmpty() == false)
            {
                //Extract the next element (it will be removed from the set of file to delete).
                std::optional<string> pathToDelete = this->filesToDeleteSet.Extract();
                if (pathToDelete.has_value() == true)
                {
                    this->mexToSend = "RM " + pathToDelete.value();
                    //std::cout << "[DEBUG] Sending " << this->mexToSend << std::endl;
                    utils::SendDataSynchronously(this->clientSocket.back(), this->mexToSend);
                }
            }

            //Communicates the paths to be modified
            for (auto const& it : this->filesToModifySet)
            {
                //TODO
                //std::cout << it << std::endl;
                //this->filesToModifySet.Remove(it);
            }

            //Communicates the paths to be created
            for (auto const& it : this->filesToCreateSet)
            {
                //TODO
                //std::cout << it << std::endl;
                //this->filesToCreateSet.Remove(it);
            }
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
    //Since a SharedSet object encapsulates a set, the elements will be inserted in order.
    switch(fs)
    {
        case FileSystemWatcher::FileStatus::FS_Created:
            debugStr = "Creato";
            this->filesToCreateSet.Insert(path);
            break;
        case FileSystemWatcher::FileStatus::FS_Modified:
            debugStr = "Modificato";
            this->filesToModifySet.Insert(path);
            break;
        case FileSystemWatcher::FileStatus::FS_Erased:
            debugStr = "Cancellato";
            this->filesToDeleteSet.Insert(path);
            break;
        default:
            return;
    }
    //cout << "[DEBUG] Client::NotifyFileChange --> path = " << path << "\n\tStatus = " << debugStr << endl;
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

unordered_map<string,string> Client::GetDigestsFromServer(void)
{
    unordered_map<string,string> digestMap;
    this->mexToSend = "ACK_DIGEST";

    //Try to get first digest.
    this->receivedMex = utils::GetDataSynchronously(this->clientSocket.back());
    //cout << "[DEBUG] Received message: " << this->receivedMex << endl;

    while (this->receivedMex != "DIGESTS_LIST_EMPTY")
    {
        //Extract filename and its digest
        pair<string, string> split = utils::SplitString(this->receivedMex, '\n');
        string filepath = std::move(split.first);
        string digest = std::move(split.second);
        //Store information
        digestMap[std::move(filepath)] = std::move(digest);

        //Send ACK
        utils::SendDataSynchronously(this->clientSocket.back(), mexToSend);

        //Try to get an another digest
        this->receivedMex = utils::GetDataSynchronously(this->clientSocket.back());
        //cout << "[DEBUG] Received message: " << this->receivedMex << endl;
    }
    return digestMap;
}
#pragma endregion
