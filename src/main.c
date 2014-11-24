#include "server.h"

const char* IP_ADDR = "127.0.0.1";
const int PORT_NUMBER = 8080;
const int MAX_BACKLOG = 200;

int main()
{
    run_server(IP_ADDR, PORT_NUMBER, MAX_BACKLOG);
    
    return 0;
}
