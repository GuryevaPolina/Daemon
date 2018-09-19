CC=gcc

daemon: daemon.c
	$(CC) -o daemon daemon.c

clean:
	rm daemon
