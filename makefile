prog: LED.o
	gcc -o LED.o lwiringPi lpthread

LED.o: LEDServer.c
	gcc -c LEDServer.c

