#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define MYPORT 3490

int ponerAEscuchar(int sockfd,int puertoServidor);
int enviar(char* envio,int socketAlQueEnvio,int tamanioDelEnvio);
int recibir(char* bufferReceptor,int socketReceptor,int tamanioQueRecibo);
int conectarseA(char* ipDestino,int puertoDestino);

int ponerAEscuchar(int sockfd,int puertoServidor){
	struct sockaddr_in socketInfo;
	int nuevoSocket;
	socketInfo.sin_family=AF_INET;
	socketInfo.sin_port=htons(puertoServidor);
	socketInfo.sin_addr.s_addr=inet_addr("10.0.2.15");
	struct sockaddr_in  their_addr;
	int sin_size;
	printf("%s \n",inet_ntoa(socketInfo.sin_addr));
	/*
	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,socketActivado,sizeof(int))==-1){
		perror("No se puede setear el socket");
		exit(1);
	}*/

	if(bind(sockfd,(struct sockaddr*)&socketInfo,sizeof(struct sockaddr))!=0){
		perror("Fallo el bindeo de la conexion");
		printf("Revisar que el puerto no este en uso");
		close(sockfd);
		return -1;
	}

	if(listen(sockfd,SOMAXCONN)==-1){
		perror("Fallo el listen");

	}
	sin_size=sizeof(struct sockaddr_in);
	nuevoSocket=accept(sockfd,(struct sockaddr*)&their_addr,&sin_size);
	return nuevoSocket;

}

int conectarseA(char* ipDestino,int puertoDestino){
	int socketAConectarse;
	struct sockaddr_in dest_addr;
	socketAConectarse=socket(AF_INET,SOCK_STREAM,0);
	dest_addr.sin_family=AF_INET;
	dest_addr.sin_port=htons(puertoDestino);
	dest_addr.sin_addr.s_addr=inet_addr(ipDestino);
	memset(&(dest_addr.sin_zero),'\0',8);
	connect(socketAConectarse,(struct sockaddr*)&dest_addr,sizeof(struct sockaddr));
	return 0;
}

int enviar(char* envio,int socketAlQueEnvio,int tamanioDelEnvio){
	int bytesEnviados;
	bytesEnviados=send(socketAlQueEnvio,envio,tamanioDelEnvio,0);
	return bytesEnviados;
}

int recibir(char* bufferReceptor,int socketReceptor,int tamanioQueRecibo){
	int bytesRecibidos;
	bytesRecibidos=recv(socketReceptor,bufferReceptor,tamanioQueRecibo,0);
	return bytesRecibidos;
}

