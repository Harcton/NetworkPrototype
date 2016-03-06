#pragma once
#include <string>

class UDPClient
{
public:
	UDPClient();
	~UDPClient();
	void run(std::string str);
};

