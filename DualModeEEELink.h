#ifndef DUALMODEEEELINK_H
#define DUALMODEEEELINK_H

#include <iostream>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cmath>
#include <climits>
#include <cassert>

#include "SleepLink.h"
#include "cpp.h"

class DualModeEEELink : public SleepLink
{
private:
	table * tblActive;                              // Time in Active table
	table * tblFastWake;                            // Time in Fast Wake table
	table * tblDeepSleep;                           // Time in Deep Sleep table
	table * tblAtoF;                                // Time in AtoF table
	table * tblFtoD;                                // Time in FtoD table
	table * tblDtoA;                                // Time in DtoA table
	table * tblFtoA;                                // Time in FtoA table
	table * tblAtoD;                                // Time in AtoD table
	table * tblDelay;                               // Packet delay table
	table * tblQWait;								// Packet waiting time in the queue table

	enum LinkModes {Active, FastWake, DeepSleep, TransAtoF, TransFtoD, TransDtoA, TransFtoA, TransAtoD};
	LinkModes curMode;                                  // Maintains link state at any time
	double   stateChange;                               // Time that the state changed to the current

	bool     DFlag;                                     // True if the link goes to DeepSleep after serving all the buffered pkts

	event    * evtLinkActive;                           // Triggered when the link becomes active
	event    * B_event;                                 // Triggered when a burst is complete

	double   P_Active, P_FastWake, P_DeepSleep, P_AtoF, P_FtoD, P_DtoA, P_FtoA, P_AtoD;
	double   T_AtoF, T_FtoD, T_DtoA, T_FtoA, T_AtoD;
				 
	double   T_Idle;
	unsigned int S_Coal;                    	        // Max # of pkts in a burst
	double   T_Coal;                                    // Max time for a burst
	unsigned int S_Lim;                                 // If pkts in a burst greater than this, go to Fast Wake after serving them 

private:
	bool chkCondDSFirstArrival(Packet * pkt);
	bool chkCondDSBurstFull(Packet * pkt);
	bool chkCondBufferEmpty(Packet * pkt);
	void putToDSFromFW();
	void putToDSFromA();
	void putToFWFromA();
	void stayInFW(double tm);
	void wakeupFromFW();
	void wakeupFromDS(Packet * pkt);

public:
	DualModeEEELink (std::string name, double cap);
	~DualModeEEELink ();

	std::string getName() const { return (my_name); }

	void setCoalParams (double t_idle, double t_coal, unsigned int s_coal, unsigned int s_lim);

	void connectLink (DualModeEEELink * conLink);
	void processPacket(Packet * pkt);
	void report();
};

#endif