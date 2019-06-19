prog: LEDServer.o
	gcc -o prog LEDServer.o -lwiringPi -lpthread

LEDServer.o: LEDServer.c
	gcc -c LEDServer.c

