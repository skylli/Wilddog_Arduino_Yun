#makefile
CC=mips-openwrt-linux-gcc
FLAGS = -Wall

objects = Wilddog_watch.o sjson.o 
wilddog_watch : $(objects)
	$(CC) $(CFLAGS) -o wilddog_watch $(objects)
	rm $(objects)
$(objects) : Wilddog_daemon.h
Wilddog_transfer.o sjson.o files.o : Wilddog_daemon.h

.PHONY : clean
clean :
	rm wilddog_watch 	
