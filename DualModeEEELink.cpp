#include "DualModeEEELink.h"

DualModeEEELink::DualModeEEELink (std::string name, double cap) : SleepLink(name, cap)
{
	P_Active = 100.00;
	P_FastWake = 70.00;
	P_DeepSleep = 10.00;
	P_AtoF = P_FtoD = P_DtoA = P_FtoA = P_AtoD = 100.00;
	
	T_AtoF = 0.18e-6;
	T_FtoD = 0.72e-6;
	T_DtoA = 5.50e-6;
	T_FtoA = 0.34e-6;
	T_AtoD = 0.90e-6;

	curMode = Active;
	DFlag = true;
	
	evtLinkActive = new event ("The link is ACTIVE");
	B_event = new event ("A burst is complete");

	tblDelay = new table ("Packet delay table");
	tblQWait = new table ("Packet waiting time in the queue table");
	tblActive = new table("Time in Active");
	tblFastWake = new table ("Time in Fast Wake");
	tblDeepSleep = new table ("Time in Deep Sleep");
	tblAtoF = new table ("Time in AtoF");
	tblFtoD = new table ("Time in FtoD");
	tblDtoA = new table ("Time in DtoA");
	tblFtoA = new table ("Time in FtoA");
	tblAtoD = new table ("Time in AtoD");
	
	evtLinkActive->clear();
	B_event->clear();
}
DualModeEEELink::~DualModeEEELink ()
{
	delete Server;
	delete evtLinkActive;
}
void DualModeEEELink::connectLink (DualModeEEELink * conLink)
{
	nextlink = conLink;
}
void DualModeEEELink::setCoalParams (double t_idle, double t_coal, unsigned int s_coal, unsigned int s_lim)
{
	T_Idle   = t_idle;
	T_Coal   = t_coal;
	S_Coal   = s_coal;
	S_Lim    = s_lim;
}
bool DualModeEEELink::chkCondDSFirstArrival(Packet * pkt)
{
	int ret;
	
	if (B_event->event_qlen() == 0) // If the first packet in the burst, set a burst timer
	{
		printf ("%.3f: First Packet (seq %d) in the burst, timer set to %f us, expires at %.3f\n", clock*1.0e+6, pkt->getSeqNum(), T_Coal*1.0e+6, (clock + T_Coal)*1.0e+6);
		B_event->clear(); // If you don't clear it the event will be in the OCCURED state and the next wait will resume immediately
		ret = B_event->timed_wait(T_Coal); // Set the timer
			
		if (ret == TIMED_OUT) // If triggered due to timer expiration
		{
			if (B_event->event_qlen()+1 <= S_Lim)
				DFlag = true;
			else
				DFlag = false;

			printf ("%.3f: Burst timer expired, # of pkts in burst: %d, DFlag set to %s\n", clock*1.0e+6, B_event->event_qlen()+1, DFlag==true?"TRUE":"FALSE");
			B_event->set();   // Burst to queue
			wakeupFromDS(pkt);
		}
		return true;
	}
	else
		return false;
}

bool DualModeEEELink::chkCondDSBurstFull(Packet * pkt)
{
	//assert(curMode == DeepSleep);
	if (B_event->event_qlen() >= (S_Coal-1)) // If reached burst size
	{
		if (B_event->event_qlen()+1 <= S_Lim)
			DFlag = true;
		else
			DFlag = false;
		printf ("%.3f: Burst buffer full, # of pkts in burst: %d, DFlag set to %s \n", clock*1.0e+6, B_event->event_qlen()+1, DFlag==true?"TRUE":"FALSE");
		
		B_event->set();   // Burst to queue
		wakeupFromDS(pkt);
		return true;
	}
	else
		return false;
}

bool DualModeEEELink::chkCondBufferEmpty(Packet * pkt)
{
	assert(curMode == Active);
	if ( (Server->qlength() == 0) && (DFlag == true) )
	{
		printf ("%.3f: Link buffer empty. Link is going to Deep Sleep...\n", clock*1.0e+6);
		putToDSFromA();
		return true;
	}
	else if ( (Server->qlength() == 0) && (DFlag == false) )
	{
		printf ("%.3f: Link buffer empty. Link is going to Fast Wake...\n", clock*1.0e+6);
		putToFWFromA();
		stayInFW(T_Idle);

		if (B_event->event_qlen() == 0) // Still no packets in the buffer, go to DS directly
		{
			printf ("%.3f: Fast Wake Timeout, no pkts in the buffer, link is going to deep sleep... Spent %.3f us in FAST WAKE.\n", clock*1.0e+6, (clock - stateChange)*1.0e+6);
			putToDSFromFW();
		}
		else // Some pkts in the buffer while in FW, go to Active
		{
			if (B_event->event_qlen()+1 <= S_Lim)
				DFlag = true;
			else
				DFlag = false;

			printf ("%.3f: Fast Wake Timeout, # of pkts in burst: %d, DFlag set to %s \n", clock*1.0e+6, B_event->event_qlen(), DFlag==true?"TRUE":"FALSE");

			B_event->set();
			wakeupFromFW();
		}
		return true;
	}
	else
		return false;
}

