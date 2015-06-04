#include <iostream>
#include <ctype.h>
#include <unistd.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

using namespace std;

int sock;
struct sockaddr_in client;
int PORT = 80;

int setupSocket(char* hostname) {
	struct hostent * host = gethostbyname(hostname);

	if ( (host == NULL) || (host->h_addr == NULL) ) {
		cout << "Error retrieving DNS information." << endl;
		return -1;
	}

	bzero((char*)&client, sizeof(client));
	client.sin_family = AF_INET;
	client.sin_port = htons( PORT );        
	memcpy(&client.sin_addr, host->h_addr, host->h_length);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sock < 0) {
		cout << "Error creating socket." << endl;
		return -1;
	}
	return 0;
}

int connectToHost(char* hostname) {
	if ( connect(sock, (struct sockaddr *)&client, sizeof(client)) < 0 ) {
		close(sock);
		cout << "Could not connect to " << hostname << endl;
		return -1;
	}
	return 0;
}

int sendRequest(char* hostname,char *requast) {
	string request = "GET /get?";
	request+=string(requast);
	request+= " HTTP/1.1\r\nHost: ";
	request+= string(hostname);
	request += "\r\nConnection: close\r\n\r\n";      
	if (send(sock, request.c_str(), request.length(), 0) != (int)request.length()) {
		cout << "Error sending request." << endl;
		return -1;
	}
	return 0;
}

void getResponse() {
	char cur;
	while ( read(sock, &cur, 1) > 0 ) {		
	}
}

typedef struct _http_requast_data{
	char* hostname;
	char *requast;
} http_requast_data;

void* simple_http_requast(void *_data){
        
        http_requast_data *data=(http_requast_data *)_data;	
	if (setupSocket(data->hostname)<0) goto free_resource; 
	if (connectToHost(data->hostname)<0 ) goto free_resource;
	if (sendRequest(data->hostname,data->requast)<0) goto free_resource;
	getResponse();
        free_resource :{
	 delete data->hostname;	
	 delete data->requast;
        }
	return NULL;
	
}

pthread_t th;
http_requast_data data;

int send_http_requast(char* hostname,char* requast){

  data.hostname=new char[strlen(hostname)];
  data.requast=new char[strlen(requast)];

  strcpy(data.hostname,hostname);
  strcpy(data.requast,requast);

  pthread_create(&th,NULL,simple_http_requast,&data);

}
