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

hsval: all
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histserver 1 all
hcval: all
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histclient 1000 1000 1000

hstval: allt
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histserver_th 2 abc def
hctval: allt
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histclient_th 10 10 10


clean:
	rm -fr *~ histserver histclient histserver_th histclient_th
