#include "server_config.h"
#include <string>


server_config::server_config(int id, std::string hostname, int port):id(id), hostname(hostname), port(port)
{
}


server_config::~server_config()
{
}
