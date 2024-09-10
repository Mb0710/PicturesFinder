#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>


#ifdef _WIN32
 	#include <winsock2.h>
 	#include <ws2tcpip.h>
#else
	 #include <arpa/inet.h>
 	 #include <netinet/in.h>
 	 #include <sys/socket.h>
#endif


#define CONNECTION_HOST "127.0.0.1"
#define LISTENING_PORT 5094
#define IPV4 AF_INET
#define BUFFER_SIZE 1024

int main(void)
{
	#ifdef _WIN32
		WSADATA wsa;
		
		if(WSAStartup(MAKEWORD(2, 2), &wsa= != 0)
		{
		
			fprintf(stderr, "(CLIENT) Echec d'initilisation de WinSock\n");
			exit(1);
		}
	#endif
	
	int socketFD = socket(IPV4, SOCK_STREAM, 0);
	
	if(socketFD == -1)
	{
		fprintf(stderr, "(CLIENT) Echec d'initilisation du socket\n");
		exit(1);
	}
	
	struct sockaddr_in socketAddress;
	socketAddress.sin_family = IPV4;
	socketAddress.sin_port = LISTENING_PORT;
	
	
	int inetReturnCode = inet_pton(IPV4, CONNECTION_HOST , &socketAddress.sin_addr);
	
	if(inetReturnCode == -1){
		fprintf(stderr, "(CLIENT) Addresse invalide ou non prise en charge\n");
		exit(1);
	}
	
	int socketAddressLength = sizeof(socketAddress);
	int connectionStatus = connect(socketFD,(struct sockaddr*) &socketAddress,socketAddressLength);
	
	if(connectionStatus == -1){
		fprintf(stderr, "(CLIENT) Echec de la connexion au serveur\n");
		exit(1);
	}
	
	char keyword[BUFFER_SIZE];
	printf("Entrez un mot-cle pour rechercher votre image : ");
	fgets(keyword, BUFFER_SIZE, stdin);
	keyword[strcspn(keyword, "\n")] = '\0';
	
	int sentBytes = send(socketFD, keyword, strlen(keyword), 0);
	
	if(sentBytes == -1)
	{
		fprintf(stderr, "(CLIENT) Echec  d'envoi du message au serveur\n");
		exit(1);
	}
	
	FILE *fp = fopen("image_recue.jpg", "wb");
	if(fp == NULL)
	{
		fprintf(stderr, "(CLIENT) Echec  d'ouverture du fichier pour l'ecriture\n");
		exit(1);
	}
	
	char buffer[BUFFER_SIZE];
	int receivedBytes;
	while((receivedBytes = recv(socketFD, buffer, BUFFER_SIZE, 0)) > 0){
		fwrite(buffer, 1 , receivedBytes, fp);
	}
	
	if(receivedBytes == 1)
	{
		fprintf(stderr, "(CLIENT) Echec  de reception de l'image\n");
		fclose(fp);
		exit(1);	
	}
	
	fclose(fp);
	
	
	
	#ifdef _WIN32
		closesocket(socketFD);
		WSACleanup();
	#else
		close(socketFD);
	#endif
			
	return 0;

}
