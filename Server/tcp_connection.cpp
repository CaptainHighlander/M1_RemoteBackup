#include "tcp_connection.h"
#include "Database/DAO/dao.h"

#include <iostream> //TMP, only for debug print.
#include <boost/bind.hpp>
#include <openssl/sha.h>

using namespace DAO;
using namespace boost::asio;

#pragma region Static public members:
TCP_Connection::pointer TCP_Connection::Create(boost::asio::io_context& io_context)
{
    return pointer(new TCP_Connection(io_context));
}
#pragma endregion

#pragma region Public members:
TCP_Connection::~TCP_Connection(void)
{
    this->Disconnect();
    std::cout << "[DEBUG] Connection closed" << std::endl;
}

tcp::socket& TCP_Connection::GetSocket(void)
{
    return this->socketServer;
}

void TCP_Connection::Start(void)
{
    this->DoLogin();
}
#pragma endregion

#pragma region Private members:
TCP_Connection::TCP_Connection(boost::asio::io_context& io_context) : socketServer(io_context), failedLoginAttempts(0)
{
}

void TCP_Connection::Disconnect(void)
{
    this->socketServer.close();
}

void TCP_Connection::DoLogin(void)
{
    const string loginRequestMsg = "AUTH REQUEST";
    this->outgoingMessage = loginRequestMsg;
    // When initiating the asynchronous operation, and if using boost::bind(),
    // we must specify only the arguments that match the handler's parameter list.
    // Call boost::asio::async_write() to serve the data to the client.
    // We are using boost::asio::async_write(), rather than ip::tcp::socket::async_write_some(),
    // to ensure that the entire block of data is sent.
    async_write(this->socketServer, buffer(this->outgoingMessage + "\n"),
                bind(&TCP_Connection::HandleLoginWrite, shared_from_this(),
                     placeholders::error, placeholders::bytes_transferred));
}

void TCP_Connection::HandleLoginWrite(const boost::system::error_code& error, size_t bytes_transferred)
{
    //std::cout << "[DEBUG] Write operation completed; transferred " << bytes_transferred << "bytes" << std::endl;
    //Ask for login credentials (max MAX_NUMBER_OF_FAILED_LOGINS of fails)
    if (this->associatedUserID.empty() == true && this->failedLoginAttempts <= this->MAX_NUMBER_OF_FAILED_LOGINS)
    {
        //Clear previous incoming message.
        this->incomingMessage.clear();

        //Wait for reading of username and password
        async_read_until(this->socketServer, dynamic_buffer(this->incomingMessage), '\n',
                         bind(&TCP_Connection::HandleLoginRead, shared_from_this(),
                              placeholders::error, placeholders::bytes_transferred));
    }
}

void TCP_Connection::HandleLoginRead(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (this->associatedUserID.empty() == true)
    {
        // Popping last character "\n"
        this->incomingMessage.pop_back();

        //Try to get username and password provided from the client.
        size_t pos = this->incomingMessage.find_first_of(' ');
        if (pos != string::npos)
        {
            //Authentication in progress...
            const string username = this->incomingMessage.substr(0, pos);
            string userPassword = this->incomingMessage.substr(pos + 1);
            pair<bool, Dao::UserInfo> userInfomation = Dao::GetUserInfo(username);
            if (userInfomation.first == true && username == userInfomation.second.GetUserId())
            {
                const string userSalt = userInfomation.second.GetSalt();
                userPassword.append(userSalt);
                unsigned char hash[SHA512_DIGEST_LENGTH];
                unsigned char* computedHashPassword = SHA512(reinterpret_cast<const unsigned char*>(userPassword.c_str()), userPassword.size(), hash);
                string computedHashPasswordStr;
                for (uint32_t i = 0; i < SHA512_DIGEST_LENGTH; i++)
                    //computedHashPasswordStr.append(computedHashPassword[i]);
                    //printf("%02x", );
                std::cout << computedHashPasswordStr << std::endl;

                //Check hash of the password provided by the user.
                if (computedHashPasswordStr == userInfomation.second.GetPasswordHash())
                {
                    std::cout << "Credenziali valide" << std::endl;
                    this->outgoingMessage = "AUTHENTICATED";
                    this->associatedUserID = username;
                    async_write(this->socketServer, buffer(this->outgoingMessage + "\n"),
                                bind(&TCP_Connection::HandleLoginWrite, shared_from_this(),
                                     placeholders::error, placeholders::bytes_transferred));

                    //Successful login: don't ask again for login.
                    return;
                }
            }
        }

        //Wrong login: try again or close the connection
        this->failedLoginAttempts += 1;
        this->outgoingMessage = (this->failedLoginAttempts <= this->MAX_NUMBER_OF_FAILED_LOGINS) ? "AUTH REQUEST RETRY" : "ACCESS DENIED";
        async_write(this->socketServer, buffer(this->outgoingMessage + "\n"),
                    bind(&TCP_Connection::HandleLoginWrite, shared_from_this(),
                         placeholders::error, placeholders::bytes_transferred));
    }
    //std::cout << "[DEBUG] Read operation completed; transferred " << bytes_transferred << "bytes" << std::endl;
}
#pragma endregion
