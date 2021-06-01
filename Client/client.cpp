#include "client.h"
#include <iostream>

#include "../Common/Utils/utils.h"

using std::string;
using std::cout;
using std::cin;
using std::endl;

#ifndef DELIMITATOR
#define DELIMITATOR     "§DELIMITATOR§"
#endif

#pragma region Signal handler:
std::function<void(int)> signalHandler = nullptr;
void signal_handler(int i)
{
    signalHandler(i);
    exit(i);
}
#pragma endregion

#pragma region Constructor:
Client::Client(const string& _address, const uint16_t _port, const char* _pathToWatch)
    : address(_address), port(_port), bIsAuthenticated(false), errorFromFSW(0), pathToWatch(_pathToWatch)
{
    while (this->pathToWatch.back() == '/')
        this->pathToWatch.pop_back();

    //Set signals to catch.
    signalHandler = std::bind(&Client::SignalHandler, this, std::placeholders::_1);
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGSEGV, signal_handler);
}

Client::~Client(void)
{
    //Close the socket
    if (this->clientSocket.empty() == false)
        this->clientSocket.back().close();
    std::cout << "[DEBUG] Destructor of client" << std::endl;
}
#pragma endregion

#pragma Public members:
void Client::SignalHandler(const int signum)
{
    //A clean deletion is performed.
    this->~Client();
}

void Client::Run(void)
{
    //Try to establish a connection with the server.
    if (this->ConnectToServer() == false)
    {
        std::cerr << "Ops, the server appears to be unvailable. Please, try later..." << std::endl;
        return;
    }

    /**** Execution... ****/
    try
    {
        //Try to do login.
        this->DoLogin();
        if (this->bIsAuthenticated == false)
            return; //User isn't logged: client will not be able to continue it'execution. So, it's stopped.

        //Init a file watcher checking for the synchronization between client and server
        const FileSystemWatcher::notificationFunc fswActionFunc = std::bind(&Client::NotifyFileChange, this, std::placeholders::_1, std::placeholders::_2);
        this->fsw = FileSystemWatcher::Create(pathToWatch, this->GetDigestsFromServer(), fswActionFunc);
        fsw->StartWatch();

        /* OUTGOING COMMANDS */
        do
        {
            //Check for errors of FileSystemWatcher
            if (this->GetErrorFromFWS() == 0) //No errors
            {
                //Communicates to the server the paths to be deleted
                this->CommunicateDeletions();

                //Communicates to the server if a path was added/modified and send the corrisponding file.
                this->CommunicateCreationOrChanges();
            }
            else //Some errors occurs during the execution of the FileSytstemWatcher
            {
                //Close connection between client and server, print an error and exit.
                this->mexToSend = "EXIT"; //It's also an exit condition.
                utils::SendStringSynchronously(this->clientSocket.back(), this->mexToSend);
                if (this->GetErrorFromFWS() == FileSystemWatcher::FileStatus::FS_Error_MissingMainFolder)
                    std::cerr << "Cannot find the folder to monitor. Please, check its name" << std::endl;
                else
                    std::cerr << "An unexpected error occurs during the monitoring of the folder to monitor" << std::endl;
                std::cout << "Connection closed" << std::endl;
            }
        }
        while (this->receivedMex != "EXIT" && this->mexToSend != "EXIT");
    }
    catch (const boost::wrapexcept<boost::system::system_error>& boostE)
    {
        //End of file during a read from a socket or during a write on a socket.
        std::cerr << "Server cannot be reached. Please, try later!" << std::endl;
    }
    catch (const std::exception& otherExeception)
    {
        std::cerr << "Something went wrong! (" << otherExeception.what() << ")" << std::endl;
    }
}

void Client::NotifyFileChange(const string& path, const FileSystemWatcher::FileStatus fileStatus)
{
    const string fullPath = this->pathToWatch + path;
    //The size of the file or -1 if the path corresponds to a folder.
    ssize_t sizeOfFile;
    if (fileStatus != FileSystemWatcher::FileStatus::FS_Erased || path.empty() == false)
        sizeOfFile = (fs::exists(fullPath) == true && fs::is_directory(fullPath) == false) ? fs::file_size(fullPath) : -1;

    switch (fileStatus)
    {
        case FileSystemWatcher::FileStatus::FS_Error_MissingMainFolder:
        case FileSystemWatcher::FileStatus::FS_Error_Generic:
            this->filesToDeleteSet.Clear();
            this->filesToSendMap.Clear();
            this->SetErrorFromFSW(fileStatus);
            break;
        case FileSystemWatcher::FileStatus::FS_Created:
        case FileSystemWatcher::FileStatus::FS_Modified:
            if (fs::exists(fullPath) == true)
                this->filesToSendMap.InsertOrUpdate(path, std::make_pair(0, sizeOfFile));
            break;
        case FileSystemWatcher::FileStatus::FS_Erased:
            if (fullPath != this->pathToWatch)
                this->filesToDeleteSet.Insert(path);
            break;
        default:
            return;
    }
}
#pragma endregion

#pragma Private members:
bool Client::ConnectToServer(void)
{
    io_service io_service;
    //Socket creation
    this->clientSocket.push_back(ip::tcp::socket(io_service));
    //Try to connect to server
    boost::system::error_code err;
    this->clientSocket.back().connect(tcp::endpoint(address::from_string(this->address), this->port), err);
    if (err)
    {
        return false;
    }
    return true;
}

