a: hc hs

hc: histclient.c
	gcc -Wall -o histclient histclient.c -lrt
hs: histserver.c
	gcc -Wall -o histserver histserver.c -lrt


clean: 
	rm -fr *~ histserver histclient