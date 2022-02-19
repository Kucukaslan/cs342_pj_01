a: hc hs

all: hc hs

hc: histclient.c
	gcc -Wall -gdwarf-4 -o histclient histclient.c -lrt
hs: histserver.c
	gcc -Wall -gdwarf-4 -o histserver histserver.c -lrt


clean: 
	rm -fr *~ histserver histclient