# Enables debug messages while compiling
COMPILE_DEBUG=@
VERSION=2.0

# Compile and Link flags, libraries
CC=$(CROSS_PREFIX)gcc
CFLAGS= -O0 -DVERSION=$(VERSION)
LDFLAGS=
LIBS=

PROGS= apex_sim

all: clean $(PROGS)

# Add all object files to be linked in sequence
APEX_OBJS:= file_parser.o apex_cpu.o main.o

apex_sim: $(APEX_OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(COMPILE_DEBUG)$(CC) $(CFLAGS) -c -o $@ $<
	$(COMPILE_DEBUG)echo "CC $<"

clean:
	rm -f *.o *.d *~ $(PROGS)

cpu:clean all
	./apex_sim	old_input.asm