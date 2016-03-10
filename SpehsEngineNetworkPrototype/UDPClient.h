#pragma once
#include <string>
namespace boost
{
	namespace system
	{
		struct error_code;
	}
}

class UDPClient
{
public:
	UDPClient();
	~UDPClient();
	void run(std::string str);
	void connectHandler(const boost::system::error_code& error);
};

