#include <iostream>
#include <boost/array.hpp>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <boost/asio.hpp> // must be after logging (otherwise you get linker error, don't know why)

using namespace boost::asio;

static const int HEADER_LENGTH = 5;

void assertResponseOk(size_t bytesReceived, int assumedLength, boost::system::error_code& errorCode) {
	if (bytesReceived != assumedLength) {
		throw new std::exception(std::string("Exception while reading response: unexpected length: " + bytesReceived).c_str());
	}
	if (errorCode != 0) {
		// TODO handle eof when server gets down
		throw new std::exception(("Exception while reading response: error: " + errorCode.message()).c_str());
	}
}

std::string getBufAsString(streambuf& buf) {
	std::ostringstream oss;
	oss << &buf;
	return oss.str();
}

int getResponseLength(streambuf& buf) {
	std::string header = getBufAsString(buf);
	int responseLength = atoi(header.c_str());
	return responseLength;
}

std::string makeHeader(int bodySize) {
	char h[HEADER_LENGTH + 1];
	sprintf_s(h, ("%0" + std::to_string(HEADER_LENGTH) + "d").c_str(), bodySize);
	return h;
}

std::string makeRequest() {
	std::string req = "{\"name\": \"read\", \"transaction_id\": \"dc:85:de:57:7c:7a|1|1454451466.18\", \"filename\": \"a\"}";
	return makeHeader(req.size()) + req;
}

int main()
{
	// global logging filter
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);

	try
	{
		// preparing new socket
		io_service io_service;
		ip::tcp::socket socket(io_service);
		socket.connect(ip::tcp::endpoint(ip::address_v4::from_string("127.0.0.1"), 8183));

		// sending request
		std::string request = makeRequest();
		BOOST_LOG_TRIVIAL(debug) << "Sending request:\n" << request;
		write(socket, buffer(request));
		BOOST_LOG_TRIVIAL(debug) << "Request sent";

		// receiving response header (number of remaining bytes)
		boost::system::error_code errorCode;
		streambuf buf;
		BOOST_LOG_TRIVIAL(debug) << "Waiting for response length header...";
		size_t bytesReceived = read(socket, buf, detail::transfer_exactly_t(HEADER_LENGTH), errorCode);
		assertResponseOk(bytesReceived, HEADER_LENGTH, errorCode);
		int responseLength = getResponseLength(buf);
		//buf.consume(bytesReceived); // clear buf
		BOOST_LOG_TRIVIAL(debug) << "Response length header received: " << responseLength;

		// receiving response body
		BOOST_LOG_TRIVIAL(debug) << "Waiting for response body...";
		bytesReceived = read(socket, buf, detail::transfer_exactly_t(responseLength), errorCode);
		assertResponseOk(bytesReceived, responseLength, errorCode);
		std::string response = getBufAsString(buf);
		BOOST_LOG_TRIVIAL(debug) << "Response body received:\n" << response;
	}
	catch (std::exception& e)
	{
		BOOST_LOG_TRIVIAL(fatal) << "Exception: [" << e.what() << "]";
	}

	system("pause");
	return 0;
}