void DualModeEEELink::putToDSFromFW()
{
	assert(curMode == FastWake);

	curMode = TransFtoD; stateChange = clock;
	hold(T_FtoD);
	tblFtoD->record(clock - stateChange);
	curMode = DeepSleep; stateChange = clock;
	
	printf ("%.3f: Link is in DEEP SLEEP state. %d packet(s) arrived while transitioning to DEEP SLEEP. \n", clock*1.0e+6, B_event->event_qlen());
}

void DualModeEEELink::putToDSFromA()
{
	assert(curMode == Active);

	create("putToDSFromA");
	tblActive->record(clock - stateChange);
	
	curMode = TransAtoD; stateChange = clock;
	hold(T_AtoD);
	tblAtoD->record(clock - stateChange);
	curMode = DeepSleep; stateChange = clock;
	
	printf ("%.3f: Link is in DEEP SLEEP state. %d packet(s) arrived while transitioning to DEEP SLEEP. \n", clock*1.0e+6, B_event->event_qlen());
}

void DualModeEEELink::putToFWFromA()
{
	tblActive->record(clock - stateChange);
	curMode = TransAtoF; stateChange = clock;
	hold(T_AtoF);
	tblAtoF->record(clock - stateChange);
	curMode = FastWake; stateChange = clock;
	printf ("%.3f: Link is in FAST WAKE. Will go to DEEP SLEEP at %.3f us if buffer empty by then. %d pkt(s) arrived while transitioning to FAST WAKE.\n", clock*1.0e+6, (clock + T_Idle)*1.0e+6, B_event->event_qlen());
}

void DualModeEEELink::stayInFW(double tm)
{
	assert(curMode == FastWake);
	hold (tm);	
	tblFastWake->record(clock - stateChange);
}

void DualModeEEELink::wakeupFromFW()
{
	assert(curMode == FastWake);
	
	create("wakeupFromFW");
	printf ("%.3f: Link is waking up from Fast Wake...\n", clock*1.0e+6);
	
	curMode = TransFtoA; stateChange = clock;
	hold(T_FtoA);
	tblFtoA->record(clock - stateChange);
	curMode = Active; stateChange = clock;
	printf ("%.3f: Link is in the ACTIVE state. \n", clock*1.0e+6);
	//printf ("%.3f: About to start serving packets starting at seq #%ld in this busy period\n", clock*1.0e+6, pkt->getSeqNum());
	evtLinkActive->set();		
}

void DualModeEEELink::wakeupFromDS(Packet * pkt)
{
	assert((curMode == TransAtoD) || (curMode == TransFtoD) || (curMode == DeepSleep) || (curMode == TransDtoA));
	
	if (curMode == TransDtoA)
	{
		printf ("%.3f: Link in transition DtoA. Waiting until it becomes active again.\n", clock*1.0e+6);
		return;
	}
	
	create("wakeupFromDS");
	tblDeepSleep->record(clock - stateChange);
	printf ("%.3f: Link is waking up from Deep Sleep, triggered by pkt %ld...\n", clock*1.0e+6, pkt->getSeqNum());
	curMode = TransDtoA; stateChange = clock;
	hold(T_DtoA);
	tblDtoA->record(clock - stateChange);
	curMode = Active; stateChange = clock;
	printf ("%.3f: Link is in the ACTIVE state. \n", clock*1.0e+6);
	evtLinkActive->set();
}

