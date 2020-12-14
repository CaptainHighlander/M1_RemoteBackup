#include "tcp_connection.h"
#include "Database/DAO/dao.h"

#include <iostream> //TMP, only for debug.
#include <fstream>
#include <iomanip>
#include <experimental/filesystem>
#include <boost/algorithm/string.hpp>
#include <openssl/sha.h>
#include <list>

using std::list;
using namespace DAO;
using namespace boost::asio;
namespace fs = std::experimental::filesystem;

#ifndef DELIMITATOR
#define DELIMITATOR     "§DELIMITATOR§"
#endif

#pragma region Static attributes:
unordered_map<uint64_t, TCP_Connection::pointer> TCP_Connection::connectionsMap{};
#pragma endregion

#pragma region Constructor and destructor:
TCP_Connection::TCP_Connection(tcp::socket io_context, const uint64_t _id)
        :   socketServer(std::move(io_context)), failedLoginAttempts(0), id(_id)
{
}

TCP_Connection::~TCP_Connection(void)
{
    TCP_Connection::connectionsMap.erase(this->id);
    this->Disconnect();
    if (this->incomingMessage == "EXIT")
        std::cout << "[DEBUG] TCP_Connection --> Connection closed by the CLIENT" << std::endl;
    else
        std::cout << "[DEBUG] TCP_Connection --> Connection closed by the SERVER" << std::endl;
}
#pragma endregion

#pragma region Static public members:
TCP_Connection::pointer TCP_Connection::Create(tcp::socket io_context, const uint64_t id)
{
    auto connectionPointer = pointer(new TCP_Connection(std::move(io_context), id));
    TCP_Connection::connectionsMap[id] = connectionPointer;
    return connectionPointer;
}

unordered_map<uint64_t, TCP_Connection::pointer> TCP_Connection::GetConnectionsMap(void)
{
    return TCP_Connection::connectionsMap;
}
#pragma endregion

#pragma region Public members:
void TCP_Connection::Start(void)
{
    try
    {
        this->ManageConnection();
    }
    catch (const std::exception& ex)
    {
        //Destructor will be called due to this instruction since there are no further reference to this connection.
        TCP_Connection::connectionsMap.erase(this->id);
    }
}

void TCP_Connection::Close(void)
{
    delete this;
}
#pragma endregion

#pragma region Private members:
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
        //Wait for ACK after to AUTHENTICATED message.
        this->incomingMessage = utils::GetStringSynchronously(this->socketServer);

        //Set folder associated to logged user.
        this->userFolder = USERS_PATH + this->associatedUserID;

        //Check if the folder associated to the connected user has the same files that are in the folder client-side.
        this->CheckSynchronization();

        /* INCOMING COMMANDS */
        this->ManageCommunicationWithClient();
    }

    //Destructor will be called due to this instruction since there are no further reference to this connection.
    TCP_Connection::connectionsMap.erase(this->id);
}

void TCP_Connection::DoLogin(void)
{
    std::cout << "[DEBUG] Start login" << std::endl;

    this->outgoingMessage = "AUTH REQUEST";
    //Send a message to client
    utils::SendStringSynchronously(this->socketServer, this->outgoingMessage);

    while (this->associatedUserID.empty() == true && this->failedLoginAttempts <= this->MAX_NUMBER_OF_FAILED_LOGINS)
    {
        //Get a new message from the client
        this->incomingMessage = utils::GetStringSynchronously(this->socketServer);

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
        utils::SendStringSynchronously(this->socketServer, this->outgoingMessage);
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
            unsigned char computedHashPassword[SHA512_DIGEST_LENGTH]{};
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

void TCP_Connection::CheckSynchronization(void)
{
    //Creation of a list of pair (file name, digest).
    list<pair<string,string>> digestList;
    std::optional<string> digestStr;
    for (auto &path_iterator : fs::recursive_directory_iterator(this->userFolder))
    {
        string pathName = path_iterator.path().string();
        //Check if the current path is a directory or a regular file
        if (fs::is_directory(path_iterator.path()) == false && fs::is_regular_file(path_iterator.path()) == false)
            continue;

        //Compute current digest.
        digestStr = utils::DigestFromFile(pathName);
        if (digestStr.has_value() == false)
            continue;

        //Remove main path from the current path
        utils::EraseSubStr(pathName, this->userFolder);

        //Store computed digest
        digestList.push_back(std::make_pair(pathName, std::move(digestStr.value())));
    }

    //For each computed digest, send both it and its file name to the the client annd say if the path is a directory or a file
    while (digestList.empty() == false)
    {
        //Get an element from the list of digests.
        auto const& digest_iterator = digestList.front();
        this->outgoingMessage = digest_iterator.first + DELIMITATOR + digest_iterator.second;

        //Send a message to client
        utils::SendStringSynchronously(this->socketServer, this->outgoingMessage);

        //Wait ACK
        do
        {
            this->incomingMessage = utils::GetStringSynchronously(this->socketServer);
        }   while (this->incomingMessage != "ACK_DIGEST");

        //Remove element from the list.
        digestList.pop_front();
    }

    //There are no further digests to send.
    this->outgoingMessage = "DIGESTS_LIST_EMPTY";

    //Send a message to client
    utils::SendStringSynchronously(this->socketServer, this->outgoingMessage);
}

void TCP_Connection::ManageCommunicationWithClient(void)
{
    vector<string> receivedMexSubstrings;
    this->outgoingMessage = "ACK";
    do
    {
        //Get a command from the client
        this->incomingMessage.clear(); //Reset incoming message.
        this->incomingMessage = utils::GetStringSynchronously(this->socketServer);
        receivedMexSubstrings = utils::GetSubstrings(this->incomingMessage, DELIMITATOR);
        std::cout << "[DEBUG] Received\n";
        for (auto const & it : receivedMexSubstrings)
            std::cout << "\t" << it << std::endl;

        //Deleting a path
        if (receivedMexSubstrings[0] == "RM")
        {
            const string pathToRemove = this->userFolder + receivedMexSubstrings[1];
            if (fs::exists(pathToRemove) == true)
                fs::remove(pathToRemove); //Perform deletion.
        }

        //Adding a path
        if (receivedMexSubstrings[0] == "NEW_DIR")
        {
            fs::create_directory(this->userFolder + receivedMexSubstrings[1]);
        }
        else if (receivedMexSubstrings[0] == "FILE")
        {
            std::ofstream outputFile;

            //Open file using info provided by the client.
            if (receivedMexSubstrings[1] == "NEW")
            {
                outputFile.open(this->userFolder + receivedMexSubstrings[2], std::ios_base::binary | std::ios_base::out);
            }
            else if (receivedMexSubstrings[1] == "APPEND")
                outputFile.open(this->userFolder + receivedMexSubstrings[2], std::ios_base::binary | std::ios_base::app);

            //Send READY
            utils::SendStringSynchronously(this->socketServer, "READY");

            //Get a chunk of the file and save it.
            const size_t byteToRead = std::stoi(receivedMexSubstrings[3]);
            const ssize_t byteRead = utils::GetBytesSynchronously(this->socketServer, this->byteBuffer.data(), byteToRead);
            if (outputFile.is_open() == true && byteRead == byteToRead)
            {
                outputFile.write(this->byteBuffer.data(), byteRead);
                //Clear flags and close the file.
                outputFile.clear();
                outputFile.close();
            }
        }

        //Send ACK - It says to client that server has received a message and so it's ready to get a new one.
        utils::SendStringSynchronously(this->socketServer, this->outgoingMessage);
    }   while (this->incomingMessage != "EXIT" && this->outgoingMessage != "EXIT");
}
#pragma endregion
