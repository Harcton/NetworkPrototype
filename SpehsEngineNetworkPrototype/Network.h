#pragma once
#include <stdint.h>
#define SERVER_SIZE 5
#define PORT_NUMBER 41624
#define LOG_NETWORK true
#define UDP_DATAGRAM_MAX 16//65527
//41624
//13
namespace packet
{
	/**Packet type:
	In most cases, if packet comes from client it defines the query type.
	Correspondingly a packet of certain type from server represents a response to given query type.*/
	enum PacketType : unsigned char
	{
		invalid = 0,
		enter,
		exit,
		update,
	};
}


/**Structure for containing all player state data*/
struct PlayerStateData
{
	PlayerStateData() : type(packet::update){}
	unsigned char type;
	int16_t ID;

	//Input data
	int16_t mouseX;
	int16_t mouseY;
};
struct ObjectData
{
	int16_t ID;
	int16_t x;
	int16_t y;
};