void DualModeEEELink::processPacket(Packet * pkt)
{
	char * temp_str = new char[pkt->getName().length()+1];
	pkt->getName().copy(temp_str,pkt->getName().length());
	temp_str[pkt->getName().length()] = '\0';
	create(temp_str);
	printf ("%.3f: %s: Pkt %ld entered the link \n", clock*1.0e+6, my_name.c_str(), pkt->getSeqNum());
	set_priority (ULONG_MAX - pkt->getSeqNum());
	
	if ( curMode == Active )
	{
	}
	else if ( (curMode == TransFtoA) || (curMode == TransDtoA) )
	{
		printf ("%.3f: Packet %ld arrived to a link in transition to ACTIVE, waiting... \n", clock*1.0e+6, pkt->getSeqNum());
		evtLinkActive->wait();
	}
	else if ( (curMode == FastWake) || (curMode == TransAtoF) )
	{
		printf ("%.3f: (seq %d) Waiting in the buffer with %d other pkts\n", clock*1.0e+6, pkt->getSeqNum(), B_event->wait_cnt());		
		B_event->wait(); // Add the pkt to burst buffer
		evtLinkActive->wait();
	}
	else if ( (curMode == DeepSleep) || (curMode == TransAtoD) || (curMode == TransFtoD))
	{
		if (!chkCondDSFirstArrival(pkt) && !chkCondDSBurstFull(pkt))
		{
			printf ("%.3f: Link not Active, (seq %ld) Waiting in the buffer with %d other pkts\n", clock*1.0e+6, pkt->getSeqNum(), B_event->wait_cnt());
			B_event->wait(); // Add the pkt to burst buffer
		}
		evtLinkActive->wait();
	}
	
	Server->reserve();
	tblQWait->record(clock - pkt->getOrgTime());

	hold ((double)pkt->getLength()/capacity);
	printf ("%.3f: Packet %ld served, arrived at %.3f, Delay = %.3f us\n", clock*1.0e+6, pkt->getSeqNum(), 0, pkt->getOrgTime()*1.0e+6, (clock - pkt->getOrgTime())*1.0e+6);
	tblDelay->record(clock - pkt->getOrgTime());

	if (nextlink)
	{
		printf ("%.3f: %s: Packet %ld will enter %s now\n", clock*1.0e+6, my_name.c_str(), pkt->getSeqNum(), nextlink->getName().c_str());
		//newPacket = new packet(clock, curr_sq, 12000, 0, my_name + std::string(":") + std::to_string(curr_sq));
		nextlink->processPacket(pkt);
	}
	chkCondBufferEmpty(pkt);
	
	Server->release();		
}
void DualModeEEELink::report()
{
	printf("============================================================= \n");
	printf("==   Simulation of EEE link with two sleep modes           == \n");
	printf("============================================================= \n");
	printf("= T_DtoA             = %.3f us    \n", T_DtoA*1.0e+6);
	printf("= T_FtoA             = %.3f us    \n", T_FtoA*1.0e+6);
	printf("= T_FtoD             = %.3f us    \n", T_FtoD*1.0e+6);
	printf("= T_AtoF             = %.3f us    \n", T_AtoF*1.0e+6);
	printf("= T_AtoD             = %.3f us    \n", T_AtoD*1.0e+6);
	printf("=------------------------------------------------------------ \n");
	printf("= Link Capacity      = %.1f Gb/s  \n", capacity/1.0e+9);
	printf("= S_Coal             = %d         \n", S_Coal);
	printf("= S_Lim              = %d         \n", S_Lim);
	printf("= T_Idle             = %.3f us    \n", T_Idle*1.0e+6);
	printf("= T_Coal             = %.3f us    \n", T_Coal*1.0e+6);
	printf("============================================================= \n");
	printf("= Time in Active     = %2.6f    \n", tblActive->sum()/clock);
	printf("= Time in Deep Sleep = %2.6f    \n", tblDeepSleep->sum()/clock);
	printf("= Time in Fast Wake  = %2.6f    \n", tblFastWake->sum()/clock);
	printf("= Time in AtoF       = %2.6f    \n", tblAtoF->sum()/clock);
	printf("= Time in FtoD       = %2.6f    \n", tblFtoD->sum()/clock);
	printf("= Time in DtoA       = %2.6f    \n", tblDtoA->sum()/clock);
	printf("= Time in FtoA       = %2.6f    \n", tblFtoA->sum()/clock);
	printf("= Time in AtoD       = %2.6f    \n", tblAtoD->sum()/clock);
	printf("=------------------------------------------------------------ \n");
	printf("= Total Completions  = %d frames\n", Server->completions());
	printf("= Mean num in system = %.3f pkts\n", Server->qlen());
	printf("= Mean response time = %.3f us  \n", Server->resp()*1.0e+6);
	printf("= Mean service time  = %.3f us  \n", Server->serv()*1.0e+6);
	printf("= Mean throughput    = %.3f pkts/s\n",Server->tput());
	printf("= Mean packet delay  = %.3f us  \n", tblDelay->mean()*1.0e+6);
	printf("= Mean waiting time  = %.3f us  \n", tblQWait->mean()*1.0e+6);
	//printf("= Power Consumption  = %.3f %   \n", pConsump);
	printf("=------------------------------------------------------------ \n");
}
