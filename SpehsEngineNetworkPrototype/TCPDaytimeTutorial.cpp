#include "TCPDaytimeTutorial.h"
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>


TCPDaytimeTutorial::TCPDaytimeTutorial()
{
}


TCPDaytimeTutorial::~TCPDaytimeTutorial()
{
}


//// TUTORIAL 1
void TCPDaytimeTutorial::tutorial1(std::string serverName)
{
	boost::asio::io_service ioService;
	boost::asio::ip::tcp::resolver resolver(ioService);
	boost::asio::ip::tcp::resolver::query query(serverName, "daytime"/*Service name*/);
	boost::asio::ip::tcp::resolver::iterator endpointIterator = resolver.resolve(query);
	boost::asio::ip::tcp::socket socket(ioService);
	boost::asio::connect(socket, endpointIterator);

	while (true)
	{
		boost::array<char, 128> buffer;

	}
}


//// TUTORIAL 2
void TCPDaytimeTutorial::tutorial2(std::string serverName)
{

}


//// TUTORIAL 3
void TCPDaytimeTutorial::tutorial3(std::string serverName)
{

}