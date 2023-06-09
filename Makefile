# Makefile for the smash program
CC = g++
CFLAGS = -std=c++11 -g -Wall 
CCLINK = $(CC)
OBJS = smash.o commands.o signals.o
RM = rm -f
# Creating the  executable
smash: $(OBJS)
	$(CCLINK) $(CFLAGS)-o smash $(OBJS)
# Creating the object files
signals.o: signals.cpp signals.h commands.h
commands.o: commands.cpp commands.h
smash.o: smash.cpp commands.h

# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*

