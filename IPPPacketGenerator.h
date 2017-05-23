#ifndef IPPPACKETGENERATOR_H
#define IPPPACKETGENERATOR_H

#include <cmath>
#include <climits>
#include "PacketGenerator.h"

class IPPPacketGenerator : public PacketGenerator
{
private:
	double lambda, alpha, beta;
	unsigned long int start_sq, curr_sq;
	double expon(double x);                          // Returns an exponential random variable
	double rand_val(int seed);                       // Jain's RNG
	int rand_seed;
public:
	IPPPacketGenerator (std::string name, double pktsPerSec, double IPPAlpha, double IPPBeta, unsigned long int startSq) : 
				  PacketGenerator(name), lambda(pktsPerSec), alpha(IPPAlpha), beta(IPPBeta), start_sq(startSq)
	{
		running = false;
		rand_seed = 2;
	}
	void run();
};

#endif