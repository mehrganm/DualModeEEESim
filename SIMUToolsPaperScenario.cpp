#include <iostream>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <climits>
#include <cassert>
#include <string>

#include "cpp.h"

#include "Packet.h"
#include "PoissonPacketGenerator.h"
#include "DualModeEEELink.h"

extern "C" void sim(int, char **);

void sim(int argc, char *argv[])
{
	const double SIM_TIME = 1.00e+0;
	double   lambda = 15.0;                                 // Mean packet arrival rate (pkts/s)

	create("sim");
	max_processes (10000000);

	PoissonPacketGenerator gen1("Pkt Generator 1", lambda, 0L);
	DualModeEEELink * link1 = new DualModeEEELink("Dual EEE Link 1", 40.0e+9);
	link1->setCoalParams(3.50e-6, 10.0e-6, 12 , 6);
	gen1.connectLink(link1);

	PoissonPacketGenerator gen2("Pkt Generator 2", lambda, 0L);
	DualModeEEELink * link2 = new DualModeEEELink("Dual EEE Link 2", 40.0e+9);
	link2->setCoalParams(3.50e-6, 10.0e-6, 12 , 6);
	gen2.connectLink(link2);

	DualModeEEELink * link_lev2 = new DualModeEEELink("Dual EEE Link Level 2", 40.0e+9);
	link_lev2->setCoalParams(3.50e-6, 10.0e-6, 12 , 6);
	link1->connectLink(link_lev2);
	link2->connectLink(link_lev2);

	gen1.run();
	gen2.run();

	event evtDone("done");
	evtDone.timed_wait(SIM_TIME);

	gen1.stop();
	gen2.stop();
	
	link1->report();
	link2->report();
	link_lev2->report();
}