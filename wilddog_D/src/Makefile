#makefile
#CC=gcc#mips-openwrt-linux-gcc
#FLAGS = -Wall
LDFLAGS := ./libwilddog.a -lm

objects = wilddog_D.o
wilddog_D : $(objects)
	$(CC) $(CFLAGS) -o wilddog_D $(objects) $(LDFLAGS) -static
	rm $(objects)
$(objects) : wilddog_D.h utlist.h
Wilddog_transfer.o files.o :

.PHONY : clean
clean :
	rm wilddog_D 	
