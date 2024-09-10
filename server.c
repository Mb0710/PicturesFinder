#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <json-c/json.h>



#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>	
#else
	#include <netinet/in.h>
	#include <sys/socket.h>
#endif


#define  LISTENING_PORT  5094
#define  IPV4  AF_INET
#define PENDING_QUEUE_MAXLENGTH 1
#define BUFFER_SIZE 1024

#define PIXABAY_API_URL "https://pixabay.com/api/?key=%s&q=%s"
#define PIXABAY_API_KEY "43706057-3a1c4807b06e95c95266d2840"


struct memory{
	char *response;
	size_t size;
};





size_t write_callback(void* data, size_t size, size_t nmemb, struct memory *mem){
	size_t realsize = size * nmemb;
	char *ptr = realloc(mem->response, mem->size + realsize + 1);
	if(ptr == NULL)
	{
		fprintf(stderr, "Erreur d'allocation memoire\n");
		return 0;
	}
	
	
	mem->response = ptr;
	memcpy(&(mem->response[mem->size]), data, realsize);
	mem->size += realsize;
	mem->response[mem->size] = '\0';
	
	return realsize;
}


char* searchImageUrl(const char *keyword)
{
	CURL *curl;
	CURLcode res;
	struct memory chunk = {0};
	
	curl = curl_easy_init();
	
	if(!curl){
		fprintf(stderr,"Erreur d'initialisation de lilcurl\n");
		return NULL;
	}
	
	char url[256];
	snprintf(url, sizeof(url), PIXABAY_API_URL , PIXABAY_API_KEY , keyword);
	
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
	
	res = curl_easy_perform(curl);
	if(res != CURLE_OK)
	{
		fprintf(stderr, "Erreur lors de la requête : %s\n", curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		return NULL;
	}
	
	curl_easy_cleanup(curl);
	
	struct json_object *parsed_json,*hits,*first_result,*largeImageURL;
	
	parsed_json = json_tokener_parse(chunk.response);
	json_object_object_get_ex(parsed_json, "hits", &hits);
	first_result = json_object_array_get_idx(hits,0);
	json_object_object_get_ex(first_result, "largeImageURL", &largeImageURL);
	
	const char *image_url = json_object_get_string(largeImageURL);
	
	char *url_copy = strdup(image_url);
	
	json_object_put(parsed_json);
	free(chunk.response);
	
	return url_copy;
}


int dlImage(const char *url, const char *filename)
{
	CURL *curl;
	FILE *fp;
	CURLcode res;
	
	curl = curl_easy_init();
	if(!curl)
	{
		fprintf(stderr,"Erreur d'initialisation du curl\n");
		return 1;
	}
	
	fp = fopen(filename, "wb");
	if(!fp)
	{
		fprintf(stderr,"Erreur d'ouverture du fichier pour l'ecriture \n");
		curl_easy_cleanup(curl);
		return 1;
	}
	
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
	res = curl_easy_perform(curl);
	
	if(res != CURLE_OK)
	{
		fprintf(stderr,"Erreur de téléchargement : %s\n", curl_easy_strerror(res));
		fclose(fp);
		curl_easy_cleanup(curl);
		return 1;
	}
	
	fclose(fp);
	curl_easy_cleanup(curl);
	return 0;
}

int main(void)
{
	#ifdef _WIN32
		WSADATA wsa:
		if(WSAStartup(MAKEWORD(2, 2), &wsa != 0)
		{
			fprintf(stderr, "(SERVEUR) Echec d'initilisation de WinSock\n");
			exit(1);
		}
	#endif
	
	
	int socketFD = socket(IPV4, SOCK_STREAM, 0);
	
	if(socketFD == -1)
	{
		fprintf(stderr, "(SERVEUR) Echec d'initilisation du socket\n");
		exit(1);
	}
	
	
	int opt = 1;
	if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
	    fprintf(stderr, "(SERVEUR) Echec de configuration de l'option SO_REUSEADDR\n");
	    exit(1);
	}
	
	// CFG SOCKET + LIAISON
	
	struct sockaddr_in socketAddress;
	socketAddress.sin_family = IPV4;
	socketAddress.sin_port = LISTENING_PORT;
	socketAddress.sin_addr.s_addr =  INADDR_ANY;
	
	int socketAddressLength = sizeof(socketAddress);
	int bindReturnCode = bind(socketFD, (struct sockaddr*)&socketAddress, socketAddressLength);
	
	if(bindReturnCode == -1)
	{
		fprintf(stderr, "(SERVEUR) Echec de liaison pour le  socket\n");
		exit(1);
	}
	
	if(listen(socketFD, PENDING_QUEUE_MAXLENGTH) == -1)
	{
		fprintf(stderr, "(SERVEUR) Echec de l'ecoute \n");
		exit(1);
	}
	
	puts("En attente de connexions...");
	
	int connectedSocketFD = accept(socketFD, (struct sockaddr*) &socketAddress,(socklen_t*) &socketAddressLength);
	
	
	if(connectedSocketFD == -1)
	{
		fprintf(stderr, "(SERVEUR) Echec de l'etablissement de la connexion\n");
		exit(1);
	}
	
	
	// RECEPTION D UN MSG
	char buffer[BUFFER_SIZE] = {0};
	int receivedBytes = recv(connectedSocketFD, buffer, BUFFER_SIZE, 0);
	
	if(receivedBytes == -1)
	{
		fprintf(stderr, "(SERVEUR) Echec de reception du message du client\n");
		exit(1);
	}
	
	buffer[receivedBytes] =  '\0';
	
	printf("Client : %s\n" , buffer);
	
	char *imageURL = searchImageUrl(buffer);
	if(imageURL == NULL)
	{
		fprintf(stderr, "(SERVEUR) Echec de la recherche d'image\n");
		exit(1);
	}
	
	const char *filename = "image.jpg";
	if(dlImage(imageURL, filename) !=0)
	{
		fprintf(stderr, "(SERVEUR) Echec de telechargement de l'image\n");
		free(imageURL);
		exit(1);
	}
	
	free(imageURL);
	
	FILE* fp = fopen(filename, "rb");
	if(fp == NULL){
		fprintf(stderr,"(SERVEUR) Echec d'ouverture du fichier image\n");
		exit(1);
	}
	
	char fileBuffer[BUFFER_SIZE];
	size_t bytesRead;
	while((bytesRead = fread(fileBuffer, 1, sizeof(fileBuffer), fp)) > 0){
		if (send(connectedSocketFD, fileBuffer, bytesRead, 0 ) == -1){
			fprintf(stderr, "(SERVEUR) Echec de l'envoi de l'image\n");
			fclose(fp);
			exit(1);
		}
	}
	
	fclose(fp);
		
	//FERMETURE DES SOCKETS 	
	#ifdef _WIN32
		closesocket(connectedSocketFD);
		closesocket(socketFD);
		WSACleanup();
	#else
		close(connectedSocketFD);
		close(socketFD);
	#endif
	return 0;

}
 
