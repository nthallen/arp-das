.PHONY : all
all : server client
server : server.o network.o
	cc -o server server.o network.o
client : client.o network.o
	cc -o client client.o network.o
network.o : network.c network.h
client.o : client.c network.h
server.o : server.c network.h
