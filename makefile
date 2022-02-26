a: all allt

all: hc hs

allt: hct hst


hc: histclient.c
	gcc -Wall -gdwarf-4 -o histclient histclient.c -lrt
hs: histserver.c
	gcc -Wall -gdwarf-4 -o histserver histserver.c -lrt

hct: histclient_th.c
	gcc -Wall -gdwarf-4 -o histclient_th histclient_th.c -lpthread -lrt
hst: histserver_th.c
	gcc -Wall -gdwarf-4 -o histserver_th histserver_th.c -lpthread -lrt

hsval: hs
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histserver 2 abc def
hcval: hc
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histclient 10 10 0

hstval: hst
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histserver_th 2 abc def
hctval: hct
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histclient_th 10 10 0


rs: hs
	./histserver 1 abc
rc: hc
	./histclient 10 10 0
rst: hst
	./histserver_th 2 abc def
rct: hct
	./histclient_th 10 10 0
clean:
	rm -fr *~ histserver histclient histserver_th histclient_th
