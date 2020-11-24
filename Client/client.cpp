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
        this->DoLogin(this->clientSocket.back());
        if (this->bIsAuthenticated == false)
            return; //User isn't logged: client will not be able to continue it'execution. So, it's stopped.

        this->pathToWatch = "./FoldersTest/Riccardo_Client";
        FileSystemWatcher fsw { pathToWatch };
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
    string debugStr;
    switch(fs)
    {
        case FileSystemWatcher::FileStatus::FS_Created:
            debugStr = "Creato";
            break;
        case FileSystemWatcher::FileStatus::FS_Modified:
            debugStr = "Modificato";
            break;
        case FileSystemWatcher::FileStatus::FS_Erased:
            debugStr = "Cancellato";
            break;
        default:
            return;
    }
    cout << "[DEBUG] Client::NotifyFileChange --> path = " << path << "\n\tStatus =" << debugStr <<  endl;
    utils::SendFile(this->clientSocket.back(), this->pathToWatch + path);
}
#pragma endregion

#pragma region Private static members:
string Client::GetData(tcp::socket& socket)
{
    boost::asio::streambuf buf;
    read_until(socket, buf, '\n');
    string data = buffer_cast<const char*>(buf.data());
    data.pop_back();
    return data;
}

void Client::SendData(tcp::socket& socket, const string& message)
{
    write(socket, buffer(message + "\n"));
}
#pragma

#pragma Private members:
void Client::DoLogin(ip::tcp::socket& client_socket)
{
    bool bFirstContinuationCondition;
    do
    {
        this->bIsAuthenticated = false;

        //Fetching a mex from the server.
        this->receivedMex = Client::GetData(client_socket);
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
            Client::SendData(client_socket, this->mexToSend);
        }
        else if (this->bIsAuthenticated == false)
            cout << "Connection terminated" << endl;
    }   while (bFirstContinuationCondition && this->mexToSend != "EXIT");

    //Reset message.
    this->mexToSend = "";
    this->receivedMex = "";
}

list<pair<string, string>> Client::GetDigestFromServer(void) const
{
    list<pair<string, string>> digestList;
    return digestList;
}
#pragma endregion
