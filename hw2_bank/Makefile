# Makefile for the smash program
OBJS = Bank.o
RM = rm -f
# Creating the  executable

Bank: $(OBJS)
	g++ -std=c++11 -g -Wall -Werror -pedantic-errors -DNDEBUG -pthread *.cpp -o Bank

# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*
	rm Bank
