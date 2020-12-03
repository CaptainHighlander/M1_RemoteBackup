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

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

#ifndef DELIMITATOR
#define DELIMITATOR     "§DELIMITATOR§"
#endif

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

        do
        {
            //Communicates the paths to be deleted
            while (this->filesToDeleteSet.IsEmpty() == false)
            {
                //Extract the next element (it will be removed from the set of file to delete).
                std::optional<string> pathToDelete = this->filesToDeleteSet.Extract();
                if (pathToDelete.has_value() == true)
                {
                    this->mexToSend = string("RM") + DELIMITATOR + pathToDelete.value();
                    //std::cout << "[DEBUG] Sending " << this->mexToSend << std::endl;
                    utils::SendDataSynchronously(this->clientSocket.back(), this->mexToSend);
                    //Wait for ACK.
                    this->receivedMex = utils::GetDataSynchronously(this->clientSocket.back());
                }
            }

            //Communicates the paths to be added
            for (auto const& it : this->filesToSendMap)
            {
                const string pathName = it.first;
                ssize_t totalBytesSent = it.second.first;
                const ssize_t totalBytesToSent = it.second.second;
                //std::cout << "[DEBUG] " << pathName << "\n\t" << totalBytesSent << " | " << totalBytesToSent << std::endl;

                if (totalBytesToSent <= -1) //Communicates the paths of the folder to be created
                {
                    this->mexToSend = string("NEW_DIR") + DELIMITATOR + pathName;
                    utils::SendDataSynchronously(this->clientSocket.back(), this->mexToSend);
                }
                else //Send a file chunk by chunk
                {
                    //Set a buffer having an appropriate size
                    const size_t bufferSize = ((totalBytesToSent - totalBytesSent) >= BUFFER_SIZE) ? BUFFER_SIZE : (totalBytesToSent - totalBytesSent);

                    //Set message to send before the chunk of the current file.
                    if (totalBytesSent > 0)
                        this->mexToSend = string("FILE") + DELIMITATOR + string("APPEND") + DELIMITATOR + pathName + DELIMITATOR;
                    else
                        this->mexToSend = string("FILE") + DELIMITATOR + string("NEW") + DELIMITATOR + pathName + DELIMITATOR;

                    //Send a chunk of the current file
                    const ssize_t bytesSent = utils::SendFile(this->clientSocket.back(), this->pathToWatch + pathName, bufferSize, totalBytesSent, this->mexToSend);
                    if (bytesSent > -1) //A new portion of the current file has been sent.
                    {
                        //Update info about the current file.
                        totalBytesSent += bytesSent;
                        this->filesToSendMap.InsertOrUpdate(pathName, std::make_pair(totalBytesSent, totalBytesToSent));
                    }
                }

                //Wait for ACK
                this->receivedMex = utils::GetDataSynchronously(this->clientSocket.back());

                //Check if a folder has been sent or if a file has TOTALLY been sent.
                if (totalBytesSent >= totalBytesToSent)
                {
                    //Remove folder/file from the map because server now has it
                    this->filesToSendMap.Remove(pathName);
                }
            }
        }
        while (this->receivedMex != "EXIT" && this->mexToSend != "EXIT");
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception\n\t" << e.what() << endl;
    }
}

void Client::NotifyFileChange(const string& path, const FileSystemWatcher::FileStatus fileStatus)
{
    const string fullPath = this->pathToWatch + path;
    //The size of the file or -1 if the path corresponds to a folder.
    ssize_t sizeOfFile;
    if (fileStatus != FileSystemWatcher::FileStatus::FS_Erased)
        sizeOfFile = (fs::is_directory(fullPath) == false) ? fs::file_size(fullPath) : -1;

    switch (fileStatus)
    {
        case FileSystemWatcher::FileStatus::FS_Created:
        case FileSystemWatcher::FileStatus::FS_Modified:
            this->filesToSendMap.InsertOrUpdate(path, std::make_pair(0, sizeOfFile));
            break;
        case FileSystemWatcher::FileStatus::FS_Erased:
            this->filesToDeleteSet.Insert(path);
            break;
        default:
            return;
    }
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
            //Send ACK
            this->mexToSend = "ACK";
            utils::SendDataSynchronously(this->clientSocket.back(), this->mexToSend);
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
        vector<string> split = utils::GetSubstrings(this->receivedMex, DELIMITATOR);
        string filepath = split[0];
        string digest = split[1];

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
