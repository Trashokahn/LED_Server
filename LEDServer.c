#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "wiringPi.h"


#define BUFSIZE 1000
#define PORT 396
#define QUEUE 3	//Warteschlange fuer Verbindungen

void writeLEDs(int state[4]);

void *LEDControl(void* arg){
	int clientfd = *(int *)arg;
	char inbuffer[BUFSIZE];
	
	//begrüßung
	write(clientfd, "Guten Tag, ich grüße Sie!\nGeben Sie ein Zahl von 0-15 ein um die LEDs zu steuern und \'q\' zum beenden:\n", 104);
	
	while(1){
		//lesen der empfangen Zeichen
		int count = read(clientfd, inbuffer, sizeof(inbuffer));
		
		//wenn q gedrückt wird die Verbindung getrennt
		if(inbuffer[0] == 'q'){
			break;
		}
		
		//press F to show respect
		if(inbuffer[0] == 'F'){
			write(clientfd, "showed respect\n", 15);
			continue;
		}
		
		//convert char* to int
		int n = atoi(inbuffer);
		
		//error checking
		if (n < 0 || n > 15){
			write(clientfd, "Sie haben eine ungültige Zahl eingegeben!\n", 45);
			continue;
		}
		
		//convert int to byte
		int state[4];
		int d = 0;
		for (int i = 3 ; i >= 0 ; i--){
			d = n >> i;
			
			if (d & 1)
				state[i] = 1;
			else
				state[i] = 0;
		}
		
		//setzt die LEDs
		writeLEDs(state);
	}
	close(clientfd);
}

int main(int argc, char *argv[])
{
	//wiringPi setup
	wiringPiSetupGpio();
	pinMode(18, OUTPUT);
	pinMode(23, OUTPUT);
	pinMode(24, OUTPUT);
	pinMode(25, OUTPUT);
	
	//Socket
	int server_socket, rec_socket;
	int len;
	struct sockaddr_in serverinfo, clientinfo;

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	serverinfo.sin_family = AF_INET; //iPv4
	//hört auf allen Interfaces: 0.0.0.0
	serverinfo.sin_addr.s_addr = htonl(INADDR_ANY);
	serverinfo.sin_port = htons(PORT);

	//verbinden
	if (bind(server_socket, (struct sockaddr *) &serverinfo, sizeof(serverinfo)) != 0){
		printf("Fehler socket!\n");
		return -1;
	}

	//Server wartet auf connect von Client
	listen(server_socket, QUEUE); 	
	
	// Endlosschleife Server zur Abarbeitung der Client Anfragen
	while(1){
		printf("Server wartet......\n");
		//Verbinung von Client
		rec_socket = accept(server_socket,(struct sockaddr*)&clientinfo, &len);
		printf("Verbindung von %s:%d\n", inet_ntoa(clientinfo.sin_addr), ntohs(clientinfo.sin_port));
		
		pthread_t child;
		//Thread mit Funktion LEDControl(rec_socket);
		if (pthread_create(&child, NULL, LEDControl, &rec_socket) != 0){ 
			perror("child is wrong");
			return -1;
		}
		else {
			pthread_join(child, NULL);
			printf("Abgekoppelt!\n"); // Fehlerfall: Abbruch
			pthread_detach(child);// abkoppeln vom Hauptprozess
		}
		
		//Verbindung zum Client getrennt
		close(rec_socket);
	}

	close(rec_socket);
	return 0;
}

void writeLEDs(int state[4]){
	printf("%d, %d, %d, %d\n",state[0],state[1],state[2],state[3]);

	digitalWrite(18, state[0]);
	digitalWrite(23, state[1]);
	digitalWrite(24, state[2]);
	digitalWrite(25, state[3]);
}

