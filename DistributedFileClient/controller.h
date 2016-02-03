#pragma once

#include <string>
#include <set>
#include "config.h"

class controller
{
public:
	controller(config& config);
	~controller();
	void runControlLoop();
private:
	static const std::string PROMPT;
	static const std::string QUIT;
	static const std::string BEGIN_TRANSACTION;

	config cfg;
	std::string transactionId;
	std::set<int> serversInTransaction;

	std::string getUserInput();
	void handleQuitOrBt();
	void handleQuit();
	void handleBt();
	void handleRead();
	void handleWrite();
	server_config getServerFromUser();
	bool isNumber(std::string s);
	std::string getFileNameFromUser();
	void handlePrepare();
	void clearTransaction();
};

