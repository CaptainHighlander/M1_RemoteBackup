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
    : address(_address), port(_port)
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
        string reply, response;
        while (true)
        {
            // Fetching response
            response = Client::GetData(client_socket);

            // Popping last character "\n"
            response.pop_back();

            cout << "[DEBUG] " << response << endl;
            if (response == "AUTH REQUEST")
            {
                cout << "Insert username and password" << endl;
            }
            else if (response == "AUTH REQUEST RETRY")
            {
                cout << "Incorrect credentials. Please, try again" << endl;
            }
            else if (response == "AUTHENTICATED")
            {
                cout << "Successful login" << endl;
                //Maybe it should be created in a different thread.
                FileSystemWatcher fsw { "./FoldersTest/Riccardo" };
                const std::function<void(const std::string&, FileSystemWatcher::FileStatus)> fswActionFunc = std::bind(&Client::NotifyFileChange, this, std::placeholders::_1, std::placeholders::_2);
                fsw.StartWatch(fswActionFunc);
                cout << "Spero di vedere questa scritta, prossimamente..." << endl;
            }
            else if (response == "ACCESS DENIED")
            {
                cout << "Access denied" << endl;
                break;
            }

            // Validating if the connection has to be closed
            if (response == "exit")
            {
                cout << "Connection terminated" << endl;
                break;
            }

            // Reading new message from input stream
            getline(cin, reply);
            Client::SendData(client_socket, reply);

            if (reply == "exit")
                break;
        }
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
    return data;
}

void Client::SendData(tcp::socket& socket, const string& message)
{
    write(socket, buffer(message + "\n"));
}
#pragma endregion
