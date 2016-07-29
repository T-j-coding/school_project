COMPILE_OPTIONS = -fno-strict-aliasing
LIBS = -lpcap -lnet -lnids -lpthread -lgthread-2.0

test-libnids:test-libnids.c packet_send.c hash.c util.c cc.o
	g++ -o test-libnids cc.o test-libnids.c packet_send.c hash.c util.c $(COMPILE_OPTONS) $(LIBS)

cc.o:cc.cc
	g++ -c cc.cc -o cc.o

clean:
	rm -f test-libnids cc.o
