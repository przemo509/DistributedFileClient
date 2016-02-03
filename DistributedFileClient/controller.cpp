#include <iostream>
#include <string>
#include <set>

#include "network.h"
#include "json_messages.h"
#include "config.h"
#include "controller.h"

using namespace std;

const string controller::PROMPT = "> ";
const string controller::QUIT = "q";
const string controller::BEGIN_TRANSACTION = "bt";


controller::controller(config& config):cfg(config)
{
}

controller::~controller()
{
}

string controller::getUserInput() {
	cout << PROMPT;
	string userInput;
	getline(cin, userInput);
	return userInput;
}

void controller::runControlLoop() {
	handleQuitOrBt();
}

void controller::handleQuitOrBt() {
	while (true) {
		cout << "Allowed operations are: '" << BEGIN_TRANSACTION << "', '" << QUIT << "'." << endl;
		string userInput = getUserInput();

		if (userInput == QUIT) {
			handleQuit();
		} else if (userInput == BEGIN_TRANSACTION) {
			handleBt();
		} else {
			cout << "Not allowed operation: " << userInput << endl;
		}
	}
}

void controller::handleQuit() {
	cout << "Good bye." << endl;
	system("pause");
	exit(EXIT_SUCCESS);
}

void controller::handleBt() {
	transactionId = cfg.getNewTransactionId();
	cout << "Started new transaction: [" << transactionId << "]." << endl;

	while (true) {
		cout << "Allowed operations are: '" << MSG_READ << "', '" << MSG_WRITE << "', '" << MSG_PREPARE << "'." << endl;
		string userInput = getUserInput();

		if (userInput == MSG_READ) {
			handleRead();
		} else if (userInput == MSG_WRITE) {
			handleWrite();
		} else if (userInput == MSG_PREPARE) {
			handlePrepare();
			clearTransaction();
			break;
		} else {
			cout << "Not allowed operation: " << userInput << endl;
		}
	}
}

void controller::handleRead() {
	server_config serverConfig = getServerFromUser();
	string fileName = getFileNameFromUser();

	string request = makeReadMessage(transactionId, fileName);
	string response = getResponse(serverConfig, request);
	cout << "File content is:" << endl << response << endl;
}

void controller::handleWrite() {
	server_config serverConfig = getServerFromUser();
	string fileName = getFileNameFromUser();
	cout << "Type file content:" << endl;
	string fileContent = getUserInput();

	string request = makeWriteMessage(transactionId, fileName, fileContent);
	string response = getResponse(serverConfig, request);
	cout << "File write response:" << endl << response << endl;
}

server_config controller::getServerFromUser() {
	while (true) {
		cout << "Type server ID:" << endl;
		string userInput = getUserInput();
		if (!isNumber(userInput)) {
			cout << "That is not a number" << endl;
			continue;
		}
		int id = stoi(userInput);
		try {
			server_config config = cfg.getServerConfig(id);
			serversInTransaction.insert(id);
			return config;
		} catch (exception e) {
			cout << e.what() << endl;
			continue;
		}
	}
}

bool controller::isNumber(string s) {
	for (unsigned int i = 0; i < s.size(); ++i) {
		if (!isdigit(s[i])) {
			return false;
		}
	}
	return !s.empty();
}

string controller::getFileNameFromUser() {
	while (true) {
		cout << "Type file name:" << endl;
		string userInput = getUserInput();
		if (userInput.empty()) {
			cout << "Empty file name is not accepted" << endl;
			continue;
		}
		return userInput;
	}
}

void controller::handlePrepare() {
	for (auto id : serversInTransaction) {
		server_config serverConfig = cfg.getServerConfig(id);

		string request = makePrepareMessage(transactionId);
		string response = getResponse(serverConfig, request);
		cout << "Server " << id << " responded to prepare:" << endl << response << endl;
	}
}

void controller::clearTransaction() {
	transactionId = "";
	serversInTransaction.clear();
}