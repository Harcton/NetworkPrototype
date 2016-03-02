#include "Game.h"
#include <iostream>
#include <boost/asio.hpp>
//#include <boost/thread/thread.hpp> //??? compiler internal error
#include <thread>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

Game::Game()
{
}
Game::~Game()
{
}


//// TIMER TUTORIAL 5 - Synchronising handlers in multithreaded programs
class PrinterExtended
{
public:
	PrinterExtended(boost::asio::io_service& io) :
		strand(io),
		timer1(io, boost::posix_time::seconds(1)),
		timer2(io, boost::posix_time::seconds(1)),
		count(0)
	{
		timer1.async_wait(strand.wrap(boost::bind(&PrinterExtended::print1, this)));
		timer2.async_wait(strand.wrap(boost::bind(&PrinterExtended::print2, this)));
	}
	~PrinterExtended()
	{
		std::cout << "\nThe final count is: " << count;
	}
	void print1()
	{
		if (count < 10)
		{
			std::cout << "\nTimer 1: " << count;
			++count;
			timer1.expires_at(timer1.expires_at() + boost::posix_time::seconds(1));
			timer1.async_wait(strand.wrap(boost::bind(&PrinterExtended::print1, this)));
		}
	}
	void print2()
	{
		if (count < 10)
		{
			std::cout << "\nTimer 2: " << count;
			++count;
			timer2.expires_at(timer2.expires_at() + boost::posix_time::seconds(1));
			timer2.async_wait(strand.wrap(boost::bind(&PrinterExtended::print2, this)));
		}
	}

private:
	boost::asio::io_service::strand strand;
	boost::asio::deadline_timer timer1;
	boost::asio::deadline_timer timer2;
	int count;
};
void Game::asioTimerTutorial5()
{
	boost::asio::io_service ioService;
	PrinterExtended printer(ioService);

	//Running from multiple threads
	//boost::thread thread(boost::bind(&boost::asio::io_service::run, &ioService)); 
	std::thread thread(boost::bind(&boost::asio::io_service::run, &ioService));
	ioService.run();
	thread.join();
}


//// TIMER TUTORIAL 4 - Using a member function as a handler
class Printer
{
public:
	Printer(boost::asio::io_service& io) : timer(io, boost::posix_time::seconds(1)), count(0)
	{
		timer.async_wait(boost::bind(&Printer::print, this));
	}
	~Printer()
	{
		std::cout << "\nFinal count is " << count;
	}
	void print()
	{
		if (count < 5)
		{
			std::cout << "\ncount: " << count;
			++count;
			timer.expires_at(timer.expires_at() + boost::posix_time::seconds(1));
			timer.async_wait(boost::bind(&Printer::print, this));
		}
	}
private:
	boost::asio::deadline_timer timer;
	int count;
};
void Game::asioTimerTutorial4()
{
	boost::asio::io_service ioService;
	Printer printer(ioService);
	ioService.run();
}


//// TIMER TUTORIAL 3 - Binding arguments to a handler
void asioTimerTutorial3Print(const boost::system::error_code& e, boost::asio::deadline_timer* t, int* count)
{//Callback handler function. Expects function with the signature void(const boost::system::error_code&)

	if (*count < 5)
	{
		std::cout << "\n" << *count;
		++(*count);
		t->expires_at(t->expires_at() + boost::posix_time::seconds(1));
		t->async_wait(boost::bind(asioTimerTutorial3Print, boost::asio::placeholders::error, t, count));
	}
}
void Game::asioTimerTutorial3()
{
	int count = 0;

	boost::asio::io_service io;
	boost::asio::deadline_timer timer(io, boost::posix_time::seconds(1));

	timer.async_wait(boost::bind(asioTimerTutorial3Print, boost::asio::placeholders::error, &timer, &count));//Notice extra parameters bound using boost bind

	io.run();//Stops when there is no more work to do

	std::cout << "\nFinal count is " << count;
}