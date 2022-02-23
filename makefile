a: all allt

all: hc hs

allt: hct hst


hc: histclient.c
	gcc -Wall -gdwarf-4 -o histclient histclient.c -lrt
hs: histserver.c
	gcc -Wall -gdwarf-4 -o histserver histserver.c -lrt

hc: histclient_th.c
	gcc -Wall -gdwarf-4 -o histclient_th histclient_th -lrt
hs: histserver_th.c
	gcc -Wall -gdwarf-4 -o histserver_th histserver_th -lrt

hval: all
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histserver 2 abc def
sval: all
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histclient 10 10 10

htval: all
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histserver_th 2 abc def
stval: all
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histclient_th 10 10 10


clean:
	rm -fr *~ histserver histclient histserver_th histclient_th
