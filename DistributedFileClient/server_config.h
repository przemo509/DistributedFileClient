#pragma once
#include <string>

class server_config
{
public:
	server_config(int id, std::string hostname, int port);
	~server_config();
	int getId() { return id; }
	std::string getHostname() { return hostname; }
	int getPort() { return port; }
private:
	int id;
	std::string hostname;
	int port;
};

