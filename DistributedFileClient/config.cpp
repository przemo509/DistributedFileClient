#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/log/trivial.hpp>

// MAC Address - Windows only
#include <windows.h>
#include <iphlpapi.h>

#include "config.h"

using namespace boost::property_tree;

const std::string config::TRANSACTION_ID_SEPARATOR = "|";

config::config(const std::string& pathToFile)
{
	BOOST_LOG_TRIVIAL(info) << "Reading client configuration file [" << pathToFile << "]:";
	ptree tree;
	read_json(pathToFile, tree);

	logFileFullPath = tree.get<std::string>("log_file_full_path");
	BOOST_LOG_TRIVIAL(info) << " - log file is            " << logFileFullPath;
	id = tree.get<std::string>("id");
	BOOST_LOG_TRIVIAL(info) << " - client id is           " << id;
	macAddress = getMAC("Intel(R) Centrino(R) Wireless-N 2230");
	BOOST_LOG_TRIVIAL(info) << " - client mac address is  " << macAddress;

	setServers(tree.get_child("servers"));

	check();
	BOOST_LOG_TRIVIAL(info) << "Reading server configuration file - OK";
}

std::string config::getMAC(std::string networkCardName) {
	IP_ADAPTER_INFO *info = NULL, *pos;
	DWORD size = 0;

	GetAdaptersInfo(info, &size); // calculate size for malloc
	info = (IP_ADAPTER_INFO *)malloc(size);
	GetAdaptersInfo(info, &size); // get actual info

	for (pos = info; pos != NULL; pos = pos->Next) {
		BOOST_LOG_TRIVIAL(trace) << "Checking network card: [" << pos->Description << "]";
		if (strcmp(networkCardName.c_str(), pos->Description) == 0) {
			char mac[18];
			sprintf_s(mac, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x", pos->Address[0], pos->Address[1], pos->Address[2], pos->Address[3], pos->Address[4], pos->Address[5]);
			free(info);
			return std::string(mac);
		}
	}

	free(info);
	throw std::exception(("Network card [" + networkCardName + "] not found!").c_str());
}

void config::setServers(ptree& serversJsonArray) {
	for (auto& item : serversJsonArray) {
		server_config serverConfig(
			item.second.get<int>("ID"),
			item.second.get<std::string>("hostname"),
			item.second.get<int>("port"));
		servers.push_back(serverConfig);
	}
}

void config::check() {
	// log file
	if (!boost::filesystem::exists(logFileFullPath)) {
		throw std::exception(("Log file [" + logFileFullPath + "] does not exist!").c_str());
	}
	if (!boost::filesystem::is_regular_file(logFileFullPath)) {
		throw std::exception(("Path [" + logFileFullPath + "] is not a file!").c_str());
	}
}

std::string config::getNewTransactionId() {
	return macAddress + TRANSACTION_ID_SEPARATOR + id + TRANSACTION_ID_SEPARATOR + std::to_string(std::time(NULL));
}


config::~config()
{
}
