#include "client.h"

int main()
{
    Client client { "127.0.0.1", 1996 };

    //Run a new client and wait for its termination
    client.Run();

    return EXIT_SUCCESS;
}
