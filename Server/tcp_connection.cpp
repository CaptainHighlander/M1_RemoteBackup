#include "tcp_connection.h"
#include "Database/DAO/dao.h"
#include "../Common/Utils/utils.h"

#include <iostream> //TMP, only for debug.
#include <fstream>
#include <iomanip>
#include <experimental/filesystem>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <openssl/sha.h>
#include <list>

using std::list;
using namespace DAO;
using namespace boost::asio;
namespace fs = std::experimental::filesystem;

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
    try
    {
        this->ManageConnection();
    }
    catch (const std::exception& ex)
    {
    }
}
#pragma endregion

#pragma region Private members:
TCP_Connection::TCP_Connection(boost::asio::io_context& io_context) : socketServer(io_context), failedLoginAttempts(0)
{
    this->associatedUserID = "";
}

void TCP_Connection::Disconnect(void)
{
    this->socketServer.close();
}

void TCP_Connection::ManageConnection(void)
{
    //Try to do login.
    this->DoLogin();

    //Reset incoming and outgoing messages.
    this->incomingMessage.clear();
    this->outgoingMessage.clear();

    //A new connection has been established:
    if (this->associatedUserID.empty() == false)
    {
        //Check if the folder associated to the connected user has the same files that are in the folder client-side.
        this->CheckSynchronization();

        //TODO - Incoming files
        do
        {

        }   while (this->incomingMessage != "EXIT" && this->outgoingMessage != "EXIT");
    }
}

void TCP_Connection::CheckSynchronization(void)
{
    //Creation of a list of pair (file name, digest).
    list<pair<string,string>> digestList;
    string digestStr;
    for (auto &path_iterator : fs::recursive_directory_iterator(USERS_PATH + this->associatedUserID))
    {
        string pathName = path_iterator.path().string();
        //Check if the current path is a directory or a regular file
        if (fs::is_directory(path_iterator.path()) == false && fs::is_regular_file(path_iterator.path()) == false)
            continue;

        //Compute current digest.
        digestStr = utils::DigestFromFile(pathName);

        //Remove main path from the current path
        utils::EraseSubStr(pathName, USERS_PATH + this->associatedUserID);

        //Store computed digest
        digestList.push_back(std::make_pair(pathName, std::move(digestStr)));
    }

    //For each computed digest, send both it and its file name to the the client
    while (digestList.empty() == false)
    {
        //Get an element from the list of digests.
        auto const& digest_iterator = digestList.front();
        this->outgoingMessage = digest_iterator.first + "\n" + digest_iterator.second;

        //Send a message to client
        utils::SendDataSynchronously(this->socketServer, this->outgoingMessage);

        //Wait ACK
        do
        {
            this->incomingMessage = utils::GetDataSynchronously(this->socketServer);
        }   while (this->incomingMessage != "ACK_DIGEST");

        //Remove element from the list.
        digestList.pop_front();
    }
    //There are no further digests to send.
    this->outgoingMessage = "DIGESTS_LIST_EMPTY";
    //Send a message to client
    utils::SendDataSynchronously(this->socketServer, this->outgoingMessage);
}

void TCP_Connection::DoLogin(void)
{
    std::cout << "[DEBUG] Start login" << std::endl;

    this->outgoingMessage = "AUTH REQUEST";
    //Send a message to client
    utils::SendDataSynchronously(this->socketServer, this->outgoingMessage);

    while (this->associatedUserID.empty() == true && this->failedLoginAttempts <= this->MAX_NUMBER_OF_FAILED_LOGINS)
    {
        //Get a new message from the client
        this->incomingMessage = utils::GetDataSynchronously(this->socketServer);

        //Verify provided credentials and provide a response to the client:
        if (this->CheckLoginCredentials() == true)
        {
            this->outgoingMessage = "AUTHENTICATED";
        }
        else
        {
            this->failedLoginAttempts += 1;
            this->outgoingMessage = (this->failedLoginAttempts <= this->MAX_NUMBER_OF_FAILED_LOGINS) ? "AUTH REQUEST RETRY" : "ACCESS DENIED";
        }
        //Send a message to client
        utils::SendDataSynchronously(this->socketServer, this->outgoingMessage);
    }
}

bool TCP_Connection::CheckLoginCredentials(void)
{
    if (this->associatedUserID.empty() == false)
        return false;

    //Try to get username and password provided from the client.
    const size_t pos = this->incomingMessage.find_first_of(' ');
    if (pos != string::npos) //User has provided an username and a password.
    {
        /**** Authentication in progress... ****/

        const string username = this->incomingMessage.substr(0, pos);
        string userPassword = this->incomingMessage.substr(pos + 1);

        //Try to get user's information from database.
        pair<bool, Dao::UserInfo> userInfomation = Dao::GetUserInfo(username);

        if (userInfomation.first == true && username == userInfomation.second.GetUserId())
        {
            //Append salt to password.
            const string userSalt = userInfomation.second.GetSalt();
            userPassword.append(userSalt);
            //Compute hash(psw || salt) using SHA512 as hash algorithm
            unsigned char computedHashPassword[SHA512_DIGEST_LENGTH];
            SHA512(reinterpret_cast<const unsigned char*>(userPassword.c_str()), userPassword.size(), computedHashPassword);

            //Hash converted to string
            std::ostringstream ossComputedHashPasswordStr;
            for (uint32_t i = 0; i < SHA512_DIGEST_LENGTH; ++i)
            {
                ossComputedHashPasswordStr << std::hex << std::setfill('0') << std::setw(2) << + computedHashPassword[i];
            }

            //Compare computed hash with the hash that is in the database.
            //boost::iequals performs a case-insensitive string comparison.
            if (boost::iequals(ossComputedHashPasswordStr.str(), userInfomation.second.GetPasswordHash()))
            {
                //Username and password are valid ==> Successful login: don't ask again for login.
                this->associatedUserID = username;
                return true;
            }
        }
    }

    //Wrong username and/or password
    return false;
}

void TCP_Connection::HandleReadFile(const boost::system::error_code& error, size_t bytes_transferred)
{
    //TMP
    std::ofstream ofs;
    ofs.open("txtServer.txt", std::ios::app);
    if (ofs.is_open() == true)
    {
        /*std::cout << this->incomingMessage << std::endl;*/
        ofs << this->incomingMessage;
    }
    ofs.close();
    std::cout << "[DEBUG] End of file transfer" << std::endl;
}
#pragma endregion
