#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "controller.h"
#include "config.h" // has to be last http://stackoverflow.com/a/9750437

using namespace std;

int main(int argc, char* argv[])
{
	try
	{
		// global logging filter
		boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);

		if (argc != 2) {
			cout << "Usage: " << argv[0] << " config_file_full_path" << endl;
			exit(EXIT_FAILURE);
		}
		string configFileFullPath = argv[1];
		config config(configFileFullPath);

		controller controller(config);

		controller.runControlLoop();
	}
	catch (exception& e)
	{
		BOOST_LOG_TRIVIAL(fatal) << "Exception: [" << e.what() << "]";
	}
	catch (...)
	{
		BOOST_LOG_TRIVIAL(fatal) << "Unknown exception";
	}

	system("pause");
	return 0;
}
