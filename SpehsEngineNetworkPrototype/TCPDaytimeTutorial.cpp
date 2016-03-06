#include "TCPDaytimeTutorial.h"
#include <iostream>
#include <ctime>
#include <boost/array.hpp>


TCPDaytimeTutorial::TCPDaytimeTutorial()
{
}


TCPDaytimeTutorial::~TCPDaytimeTutorial()
{
}


//// TUTORIAL 1
void TCPDaytimeTutorial::tutorial1(std::string serverName)
{
	try
	{
		boost::asio::io_service ioService;
		boost::asio::ip::tcp::resolver resolver(ioService);
		boost::asio::ip::tcp::resolver::query query(serverName, "TCPDaytimeTutorial1"/*Service name*/);
		boost::asio::ip::tcp::resolver::iterator endpointIterator = resolver.resolve(query);
		boost::asio::ip::tcp::socket socket(ioService);
		boost::asio::connect(socket, endpointIterator);

		while (true)
		{
			boost::array<char, 128> buffer;
			boost::system::error_code error;
			size_t length = socket.read_some(boost::asio::buffer(buffer), error);

			if (error == boost::asio::error::eof)
				break;//Connection closed cleanly by peer
			else if (error)
				throw boost::system::system_error(error);//Some other error
			std::cout.write(buffer.data(), length);
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "\n" << e.what();
	}
}


//// TUTORIAL 2
void TCPDaytimeTutorial::tutorial2(std::string serverName)
{
	try
	{
		boost::asio::io_service ioService;
		boost::asio::ip::tcp::acceptor acceptor(ioService,
			boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 13));

		std::string message = makeDaytimeString();
		boost::system::error_code ignoredError;
		boost::asio::write(socket, boost::asio::buffer(message), ignoredError);
	}
	catch (std::exception& e)
	{
		std::cerr << "\n" << e.what();
	}
}
std::string makeDaytimeString()
{
	std::time_t now = std::time(0);
	return std::ctime(&now);
}


//// TUTORIAL 3
void TCPDaytimeTutorial::tutorial3(std::string serverName)
{
	try
	{
		/*We need to create a server object to accept incoming client connections.
		The io_service object provides I/O services,
		such as sockets, that the server object will use. */
		boost::asio::io_service ioService;
		TCPServer server(ioService);
		ioService.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "\n" << e.what();
	}
}