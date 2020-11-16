#include "client.h"
#include <iostream>
#include <boost/asio.hpp>
#include <fstream>

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
    ip::tcp::socket client_socket(io_service);
    //Try to connect to server
    client_socket.connect(tcp::endpoint(address::from_string(this->address), this->port));

    try
    {
        /*
        int length = 2048;
        char* line = new char [length];
        std::ifstream ifs("txtClient.txt");
        if (ifs.is_open())
        {
            while (!ifs.eof())
            {
                ifs.read(line,length);
                Client::SendData(client_socket, line);
            }
            ifs.close();
        }
        delete[] line;
        */
        this->DoLogin(client_socket);
        if (this->bIsAuthenticated == false)
            return; //User isn't logged: client will not be able to continue it'execution. So, it's stopped.

        FileSystemWatcher fsw { "./FoldersTest/Riccardo" };
        const std::function<void(const std::string&, FileSystemWatcher::FileStatus)> fswActionFunc = std::bind(&Client::NotifyFileChange, this, std::placeholders::_1, std::placeholders::_2);
        fsw.StartWatch(fswActionFunc);
        do
        {
            ;
        }
        while(this->receivedMex != "EXIT" && this->mexToSend != "EXIT");
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
    }   while (bFirstContinuationCondition && this->mexToSend != "EXIT" );

    //Reset message.
    this->mexToSend = "";
    this->receivedMex = "";
}
#pragma endregion
