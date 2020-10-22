#include <iostream>
#include <boost/asio.hpp>
using namespace boost::asio;
using namespace boost::asio::ip;
using std::string;
using std::cout;
using std::cin;
using std::endl;

string getData(tcp::socket& socket)
{
    boost::asio::streambuf buf;
    read_until(socket, buf, '\n');
    string data = buffer_cast<const char*>(buf.data());
    return data;
}

void sendData(tcp::socket& socket, const string& message)
{
    write(socket, buffer(message + "\n"));
}

int main()
{
    io_service io_service;
    //Socket creation
    ip::tcp::socket client_socket(io_service);
    //Try to connect to server
    client_socket.connect(tcp::endpoint(address::from_string("127.0.0.1"), 1996));

    //Infinite loop
    try
    {
        string reply, response;
        while (true)
        {
            // Fetching response
            response = getData(client_socket);

            // Popping last character "\n"
            response.pop_back();

            // Validating if the connection has to be closed
            if (response == "exit")
            {
                cout << "Connection terminated" << endl;
                break;
            }
            cout << response << endl;

            // Reading new message from input stream
            getline(cin, reply);
            sendData(client_socket, reply);

            if (reply == "exit")
                break;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
} 
