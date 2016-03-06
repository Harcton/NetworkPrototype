#pragma once
#include <string>

class UDPSynchronousServer
{
public:
	UDPSynchronousServer();
	~UDPSynchronousServer();
	void run(std::string str);
};

