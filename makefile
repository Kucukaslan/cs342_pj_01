a: all allt

all: hc hs

allt: hct hst

tar:
	tar czf 21901779.tar.gz histclient.c histserver.c shared_defs.h shared_defs_th.h histclient_th.c histserver_th.c makefile readme.txt report.pdf
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
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histclient 1000 1000 0

hstval: hst
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histserver_th 2 abc def
hctval: hct
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes ./histclient_th 1000 1000 0

rs: hs
	./histserver 10 1m 1m 1m 1m 1m 1m 1m 1m 1m 1m  
rc: hc
	./histclient 1000 1000 0
rst: hst
	./histserver_th 10 1m 1m 1m 1m 1m 1m 1m 1m 1m 250k
rct: hct
	./histclient_th 1000 1000 0

rs1: hs
	./histserver 1 3m 3m 3m 3m 3m 3m 3m 3m 3m 3m 
rc1: hc
	./histclient 1000 1000 0
rst1: hst
	./histserver_th 1 3m 3m 3m 3m 3m 3m 3m 3m 3m 3m 
rct1: hct
	./histclient_th 1000 1000 0

rs3: hs
	./histserver 2  3m 3m 3m 3m 3m 3m 3m 3m 3m 
rc3: hc
	./histclient 1000 1000 0
rst3: hst
	./histserver_th 2 3m 3m 3m 3m 3m 3m 3m 3m 3m 3m 
rct3: hct
	./histclient_th 1000 1000 0

rs4: hs
	./histserver 4  3m 3m 3m 3m 3m 3m 3m 3m 3m 
rc4: hc
	./histclient 1000 1000 0
rst4: hst
	./histserver_th 4 3m 3m 3m 3m 3m 3m 3m 3m 3m 3m 
rct4: hct
	./histclient_th 1000 1000 0

rs5: hs
	./histserver 5 3m 3m 3m 3m 3m 3m 3m 3m 3m 3m 
rc5: hc
	./histclient 1000 1000 0
rst5: hst
	./histserver_th 5 3m 3m 3m 3m 3m 3m 3m 3m 3m 3m 
rct5: hct
	./histclient_th 1000 1000 0

rs7: hs
	./histserver 7 3m 3m 3m 3m 3m 3m 3m 3m 3m 3m 
rc7: hc
	./histclient 1000 1000 0
rst7: hst
	./histserver_th 7 3m 3m 3m 3m 3m 3m 3m 3m 3m 3m 
rct7: hct
	./histclient_th 1000 1000 0
	
clean:
	rm -fr *~ histserver histclient histserver_th histclient_th
