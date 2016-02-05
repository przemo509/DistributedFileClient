#pragma once

#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/asio.hpp> // must be after logging (otherwise you get linker error, don't know why)

#include "server_config.h"

using namespace boost::asio;
using namespace std;

static const int HEADER_LENGTH = 5;

void assertResponseOk(size_t bytesReceived, int assumedLength, boost::system::error_code& errorCode) {
	if (errorCode != 0) {
		// TODO handle eof when server gets down
		throw exception(("Exception while reading response: error: " + errorCode.message()).c_str());
	}
	if (bytesReceived != assumedLength) {
		throw exception(string("Exception while reading response: unexpected length: " + bytesReceived).c_str());
	}
}

string getBufAsString(boost::asio::streambuf& buf) {
	ostringstream oss;
	oss << &buf;
	return oss.str();
}

int getResponseLength(boost::asio::streambuf& buf) {
	string header = getBufAsString(buf);
	int responseLength = stoi(header.c_str());
	return responseLength;
}

string makeHeader(int bodySize) {
	char h[HEADER_LENGTH + 1];
	sprintf_s(h, ("%0" + to_string(HEADER_LENGTH) + "d").c_str(), bodySize);
	return h;
}

string addHeader(string& request) {
	return makeHeader(request.size()) + request;
}

string getResponse(server_config& config, string request) {
	// preparing new socket
	io_service io_service;
	ip::tcp::socket socket(io_service);
	socket.connect(ip::tcp::endpoint(ip::address_v4::from_string(config.getHostname()), config.getPort()));

	// sending request
	string requestWithHeader = addHeader(request);
	BOOST_LOG_TRIVIAL(debug) << "Sending request:\n" << requestWithHeader;
	write(socket, buffer(requestWithHeader));
	BOOST_LOG_TRIVIAL(debug) << "Request sent";

	// receiving response header (number of remaining bytes)
	boost::system::error_code errorCode;
	boost::asio::streambuf buf;
	BOOST_LOG_TRIVIAL(debug) << "Waiting for response length header...";
	size_t bytesReceived = read(socket, buf, boost::asio::detail::transfer_exactly_t(HEADER_LENGTH), errorCode);
	assertResponseOk(bytesReceived, HEADER_LENGTH, errorCode);
	int responseLength = getResponseLength(buf);
	//buf.consume(bytesReceived); // clear buf
	BOOST_LOG_TRIVIAL(debug) << "Response length header received: " << responseLength;

	// receiving response body
	BOOST_LOG_TRIVIAL(debug) << "Waiting for response body...";
	bytesReceived = read(socket, buf, boost::asio::detail::transfer_exactly_t(responseLength), errorCode);
	assertResponseOk(bytesReceived, responseLength, errorCode);
	string response = getBufAsString(buf);
	BOOST_LOG_TRIVIAL(debug) << "Response body received:\n" << response;

	return response;
}
