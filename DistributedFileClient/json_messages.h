#pragma once

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::property_tree::ptree;
using namespace std;

static const string NAME = "name";
static const string TRANSACTION_ID = "transaction_id";
static const string FILENAME = "filename";
static const string DATA = "data";

static const string MSG_WRITE = "write";
static const string MSG_WRITE_RESPONSE = "write_response";
static const string MSG_READ = "read";
static const string MSG_READ_RESPONSE = "read_response";
static const string MSG_PREPARE = "prepare";
static const string MSG_VOTE_ABORT = "vote_abort";
static const string MSG_VOTE_COMMIT = "vote_commit";
static const string MSG_GLOBAL_ABORT = "global_abort";
static const string MSG_GLOBAL_COMMIT = "global_commit";

string toString(ptree& tree) {
	stringstream ss;
	write_json(ss, tree);
	return ss.str();
}

string makeReadMessage(string transactionId, string filename) {
	ptree tree;

	tree.put(NAME, MSG_READ);
	tree.put(TRANSACTION_ID, transactionId);
	tree.put(FILENAME, filename);

	return toString(tree);
}

string makeWriteMessage(string transactionId, string filename, string fileContent) {
	ptree tree;

	tree.put(NAME, MSG_WRITE);
	tree.put(TRANSACTION_ID, transactionId);
	tree.put(FILENAME, filename);
	tree.put(DATA, fileContent);

	return toString(tree);
}

string makePrepareMessage(string transactionId) {
	ptree tree;

	tree.put(NAME, MSG_PREPARE);
	tree.put(TRANSACTION_ID, transactionId);

	return toString(tree);
}