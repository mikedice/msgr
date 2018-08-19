CPPFLAGS=-g

msgr : main.o Listener.o Client.o
	g++ -o msgr main.o Listener.o Client.o

clean : 
	rm *.o
	rm msgr
	rm listenerlog.txt
	rm clientlog.txt

Client.o : Client.h
Listener.o : Listener.h