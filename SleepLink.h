#ifndef SLEEPLINK_H
#define SLEEPLINK_H

#include <string>

#include "cpp.h"
#include "Packet.h"

class SleepLink
{
protected:
	const std::string my_name;

	facility * Server;                                  // CSIM Server facility
	double   capacity;
	
	SleepLink * nextlink;

	SleepLink(std::string name, double cap) : my_name(name), capacity (cap)
	{ 
		nextlink = NULL;
		Server = new facility("Server");
	}
public:
	std::string getName() { return (my_name); }

	virtual void processPacket(Packet * pkt) = 0;
	virtual void report() = 0;	
};

#endif