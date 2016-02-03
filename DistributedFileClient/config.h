#pragma once
#include <boost/property_tree/ptree.hpp>
#include "server_config.h"
class config
{
public:
	config(const std::string& path);
	~config();

	std::string getLogFileFullPath() { return logFileFullPath; }
	std::string getNewTransactionId();
	server_config getServerConfig(int index) { return servers[index]; }
	int getServersCount() { return servers.size(); }
private:
	static const std::string TRANSACTION_ID_SEPARATOR;
	std::string logFileFullPath;
	std::string id;
	std::string macAddress;
	std::vector<server_config> servers;

	std::string getMAC(std::string networkCardName);
	void setServers(boost::property_tree::ptree& tree);
	void check();
};

