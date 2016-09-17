
#include "conexiones.h"

#define MYPORT 3490

void abrirConexionDelServer(char* ipServer, int puertoServidor,int* socketServidor){
	*socketServidor=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in socketInfo;
	socketInfo.sin_family=AF_INET;
	socketInfo.sin_port=htons(puertoServidor);
	socketInfo.sin_addr.s_addr = inet_addr(ipServer);
	//printf("%s \n",inet_ntoa(socketInfo.sin_addr));
	int socketActivo = 1;
	setsockopt(*socketServidor, SOL_SOCKET, SO_REUSEADDR, &socketActivo, sizeof(socketActivo));
	if(bind(*socketServidor,(struct sockaddr*)&socketInfo,sizeof(struct sockaddr))!=0){
		perror("Fallo el bindeo de la conexion");
		//printf("Revisar que el puerto no este en uso");
		close(*socketServidor);
	}

	if(listen(*socketServidor,SOMAXCONN)==-1){
		perror("Fallo el listen");

	}
}



void aceptarConexionDeUnCliente(int* socketCliente,int* socketServidor){
	struct sockaddr_in  their_addr;
	unsigned int sin_size=sizeof(struct sockaddr_in);
	*socketCliente=accept(*socketServidor,(void*)&their_addr,&sin_size);
	if(*socketCliente==-1){
		//printf("Fallo en el accept");
	}else{
		//printf("Me pude conectar\n");
	}
}

void aceptarConexionDeUnClienteHilo(t_server* parametro){
	struct sockaddr_in  their_addr;
	unsigned int sin_size=sizeof(struct sockaddr_in);
	parametro->socketCliente=accept(parametro->socketServer,(void*)&their_addr,&sin_size);
	if(parametro->socketServer==-1){
		//printf("Fallo en el accept");
	}else{
		//printf("Pude aceptar la conexion\n");
	}
}

