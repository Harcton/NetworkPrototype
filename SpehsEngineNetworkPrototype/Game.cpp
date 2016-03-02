#include "Game.h"
#include <iostream>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

Game::Game()
{
}


Game::~Game()
{
}




void print(const boost::system::error_code& e)
{
	std::cout << "Hello, world!" << std::endl;
}
void Game::run()
{
	boost::asio::io_service io;
	boost::asio::deadline_timer t(io, boost::posix_time::seconds(5));
	t.async_wait(&print);
	io.run();
}

