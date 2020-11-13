#include "client.h"
#include "FileSystemWatcher/file_system_watcher.h"
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
                FileSystemWatcher fsw { "../FoldersTest/Riccardo", true };

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
        std::cerr << e.what() << std::endl;
    }
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
