#ifndef PACKETGENERATOR_H
#define PACKETGENERATOR_H

#include "DualModeEEELink.h"

class PacketGenerator
{
protected:
	const std::string my_name;
	bool running;
	DualModeEEELink * link;

	PacketGenerator(std::string name) : my_name(name) { }
public:
	void connectLink (DualModeEEELink * conLink)
	{
		link = conLink;
	}
	virtual void run() = 0;
	void stop()
	{
		running = false;
	}
};

#endif