#include "PoissonPacketGenerator.h"

void PoissonPacketGenerator::run()
{
	curr_sq = start_sq;
	running = true;
	Packet * newPacket;

	create(my_name.c_str());

	printf ("%.3f: %s: Packet %ld generated\n", clock*1.0e+6, my_name.c_str(), curr_sq);
	newPacket = new Packet(clock, curr_sq, 12000, 0, my_name + std::string(":") + std::to_string(curr_sq));
	curr_sq++;
	link->processPacket(newPacket);

	while (running)
	{
		hold(exponential(1.0 / lambda));
		printf ("%.3f: %s: Packet %ld generated\n", clock*1.0e+6, my_name.c_str(), curr_sq);
		newPacket = new Packet(clock, curr_sq, 12000, 0, my_name + std::string(":") + std::to_string(curr_sq));
		link->processPacket(newPacket);
		curr_sq++;
	}
}
