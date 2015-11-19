#makefile
CC=mips-openwrt-linux-gcc
FLAGS = -Wall

objects = Wilddog_transfer.o sjson.o 
wilddog_transfer : $(objects)
	$(CC) $(CFLAGS) -o wilddog_transfer $(objects)
	rm $(objects)
$(objects) : Wilddog_daemon.h
Wilddog_transfer.o sjson.o files.o : Wilddog_daemon.h

.PHONY : clean
clean :
	rm wilddog_transfer 	
