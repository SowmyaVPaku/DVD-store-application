
CC=gcc
CCOPTS=-g

TARGETS=ex1_timeserver ex1_timeclient ex2_server ex2_tclient ex2_uclient

all: $(TARGETS)

ex1_timeserver: server_daytime.c
	$(CC) $(CCOPTS) -o $@ $^

ex1_timeclient: client_daytime.c
	$(CC) $(CCOPTS) -o $@ $^

ex2_server: server.c
	$(CC) $(CCOPTS) -o $@ $^ -lpthread

ex2_tclient: tclient.c
	$(CC) $(CCOPTS) -o $@ $^
	
ex2_uclient: uclient.c
	$(CC) $(CCOPTS) -o $@ $^

clean:
	rm -f $(TARGETS) *.o
