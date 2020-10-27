#include "tcp_connection.h"

#include <iostream>
#include <boost/bind.hpp>
#include <openssl/sha.h>

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
    if (this->associatedUserID.empty() == true && this->failedLoginAttempts < this->MAX_NUMBER_OF_FAILED_LOGINS)
    {
        //Clear previous incoming message.
        this->incomingMessage.clear();

        //Wait for reading of username and password
        async_read_until(this->socketServer, dynamic_buffer(this->incomingMessage), '\n',
                         bind(&TCP_Connection::HandleLoginRead, shared_from_this(),
                              placeholders::error, placeholders::bytes_transferred));
    }
    else
    {
        this->outgoingMessage = "ABORT";
        async_write(this->socketServer, buffer(this->outgoingMessage + "\n"),
                    bind(&TCP_Connection::HandleLoginWrite, shared_from_this(),
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
        string username;
        string password;
        size_t pos = this->incomingMessage.find_first_of(' ');
        if (pos != std::string::npos)
        {
            username = this->incomingMessage.substr(0, pos);
            password = this->incomingMessage.substr(pos + 1);
            /*
            const char* salt= "fVgxLO/@nLJya.q5q.hNLzQz@:j9dx6=.8PBu0rn2#Fop?RBb+2k0n7N)/58Sxb+*itS#ugqNihf#/O5CQlOq!wr0*6HzQCxDhw)eUu8L-qI21NP4kXMSmS#nGj*?#(a";
            std::string password = "8d46edd1a1defe33654544e08b1e43b2CaptainHighlander";
            size_t len = password.size();
            unsigned char hash[SHA512_DIGEST_LENGTH];

            auto d1 = SHA512(password.c_str(), len, hash);
            auto d2 = SHA512(password, size_t(4), hash);
            if(d1 == d2)
                std::cout << "Equals" << std::endl;

            int i;
            for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
                printf("%02x", d1[i]);
            putchar('\n');
            for (i = 0; i < SHA256_DIGEST_LENGTH; i++)
                printf("%02x", d2[i]);
            putchar('\n');
            */
            //Stupid authentication. It will be replaced with read from database and password hash plus salt.
            if(username == "Riccardo" && password == "password")
            {
                this->associatedUserID = username;
                async_write(this->socketServer, buffer("You are logged!\n"),
                            bind(&TCP_Connection::HandleLoginWrite, shared_from_this(),
                                 placeholders::error, placeholders::bytes_transferred));

                //Successful login: don't ask
                return;
            }
        }

        //Try again for login.
        this->outgoingMessage = "AUTH REQUEST RETRY";
        this->failedLoginAttempts += 1;
        async_write(this->socketServer, buffer(this->outgoingMessage + "\n"),
                    bind(&TCP_Connection::HandleLoginWrite, shared_from_this(),
                         placeholders::error, placeholders::bytes_transferred));
    }
    //std::cout << "[DEBUG] Read operation completed; transferred " << bytes_transferred << "bytes" << std::endl;
}
#pragma endregion
