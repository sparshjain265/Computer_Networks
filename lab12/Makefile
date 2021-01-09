all: client server router

client: udp_client.c
	gcc udp_client.c -o client

server: udp_server.c
	gcc udp_server.c -o server

router: router.cpp
	g++ router.cpp -o router

clean:
	rm client server router