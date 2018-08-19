CPPFLAGS=-g

msgr : main.o Listener.o Client.o
	g++ -o msgr main.o Listener.o Client.o

clean : 
	rm *.o
	rm msgr

Client.o : Client.h
Listener.o : Listener.h