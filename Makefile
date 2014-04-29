CC      = gcc
MPICC      = mpicc
CFLAGS  = -O2
LDFLAGS  = -O2 -g


all:  oddEvenSort

oddEvenSort: oddEvenSort.o
	$(MPICC) -o $@ $^ $(LDFLAGS)

.c.o: 
	$(MPICC)  $(CFLAGS) -c $<

clean:
	rm  *.o oddEvenSort
