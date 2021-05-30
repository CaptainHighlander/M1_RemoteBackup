#include "tcp_server.h"
#include "../Common/ThreadGuard/thread_guard.h"
#include <iostream> //TMP, only for debug.
#include <boost/bind.hpp>

#define MAX_ALLOWED_THREAD    100

#pragma region Constructors and destructor:
    TCP_Server::TCP_Server(boost::asio::io_context& io_context, tcp version, uint16_t portNumber)
            : io_contextServer(io_context), signalsToIngnore(io_context), signals(io_context),
              acceptorServer(io_context, tcp::endpoint(version, portNumber)), nextConnectionId(0)
    {
        //The following signals will be catched but not handled.
        signalsToIngnore.add(SIGSEGV);
        //The following signals will be catched and handled.
        signals.add(SIGINT);
        signals.add(SIGTERM);
        //Start an asynchronous wait for one of the signals to occur.
        signals.async_wait(boost::bind(&TCP_Server::SignalsHandler, this,
                                       boost::asio::placeholders::error));
        //Start an asynchronous wait for a client request.
        this->StartAccept();
    }

    TCP_Server::~TCP_Server(void)
    {
        std::cout << "[DEBUG] Server destroyed" << std::endl;
    }
#pragma endregion

#pragma region Private members:
    void TCP_Server::StartAccept(void)
    {
        acceptorServer.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                //For each new incoming request, a thread will be created.
                //The created thread  will handle the connection.
                //N.B.: at the same time the server will be able to manage only a number of connections equal to MAX_ALLOWED_THREAD.
                //All excess requests will be discarded until some of the active connections are terminated.
                const size_t numberOfActiveConnections = TCP_Connection::GetActiveConnectionsNumber();
                if (numberOfActiveConnections < MAX_ALLOWED_THREAD)
                {
                    if (!ec)
                    {
                        //Create a new connection for the incoming client and execute it into a different thread
                        TCP_Connection::pointer new_connection = TCP_Connection::Create(std::move(socket),
                                                                                        this->nextConnectionId);
                        this->nextConnectionId += 1;
                        std::thread t(&TCP_Connection::Start, new_connection);
                        t.detach();
                    }
                }
                //Call StartAccept() to initiate the next accept operation.
                this->StartAccept();
            }
        );
    }

    void TCP_Server::SignalsHandler(const boost::system::error_code& errorCode)
    {
        auto connections = TCP_Connection::GetConnectionsMap();
        if (connections.empty() == false)
        {
            //Close alla connections before exit.
            for (auto& connection_it : connections)
            {
                if (connection_it.second != nullptr)
                    connection_it.second->Close();
            }
        }

        if (!errorCode)
        {
            //Exit.
            exit(0);
        }
        exit(-1);
    }
#pragma endregion
