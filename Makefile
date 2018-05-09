SRCMPI	= mpi_intercept_library.c hello.c

OBJMPI	=$(SRCMPI:.c=.o)

SRCINJ  = core_management.c inject_usage.c inject_memory.c inject_cpu.c inject_io.c inject_network.c

OBJINJ  =$(SRCINJ:.c=.o)

CC	= cc
PP	= -g 

CFLAGS	= -O3 $(PP) 
LIBS	= 

PRGMPI	= hello

PRGINJ  = inject

main:	$(PRGMPI) $(PRGINJ)

.c.o:	Makefile
	$(CC) -c $(CFLAGS) $(ELIBS) $<

$(PRGMPI):$(OBJMPI) Makefile definitions.h
	$(CC) $(LDFLAGS) -o $@ $(OBJMPI) $(LIBS)

$(PRGINJ):$(OBJINJ) Makefile inject_definitions.h
	$(CC) $(LDFLAGS) -o $@ $(OBJINJ) $(LIBS)

clean:
	rm -fr $(TMP) $(OBJMPI) $(OBJINJ) $(PRGMPI) $(PRGINJ) core
