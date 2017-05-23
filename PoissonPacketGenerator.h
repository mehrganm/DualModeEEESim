#ifndef POISSONPACKETGENERATOR_H
#define POISSONPACKETGENERATOR_H

#include "PacketGenerator.h"

class PoissonPacketGenerator : public PacketGenerator
{
private:
	unsigned long int start_sq, curr_sq;
	double lambda;
public:
	PoissonPacketGenerator (std::string name, double pktsPerSec, unsigned long int startSq) : 
				  PacketGenerator(name), start_sq(startSq), lambda(pktsPerSec)
	{
		running = false;
	}
	void run();
};

#endif