int ponerAEscuchar(char* ipServer ,int puertoServidor){
	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in socketInfo;
	int nuevoSocket;
	socketInfo.sin_family=AF_INET;
	socketInfo.sin_port=htons(puertoServidor);
	socketInfo.sin_addr.s_addr=inet_addr(ipServer);
	struct sockaddr_in  their_addr;
	unsigned int sin_size;
	//printf("%s \n",inet_ntoa(socketInfo.sin_addr));
	int socketActivo = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &socketActivo, sizeof(socketActivo));
	if(bind(sockfd,(struct sockaddr*)&socketInfo,sizeof(struct sockaddr))!=0){
		perror("Fallo el bindeo de la conexion");
		//printf("Revisar que el puerto no este en uso");
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
	int yes=1;
	struct sockaddr_in dest_addr;
	socketAConectarse=socket(AF_INET,SOCK_STREAM,0);
	dest_addr.sin_family=AF_INET;
	dest_addr.sin_port=htons(puertoDestino);
	dest_addr.sin_addr.s_addr=inet_addr(ipDestino);
	setsockopt(socketAConectarse,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
	memset(&(dest_addr.sin_zero),'\0',8);
	connect(socketAConectarse,(struct sockaddr*)&dest_addr,sizeof(struct sockaddr));
	return socketAConectarse;
}


int enviar(int* socketAlQueEnvio, void* envio,int tamanioDelEnvio){
	int bytesEnviados;
	bytesEnviados=send(*socketAlQueEnvio,envio,tamanioDelEnvio,0);
	return bytesEnviados;
}

int recibir(int* socketReceptor, void* bufferReceptor,int tamanioQueRecibo){
	int bytesRecibidos;
	bytesRecibidos=recv(*socketReceptor,bufferReceptor,tamanioQueRecibo,0);
	return bytesRecibidos;
}

int escucharMultiplesConexiones(int* socketEscucha,int puertoEscucha){
	fd_set master;
	fd_set read_Fs;
	struct sockaddr_in myAddr;
	struct sockaddr_in remoteAddr;
	int fdmax;
	int newfd;
	char buff[256];
	int nBytes;
	unsigned int addrlen;
	int i,j;
	struct timeval intervalo;
	intervalo.tv_sec=5;

	FD_ZERO(&master);
	FD_ZERO(&read_Fs);
	myAddr.sin_family=AF_INET;
	myAddr.sin_addr.s_addr=INADDR_ANY;
	myAddr.sin_port=htons(puertoEscucha);
	memset(&(myAddr.sin_zero),'\0',8);
	if(bind(*socketEscucha,(struct sockaddr*)&myAddr,sizeof(myAddr))==-1){
		printf("No se pudo bindear Correctamente");
		exit(1);
	}

	if(listen(*socketEscucha,10)==-1){
		printf("No se pudo poner a escuchar");
		exit(1);
	}

	FD_SET(*socketEscucha,&master);
	fdmax=*socketEscucha;
	for(;;){
		read_Fs=master;
		if(select(fdmax+1,&read_Fs,NULL,NULL,&intervalo)==-1){
			printf("Fallo la aplicacion de select()");
			exit(1);
		}

		for(i=0;i<=fdmax;i++){
			if(FD_ISSET(i,&read_Fs)){
				if(i==*socketEscucha){
					addrlen=sizeof(remoteAddr);
					newfd=accept(*socketEscucha,(struct sockaddr*)&remoteAddr,&addrlen);
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
								if(j!=*socketEscucha && j!=i){
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

//Antes de enviar cada struct serializada se debe enviar el fromProcess:
//enum_procesos proceso = [NOMBREPROCESO];
//enviar(&socketAlQueEnvio, &proceso, sizeof(int));

void serializarEntrenador_Mapa(t_MensajeEntrenador_Mapa* value, char *buffer){
	int offset = 0;

	//2)operacion
	memcpy(buffer + offset, &value->operacion, sizeof(value->operacion));
	offset += sizeof(value->operacion);

	//3)id(simbolo)
	memcpy(buffer + offset, &value->id, sizeof(value->id));
	offset += sizeof(value->id);

	//4)objetivoActual
	memcpy(buffer + offset, &value->objetivoActual, sizeof(value->objetivoActual));
	offset += sizeof(value->objetivoActual);

	//5)nombreEntrenadorLen
	int nombreEntrenadorLen = strlen(value->nombreEntrenador) + 1;
	memcpy(buffer + offset, &nombreEntrenadorLen, sizeof(nombreEntrenadorLen));
	offset += sizeof(nombreEntrenadorLen);

	//6)nombreEntrenador
	memcpy(buffer + offset, value->nombreEntrenador, nombreEntrenadorLen);

}

void deserializarMapa_Entrenador(t_MensajeEntrenador_Mapa* value, char *bufferRecibido){
	int offset = 0;

	//2)operacion
	memcpy(&value->operacion, bufferRecibido + offset, sizeof(value->operacion));
	offset += sizeof(value->operacion);

	//3)id(simbolo)
	memcpy(&value->id, bufferRecibido + offset, sizeof(value->id));
	offset += sizeof(value->id);

	//4)objetivoActual
	memcpy(&value->objetivoActual, bufferRecibido + offset, sizeof(value->objetivoActual));
	offset += sizeof(value->objetivoActual);

	//5)nombreEntrenadorLen
	int nombreEntrenadorLen = 0;
	memcpy(&nombreEntrenadorLen, bufferRecibido + offset, sizeof(nombreEntrenadorLen));
	offset += sizeof(nombreEntrenadorLen);

	//6)nombreEntrenador
	value->nombreEntrenador = malloc(nombreEntrenadorLen);
	memcpy(value->nombreEntrenador, bufferRecibido + offset, nombreEntrenadorLen);

}


void serializarMapa_Entrenador(t_MensajeMapa_Entrenador *value, char *buffer){
	int offset = 0;

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

void deserializarEntrenador_Mapa(t_MensajeMapa_Entrenador *value, char *bufferRecibido){
	int offset = 0;

	//2)operacion
	memcpy(&value->operacion, bufferRecibido, sizeof(value->operacion));
	offset += sizeof(value->operacion);

	//3)programCounter
	memcpy(&value->programCounter, bufferRecibido + offset, sizeof(value->programCounter));
	offset += sizeof(value->programCounter);

	//5)quantum
	memcpy(&value->quantum, bufferRecibido + offset, sizeof(value->quantum));
	offset += sizeof(value->quantum);

}

void serializarPokedexClient_PokedexServer(t_MensajePokedexClient_PokedexServer *value, char *buffer){
	int offset = 0;

	//2)operacion
	memcpy(buffer + offset, &value->operacion, sizeof(value->operacion));
	offset += sizeof(value->operacion);

}

void deserializarPokedexServer_PokedexClient(t_MensajePokedexClient_PokedexServer *value, char *bufferRecibido){
	int offset = 0;

	//2)operacion
	memcpy(&value->operacion, bufferRecibido, sizeof(value->operacion));
	offset += sizeof(value->operacion);

}

void serializarPokedexServer_PokedexClient(t_MensajePokedexServer_PokedexClient *value, char *buffer) {
	//int offset = 0;

}

void deserializarPokedexCliente_PokedexServer(t_MensajePokedexServer_PokedexClient *value, char * bufferRecibido) {
	//int offset = 0;

}
