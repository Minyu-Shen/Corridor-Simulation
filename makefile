#
#  makefile
#  Corridor-Simulation
#
#  Created by Samuel on 8/4/18.
#  Copyright © 2018 Samuel. All rights reserved.
#

CC=clang++
CFLAGS= -c -O3 --std=c++11 -Wall
HEADERS= arena.hpp Bus.hpp BusGenerator.hpp Convoy.hpp Corridor.hpp json.hpp Link.hpp PaxStop.hpp PaxConvoyStop.hpp Queues.hpp SimulationConfig.hpp
OBJS= main.o arena.o Bus.o BusGenerator.o Convoy.o Link.o PaxStop.o PaxConvoyStop.o Queues.o Corridor.o SimulationConfig.o

all: main

main: $(OBJS)
	$(CC) $(OBJS) -o main

arena.o: arena.cpp $(HEADERS)
	$(CC) $(CFLAGS) arena.cpp

Bus.o: Bus.cpp $(HEADERS)
	$(CC) $(CFLAGS) Bus.cpp

BusGenerator.o: BusGenerator.cpp $(HEADERS)
	$(CC) $(CFLAGS) BusGenerator.cpp

Convoy.o: Convoy.cpp $(HEADERS)
	$(CC) $(CFLAGS) Convoy.cpp

Corridor.o: Corridor.cpp $(HEADERS)
	$(CC) $(CFLAGS) Corridor.cpp

Link.o: Link.cpp $(HEADERS)
	$(CC) $(CFLAGS) Link.cpp

PaxStop.o: PaxStop.cpp $(HEADERS)
	$(CC) $(CFLAGS) PaxStop.cpp

PaxConvoyStop.o: PaxConvoyStop.cpp $(HEADERS)
	$(CC) $(CFLAGS) PaxConvoyStop.cpp

Queues.o: Queues.cpp $(HEADERS)
	$(CC) $(CFLAGS) Queues.cpp

SimulationConfig.o: SimulationConfig.cpp $(HEADERS)
	$(CC) $(CFLAGS) SimulationConfig.cpp

main.o: main.cpp $(HEADERS)
	$(CC) $(CFLAGS) main.cpp
