#ifndef PACKET_H
#define PACKET_H

#include <iostream>
#include <string>

class Packet
{
private:
	double org_time;
	unsigned long int sq, length;
	double iat;
	std::string my_name;
public:
	Packet(double timeGen, unsigned long int seqNum, unsigned long int size, double iaTime, std::string pktName) :
		org_time (timeGen), sq (seqNum), length (size), iat (iaTime), my_name (pktName)
	{
//		printf ("%.3f: New packet made with name <%s> \n", clock*1.0e+6, name.c_str());			
	}
	unsigned long int  getSeqNum()  const { return (sq); } 
	unsigned long int  getLength()  const { return (length); }
	double             getOrgTime() const { return (org_time); }
	std::string        getName()    const { return (my_name); }
};

#endif