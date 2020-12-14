#include "client.h"
#include <iostream>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Provide a path to monitor" << std::endl;
        return EXIT_FAILURE;
    }

    Client client { "127.0.0.1", 1996, argv[1] };
    //Run a new client and wait for its termination
    client.Run();

    return EXIT_SUCCESS;
}
