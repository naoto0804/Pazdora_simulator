CC = gcc
CFLAGS = -Wall -g
LDLIBS = -lm
OBJS = main.o optimize.o

a.out:	$(OBJS)
	$(CC) $(OBJS) $(LDLIBS)

clean:
	rm -f *~ *.o a.out