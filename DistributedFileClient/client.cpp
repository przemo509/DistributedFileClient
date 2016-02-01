#include <iostream>
#include <boost/array.hpp>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <boost/asio.hpp> // must be ater logging (otherwise you get linker error, don't know why)

static const std::string DELIMETER = "\n";

int main()
{
	// global logging filter
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);

	try
	{
		// preparing socket
		using namespace boost::asio;
		io_service io_service;
		ip::tcp::socket socket(io_service);
		socket.connect(ip::tcp::endpoint(ip::address_v4::from_string("127.0.0.1"), 12345));

		// sending request
		std::string request("test request");
		write(socket, buffer(request + DELIMETER));

		// receiving response
		boost::system::error_code errorCode;
		boost::asio::streambuf buf;
		size_t bytesReceived = read_until(socket, buf, DELIMETER, errorCode);
		if (errorCode != 0) {
			// TODO handle eof when server gets down
			BOOST_LOG_TRIVIAL(fatal) << "Exception: [" << errorCode.message() << "]";
		}
		std::string response;
		std::getline(std::istream(&buf), response);
		BOOST_LOG_TRIVIAL(debug) << "Response received: " << response;
	}
	catch (std::exception& e)
	{
		BOOST_LOG_TRIVIAL(fatal) << "Exception: [" << e.what() << "]";
	}

	system("pause");
	return 0;
}