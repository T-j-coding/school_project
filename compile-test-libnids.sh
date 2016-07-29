gcc -fno-strict-aliasing -o test-libnids test-libnids.c packet_send.c hash.c util.c -lpcap -lnet -lnids -lpthread -lgthread-2.0