void Client::DoLogin(void)
{
    bool bFirstContinuationCondition;
    do
    {
        this->bIsAuthenticated = false;

        //Fetching a mex from the server.
        this->receivedMex = utils::GetStringSynchronously(this->clientSocket.back());
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
            utils::SendStringSynchronously(this->clientSocket.back(), this->mexToSend);
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
    this->mexToSend = "";

    //Try to get first digest.
    this->receivedMex = utils::GetStringSynchronously(this->clientSocket.back());
    //cout << "[DEBUG] Received message: " << this->receivedMex << endl;

    while (this->receivedMex != "DIGESTS_LIST_EMPTY")
    {
        //Extract filename and its digest
        vector<string> split = utils::GetSubstrings(this->receivedMex, DELIMITATOR);
        if (split.size() != 2)
            continue;
        string filepath = split[0];
        string digest = split[1];

        //Store information
        digestMap[std::move(filepath)] = std::move(digest);

        //Try to get an another digest
        this->receivedMex = utils::GetStringSynchronously(this->clientSocket.back());
        //cout << "[DEBUG] Received message: " << this->receivedMex << endl;
    }
    return digestMap;
}

void Client::CommunicateDeletions(void)
{
    while (this->filesToDeleteSet.IsEmpty() == false && this->GetErrorFromFWS() == 0)
    {
        //Extract the next element (it will be removed from the set of file to delete).
        std::optional<string> pathToDelete = this->filesToDeleteSet.Extract();
        if (pathToDelete.has_value() == true)
        {
            this->mexToSend = string("RM") + DELIMITATOR + std::move(pathToDelete.value());
            //std::cout << "[DEBUG] Sending " << this->mexToSend << std::endl;
            utils::SendStringSynchronously(this->clientSocket.back(), this->mexToSend);
        }
    }
}

void Client::CommunicateCreationOrChanges(void)
{
    //Buffer to send when an error occurs during a reading a file.
    static const char ERROR_BUFFER[BUFFER_SIZE] = {'\0'};
    bool bError = false;

    auto iterator = this->filesToSendMap.begin();
    while (iterator != this->filesToSendMap.end() && this->GetErrorFromFWS() == 0)
    {
        bError = false;
        //Reset incoming and outgoing messages.
        this->receivedMex.clear();
        this->mexToSend.clear();

        //Get next path to send.
        auto it = (*iterator);
        const string pathName = it.first;
        ssize_t totalBytesSent = it.second.first;
        const ssize_t totalBytesToSent = it.second.second;

        //Check if path exist (e.g. a big folder can be added and instantly deleted)
        if (fs::exists(this->pathToWatch + pathName) == false)
        {
            //Remove path from the map because it no longer exist and go to the next element.
            iterator = this->filesToSendMap.Remove(iterator);
            continue;
        }

        if (totalBytesToSent <= -1) //Communicates the paths of the folder to be created
        {
            this->mexToSend = string("NEW_DIR") + DELIMITATOR + pathName;
            utils::SendStringSynchronously(this->clientSocket.back(), this->mexToSend);
        }
        else //Send a file chunk by chunk
        {
            //Set a buffer having an appropriate size
            const size_t bytesToRead = ((totalBytesToSent - totalBytesSent) >= BUFFER_SIZE) ? BUFFER_SIZE : (totalBytesToSent - totalBytesSent);

            //Set message to send before the chunk of the current file and wait for an ack.
            //This message provides the following information:
            //1) the name of the file;
            //2) if the file have to be created or if the chunk will be appended.
            //3) the size of the chunk.
            if (totalBytesSent > 0)
                this->mexToSend = string("FILE") + DELIMITATOR + string("APPEND") + DELIMITATOR + pathName + DELIMITATOR + std::to_string(bytesToRead);
            else
                this->mexToSend = string("FILE") + DELIMITATOR + string("NEW") + DELIMITATOR + pathName + DELIMITATOR + std::to_string(bytesToRead);
            utils::SendStringSynchronously(this->clientSocket.back(), this->mexToSend);

            //Send a chunk of the current file
            const ssize_t bytesSent = utils::SendFile(this->clientSocket.back(), this->pathToWatch + pathName, totalBytesSent);
            if (bytesSent > (-1)) //A new portion of the current file has been sent.
            {
                //Update info about the current file.
                totalBytesSent += bytesSent;
                this->filesToSendMap.InsertOrUpdate(pathName, std::make_pair(totalBytesSent, totalBytesToSent));
            }
            else //If an error occurs and a portion of the current file wasn't sent...
            {
                bError = true;
                //Send an empty buffer.
                utils::SendBytesSynchronously(this->clientSocket.back(), ERROR_BUFFER, bytesToRead);
                //Remove file from the map because an error occurs and go to the next element.
                iterator = this->filesToSendMap.Remove(iterator);
            }
        }

        if (bError == false)
        {
            //Check if a folder has been sent or if a file has TOTALLY been sent.
            if (totalBytesSent >= totalBytesToSent)
            {
                //Remove folder/file from the map because server now has it and go to the next element.
                iterator = this->filesToSendMap.Remove(iterator);
            }
            else
            {
                iterator++; //Go to the next element.
            }
        }
   }
}

uint8_t Client::GetErrorFromFWS(void)
{
    const std::lock_guard<std::mutex> lg{this->m_errorFromFSW};
    return this->errorFromFSW;
}

void Client::SetErrorFromFSW(const uint8_t errorCode)
{
    const std::lock_guard<std::mutex> lg{this->m_errorFromFSW};
    this->errorFromFSW = errorCode;
}
#pragma endregion
