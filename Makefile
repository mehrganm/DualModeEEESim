CC = g++
CFLAGS = -Wall -c -DCPP -DGPP -std=c++11
LFLAGS = -Wall -m64 -lm
CSIM_INC_DIR = $(HOME)/CSIM20/g++/lib
CSIM_GPP = $(HOME)/CSIM20/g++/lib/csim.gpp.a

default: SIMUToolsPaperScenario

OBJS = SIMUToolsPaperScenario.o DualModeEEELink.o IPPPacketGenerator.o PoissonPacketGenerator.o 

SIMUToolsPaperScenario : $(OBJS)
	$(CC) $(LFLAGS) $(CSIM_GPP) $(OBJS) -o SIMUToolsPaperScenario $(CSIM_GPP)

SIMUToolsPaperScenario.o : Packet.h PoissonPacketGenerator.h DualModeEEELink.h SIMUToolsPaperScenario.cpp 
	$(CC) $(CFLAGS) SIMUToolsPaperScenario.cpp -I$(CSIM_INC_DIR)

DualModeEEELink.o : DualModeEEELink.h SleepLink.h DualModeEEELink.cpp 
	$(CC) $(CFLAGS) DualModeEEELink.cpp -I$(CSIM_INC_DIR)

PoissonPacketGenerator.o : PoissonPacketGenerator.h PacketGenerator.h Packet.h PoissonPacketGenerator.cpp 
	$(CC) $(CFLAGS) PoissonPacketGenerator.cpp -I$(CSIM_INC_DIR)
	
IPPPacketGenerator.o : IPPPacketGenerator.h PacketGenerator.h Packet.h IPPPacketGenerator.cpp
	$(CC) $(CFLAGS) IPPPacketGenerator.cpp -I$(CSIM_INC_DIR)
	
clean:
	\rm *.o *~ SIMUToolsPaperScenario

