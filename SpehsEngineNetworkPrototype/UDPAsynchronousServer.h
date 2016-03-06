#pragma once
#include <string>

class UDPAsynchronousServer
{
public:
	UDPAsynchronousServer();
	~UDPAsynchronousServer();

	void run(std::string str);
};

