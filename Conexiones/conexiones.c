
#include "conexiones.h"
#define MYPORT 3490

int ponerAEscuchar(int sockfd,int puertoServidor){
	struct sockaddr_in socketInfo;
	int nuevoSocket;
	socketInfo.sin_family=AF_INET;
	socketInfo.sin_port=htons(puertoServidor);
	socketInfo.sin_addr.s_addr=inet_addr("10.0.2.15");
	struct sockaddr_in  their_addr;
	int sin_size;
	printf("%s \n",inet_ntoa(socketInfo.sin_addr));
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

int enviar(int socketAlQueEnvio, void* envio,int tamanioDelEnvio){
	int bytesEnviados;
	bytesEnviados=send(socketAlQueEnvio,envio,tamanioDelEnvio,0);
	return bytesEnviados;
}

int recibir(int socketReceptor, void* bufferReceptor,int tamanioQueRecibo){
	int bytesRecibidos;
	bytesRecibidos=recv(socketReceptor,bufferReceptor,tamanioQueRecibo,0);
	return bytesRecibidos;
}

int escucharMultiplesConexiones(int socketEscucha,int puertoEscucha){
	fd_set master;
	fd_set read_Fs;
	struct sockaddr_in myAddr;
	struct sockaddr_in remoteAddr;
	int fdmax;
	int newfd;
	char buff[256];
	int nBytes;
	int addrlen;
	int i,j;
	struct timeval intervalo;
	intervalo.tv_sec=5;

	FD_ZERO(&master);
	FD_ZERO(&read_Fs);
	myAddr.sin_family=AF_INET;
	myAddr.sin_addr.s_addr=INADDR_ANY;
	myAddr.sin_port=htons(puertoEscucha);
	memset(&(myAddr.sin_zero),'\0',8);
	if(bind(socketEscucha,(struct sockaddr*)&myAddr,sizeof(myAddr))==-1){
		printf("No se pudo bindear Correctamente");
		exit(1);
	}

	if(listen(socketEscucha,10)==-1){
		printf("No se pudo poner a escuchar");
		exit(1);
	}

	FD_SET(socketEscucha,&master);
	fdmax=socketEscucha;
	for(;;){
		read_Fs=master;
		if(select(fdmax+1,&read_Fs,NULL,NULL,&intervalo)==-1){
			printf("Fallo la aplicacion de select()");
			exit(1);
		}

		for(i=0;i<=fdmax;i++){
			if(FD_ISSET(i,&read_Fs)){
				if(i==socketEscucha){
					addrlen=sizeof(remoteAddr);
					newfd=accept(socketEscucha,(struct sockaddr*)&remoteAddr,&addrlen);
					if(newfd==-1){
						printf("Fallo la aplicacion del accept");
					}else{
					FD_SET(newfd,&master);
					if(newfd>fdmax){
						fdmax=newfd;
					}}}else{
					if((nBytes=recv(i,buff,sizeof(buff),0))<=0){
						if(nBytes==0){
							printf("Se cerro la conexcion");
						}else{
							printf("Fallo la aplicacion del Recv()");
							}
							close(i);
							FD_CLR(i,&master);
						}else{
							for(j=0;j<=fdmax;j++){
								if(j!=socketEscucha && j!=i){
									if(send(j,buff,nBytes,0)==-1){
										printf("Fallo aplicacion del send()");
									}
								}
							}
						}
					}
				}

				}
			}
		}


/********************** 	PROTOCOLO A USAR 	*****************************/

void serializarEntrenador_Mapa(t_MensajeEntrenador_Mapa *value, char *buffer, int valueSize){
	int offset = 0;
	enum_procesos proceso = ENTRENADOR;

	//0)valuSize
	memcpy(buffer, &valueSize, sizeof(valueSize));
	offset += sizeof(valueSize);

	//1)from process
	memcpy(buffer + offset, &proceso, sizeof(proceso));
	offset += sizeof(proceso);

	//2)operacion
	memcpy(buffer + offset, &value->operacion, sizeof(value->operacion));
	offset += sizeof(value->operacion);

	//3)nombreEntrenadorLen
	int nombreEntrenadorLen = strlen(value->nombreEntrenador) + 1;
	memcpy(buffer + offset, &nombreEntrenadorLen, sizeof(nombreEntrenadorLen));
	offset += sizeof(nombreEntrenadorLen);

	//4)nombreEntrenador
	memcpy(buffer + offset, value->nombreEntrenador, nombreEntrenadorLen);

}

void deserializarMapa_Entrenador(t_MensajeEntrenador_Mapa *value, char *bufferReceived){
	int offset = 0;

	//2)operacion
	memcpy(&value->operacion, bufferReceived, sizeof(value->operacion));
	offset += sizeof(value->operacion);

	//3)nombreEntrenadorLen
	int nombreEntrenadorLen = 0;
	memcpy(&nombreEntrenadorLen, bufferReceived + offset, sizeof(nombreEntrenadorLen));
	offset += sizeof(nombreEntrenadorLen);

	//4)nombreEntrenador
	value->nombreEntrenador = malloc(nombreEntrenadorLen);
	memcpy(value->nombreEntrenador, bufferReceived + offset, nombreEntrenadorLen);


}


void serializarMapa_Entrenador(t_MensajeMapa_Entrenador *value, char *buffer, int valueSize){
	int offset = 0;
	enum_procesos proceso = MAPA;

	//0)valueSize
	memcpy(buffer, &valueSize, sizeof(valueSize));
	offset += sizeof(valueSize);

	//1)from process
	memcpy(buffer + offset, &proceso, sizeof(proceso));
	offset += sizeof(proceso);

	//2)operacion
	memcpy(buffer + offset, &value->operacion, sizeof(value->operacion));
	offset += sizeof(value->operacion);

	//3)programCounter
	memcpy(buffer + offset, &value->programCounter, sizeof(value->programCounter));
	offset += sizeof(value->programCounter);

	//5)quantum
	memcpy(buffer + offset, &value->quantum, sizeof(value->quantum));
	offset += sizeof(value->quantum);



}

void deserializarEntrenador_Mapa(t_MensajeMapa_Entrenador *value, char *bufferReceived){
	int offset = 0;

	//2)operacion
	memcpy(&value->operacion, bufferReceived, sizeof(value->operacion));
	offset += sizeof(value->operacion);

	//3)programCounter
	memcpy(&value->programCounter, bufferReceived + offset, sizeof(value->programCounter));
	offset += sizeof(value->programCounter);

	//5)quantum
	memcpy(&value->quantum, bufferReceived + offset, sizeof(value->quantum));
	offset += sizeof(value->quantum);

}

void serializarPokedexClient_PokedexServer(t_MensajePokedexClient_PokedexServer *value, char *buffer, int valueSize){
	int offset = 0;
	enum_procesos proceso = POKEDEX_CLIENT;

	//0)valueSize
	memcpy(buffer, &valueSize, sizeof(valueSize));
	offset += sizeof(valueSize);

	//1)from process
	memcpy(buffer + offset, &proceso, sizeof(proceso));
	offset += sizeof(proceso);

	//2)operacion
	memcpy(buffer + offset, &value->operacion, sizeof(value->operacion));
	offset += sizeof(value->operacion);

}

void deserializarPokedexServer_PokedexClient(t_MensajePokedexClient_PokedexServer *value, char *bufferReceived){
	int offset = 0;

	//2)operacion
	memcpy(&value->operacion, bufferReceived, sizeof(value->operacion));
	offset += sizeof(value->operacion);

}

void serializarPokedexServer_PokedexClient(t_MensajePokedexServer_PokedexClient *value, char *buffer, int valueSize) {
	int offset = 0;
	enum_procesos proceso = POKEDEX_SERVER;

	//0)valueSize
	memcpy(buffer, &valueSize, sizeof(valueSize));
	offset += sizeof(valueSize);

	//1)from process
	memcpy(buffer + offset, &proceso, sizeof(proceso));
	offset += sizeof(proceso);


}

void deserializarPokedexCliente_PokedexServer(t_MensajePokedexServer_PokedexClient *value, char * bufferReceived) {
	int offset = 0;

}


