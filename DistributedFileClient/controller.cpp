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
	ptree tree = fromString(response);
	string fileContent = tree.get<string>(DATA);
	cout << "File content is: " << fileContent << endl;
}

void controller::handleWrite() {
	server_config serverConfig = getServerFromUser();
	string fileName = getFileNameFromUser();
	cout << "File content:" << endl;
	string fileContent = getUserInput();

	string request = makeWriteMessage(transactionId, fileName, fileContent);
	string response = getResponse(serverConfig, request);
	ptree tree = fromString(response);
	string name = tree.get<string>(NAME);
	cout << "File write response: " << name << endl;
}

server_config controller::getServerFromUser() {
	while (true) {
		cout << "Server ID:" << endl;
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
		cout << "File name:" << endl;
		string userInput = getUserInput();
		if (userInput.empty()) {
			cout << "Empty file name is not accepted" << endl;
			continue;
		}
		return userInput;
	}
}

void controller::handlePrepare() {
	bool abort = false;
	for (auto id : serversInTransaction) {
		server_config serverConfig = cfg.getServerConfig(id);
		string request = makePrepareMessage(transactionId);
		string response = getResponse(serverConfig, request);
		ptree tree = fromString(response);
		string name = tree.get<string>(NAME);
		cout << "Server " << id << " responded to prepare:" << endl << name << endl;
		if (name == MSG_VOTE_ABORT) {
			abort = true;
		}
	}

	cout << "Sending global " << (abort ? "abort" : "commit") << " to all servers in transaction..." << endl;
	string request = abort ? makeGlobalAbortMessage(transactionId) : makeGlobalCommitMessage(transactionId);
	for (auto id : serversInTransaction) {
		server_config serverConfig = cfg.getServerConfig(id);
		sendWithoutResponse(serverConfig, request);
	}
	cout << "Global " << (abort ? "abort" : "commit") << " sent." << endl;
}

void controller::clearTransaction() {
	transactionId = "";
	serversInTransaction.clear();
}