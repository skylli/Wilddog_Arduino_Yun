LDFLAGS := -lwilddog -lpthread

objects = wilddog_watch.o sjson.o
daemon_object = wilddogd.o

all: wilddog_watch wilddogd

wilddog_watch : $(objects)
	$(CC) $(CFLAGS) -o wilddog_watch $(objects)
	rm $(objects)
$(objects) : wilddog_daemon.h wilddogd.h utlist.h
    
wilddogd : $(daemon_object)
	$(CC) $(CFLAGS) -o wilddogd $(daemon_object) $(LDFLAGS) -static
	rm $(daemon_object)

$(daemon_object) : wilddog_daemon.h wilddogd.h utlist.h

.PHONY : clean
clean :
	rm wilddog_watch wilddogd