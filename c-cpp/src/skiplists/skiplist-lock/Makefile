ROOT = ../../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/$(LOCK)-skiplist

.PHONY:	all clean map

map: main-map

all:	main

skiplist-lock-map.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/skiplist-lock-map.o skiplist-lock-map.c

optimistic-map.o: skiplist-lock-map.h 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/optimistic-map.o optimistic-map.c 

sl-map.o: skiplist-lock-map.h optimistic-map.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/sl-map.o sl-map.c

test-map.o: skiplist-lock-map.h optimistic-map.h sl-map.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test-map.o test-map.c

main-map: skiplist-lock-map.o optimistic-map.o sl-map.o test-map.o 
	$(CC) $(CFLAGS) $(BUILDIR)/skiplist-lock-map.o $(BUILDIR)/optimistic-map.o $(BUILDIR)/sl-map.o $(BUILDIR)/test-map.o -o $(BINS)-map $(LDFLAGS)

skiplist-lock.o:
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/skiplist-lock.o skiplist-lock.c

optimistic.o: skiplist-lock.h 
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/optimistic.o optimistic.c 

intset.o: skiplist-lock.h optimistic.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/intset.o intset.c

test.o: skiplist-lock.h optimistic.h intset.h
	$(CC) $(CFLAGS) -c -o $(BUILDIR)/test.o test.c

main: skiplist-lock.o optimistic.o intset.o test.o 
	$(CC) $(CFLAGS) $(BUILDIR)/skiplist-lock.o $(BUILDIR)/optimistic.o $(BUILDIR)/intset.o $(BUILDIR)/test.o -o $(BINS) $(LDFLAGS)

clean:
	-rm -f $(BINS) *.o
