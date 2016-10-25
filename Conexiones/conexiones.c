
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
		printf("Fallo en el accept");
	}else{
		printf("Me pude conectar\n");//todo eliminar el printf
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
	int conexion = connect(socketAConectarse,(struct sockaddr*)&dest_addr,sizeof(struct sockaddr));
	if (conexion != -1) {
		printf("me pude conectar");
	}else{
		return conexion;
	}
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

}

void deserializarEntrenador_Mapa(t_MensajeMapa_Entrenador *value, char *bufferRecibido){
	int offset = 0;

	//2)operacion
	memcpy(&value->operacion, bufferRecibido, sizeof(value->operacion));
	offset += sizeof(value->operacion);

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


void serializarCadena(char* cadena, char* buffer){
	int offset = 0;

	//1) cadenaLen
	int cadenaLen = strlen(cadena) + 1;
	memcpy(buffer + offset, &cadenaLen, sizeof(cadenaLen));
	offset += sizeof(cadenaLen);

	//2) cadena
	memcpy(buffer + offset, cadena, cadenaLen);

}

void deserializarCadena(char* cadena, char* bufferRecibido){
	int offset = 0;
	//1)cadenaLen
	int cadenaLen = strlen(cadena);
	memcpy(&cadenaLen, bufferRecibido + offset, sizeof(cadenaLen));
	offset += sizeof(cadenaLen);

	//2)cadena
	cadena = malloc(cadenaLen);
	memcpy(cadena, bufferRecibido + offset, cadenaLen);
}

void serializarMensajeLeerArchivo(char* buffer,t_MensajeLeerPokedexClient_PokedexServer* infoASerializar){
	int offset=0;

	//Se carga el offset desde donde se comenzara a leer el archivo
	memcpy(buffer+offset,&infoASerializar->offset,sizeof(int));
	offset+=sizeof(int);

	//Se carga la cantidad de bytes a escribir
	memcpy(buffer+offset,&infoASerializar->cantidadDeBytes,sizeof(int));
	offset+=sizeof(int);

	//Se carga el tamaño del archivo a serializar
	memcpy(buffer+offset,&infoASerializar->buffer, strlen(buffer));
	offset+=strlen(buffer);

	//Se carga el tamanio y la ruta del archivo
	int tamanioRuta = strlen(infoASerializar->rutaArchivo);
	memcpy(buffer+offset,&tamanioRuta, sizeof(int));
	offset+=sizeof(int);

	memcpy(buffer+offset,&infoASerializar->rutaArchivo,tamanioRuta);



}

void serializarMensajeCrearArchivo(char* buffer, t_MensajeCrearArchivoPokedexClient_PokedexServer* infoASerializar){

	int offset=0;

	//Se carga la ruta del archivo a crear como ultimo parametro su nombre
	memcpy(buffer+offset,infoASerializar->rutaDeArchivoACrear,strlen(infoASerializar->rutaDeArchivoACrear));
	offset+=strlen(infoASerializar->rutaDeArchivoACrear);



}

void serializarMensajeEscribirOModificarArchivo(char*buffer, t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoASerializar){

	int offset=0;

	//Se carga la ruta del archivo a modificar
	memcpy(buffer+offset,infoASerializar->rutaArchivo,strlen(infoASerializar->rutaArchivo));
	offset+=strlen(infoASerializar->rutaArchivo);

	//Se carga el buffer de escritura
	memcpy(buffer+offset,infoASerializar->bufferAEscribir, strlen(infoASerializar->bufferAEscribir));
	offset+=strlen(infoASerializar->bufferAEscribir);

	//Se carga desde donde se comenzara a escribir en el archivo
	memcpy(buffer+offset,&infoASerializar->offset,sizeof(int));
	offset+=sizeof(int);

	//Se carga la cantidad de bytes a escribir en el archivo
	memcpy(buffer+offset,&infoASerializar->cantidadDeBytes,sizeof(int));
	offset+=sizeof(int);



}

void serializarMensajeBorrarArchivo(char*buffer, t_MensajeBorrarArchivoPokedexClient_PokedexServer* infoASerializar){

	int offset = 0;

	//Se carga la ruta del archivo a borrar
	memcpy(buffer+offset, infoASerializar->rutaArchivoABorrar, strlen(infoASerializar->rutaArchivoABorrar));
	offset+=strlen(infoASerializar->rutaArchivoABorrar);


}

void serializarMensajeCrearDirectorio(char*buffer, t_MensajeCrearDirectorioPokedexClient_PokedexServer* infoASerializar){
	int offset = 0;

	//Se carga la ruta a crear
	memcpy(buffer+offset, infoASerializar->rutaDirectorioPadre,strlen(infoASerializar->rutaDirectorioPadre));
	offset+=strlen(infoASerializar->rutaDirectorioPadre);


}
void serializarMensajeBorrarDirectorio(char*buffer, t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* infoASerializar){
	int offset =0;

	//Se carga el directorio a borrar
	memcpy(buffer+offset, infoASerializar->rutaDirectorioABorrar, strlen(infoASerializar->rutaDirectorioABorrar));
	offset+=strlen(infoASerializar->rutaDirectorioABorrar);


}

void serializarMensajeRenombrarArchivo(char*buffer, t_MensajeRenombrarArchivoPokedexClient_PokedexServer* infoASerializar){
	int offset = 0;

	//Se carga la ruta del archivo a Renombrar
	memcpy(buffer+offset, infoASerializar->rutaDeArchivo, strlen(infoASerializar->rutaDeArchivo));
	offset+=strlen(infoASerializar->rutaDeArchivo);

	//Se carga el nuevo nombre del archivo
	memcpy(buffer+offset, infoASerializar->nuevoNombre, strlen(infoASerializar->nuevoNombre));
	offset+=strlen(infoASerializar->nuevoNombre);


}
void deserializarMensajeLeerArchivo(char* bufferRecibido,t_MensajeLeerPokedexClient_PokedexServer* infoASerializar){

	int offset = 0;

	//Se carga por referencia el offset donde comenzara a leer el archivo
	memcpy(&infoASerializar->offset, bufferRecibido+offset, sizeof(int));
	offset+=sizeof(int);

	//Se carga por referencia la cantidad de bytes a escribir en el archivo
	memcpy(&infoASerializar->cantidadDeBytes, bufferRecibido+offset, sizeof(int));
	offset+=sizeof(int);

	//Se carga por referencia el tamaño del buffer
	memcpy(&infoASerializar->buffer, bufferRecibido+offset, strlen(infoASerializar->buffer));
	offset+=strlen(infoASerializar->buffer);

	//Se carga el tamanio y por referencia la ruta del archivo
	int tamanioRuta = strlen(infoASerializar->rutaArchivo);
	memcpy(&tamanioRuta, bufferRecibido+offset, sizeof(int));
	offset+=sizeof(int);

	memcpy(&infoASerializar->rutaArchivo,bufferRecibido+offset,tamanioRuta);

}
void deserializarMensajeCrearArchivo(char* bufferRecibido, t_MensajeCrearArchivoPokedexClient_PokedexServer* infoASerializar){

	int offset=0;

	//Se carga por referencia la ruta del archivo a crear como ultimo parametro su nombre
	memcpy(&infoASerializar->rutaDeArchivoACrear,bufferRecibido+offset,strlen(infoASerializar->rutaDeArchivoACrear));
	offset+=strlen(infoASerializar->rutaDeArchivoACrear);


}

void deserializarMensajeEscribirOModificarArchivo(char*bufferRecibido, t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoASerializar){

	int offset = 0;

	//Se carga por referencia la ruta del archivo a modificar
	memcpy(&infoASerializar->rutaArchivo,bufferRecibido+offset,strlen(infoASerializar->rutaArchivo));
	offset+=strlen(infoASerializar->rutaArchivo);

	//Se carga por referencia el buffer de escritura
	memcpy(&infoASerializar->bufferAEscribir,bufferRecibido+offset, strlen(infoASerializar->bufferAEscribir));
	offset+=strlen(infoASerializar->bufferAEscribir);

	//Se carga por referencia desde donde se comenzara a escribir en el archivo
	memcpy(&infoASerializar->offset,bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);

	//Se carga por referencia la cantidad de bytes a escribir en el archivo
	memcpy(&infoASerializar->cantidadDeBytes,bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);


}
void deserializarMensajeBorrarArchivo(char*bufferRecibido, t_MensajeBorrarArchivoPokedexClient_PokedexServer* infoASerializar){

	int offset = 0;

	//Se carga por referencia la ruta del archivo a borrar
	memcpy(&infoASerializar->rutaArchivoABorrar,bufferRecibido+offset, strlen(infoASerializar->rutaArchivoABorrar));
	offset+=strlen(infoASerializar->rutaArchivoABorrar);



}
void deserializarMensajeCrearDirectorio(char*bufferRecibido, t_MensajeCrearDirectorioPokedexClient_PokedexServer* infoASerializar){

	int offset = 0;

	//Se carga la ruta a crear
	memcpy(&infoASerializar->rutaDirectorioPadre,bufferRecibido+offset,strlen(infoASerializar->rutaDirectorioPadre));
	offset+=strlen(infoASerializar->rutaDirectorioPadre);


}
void deserializarMensajeBorrarDirectorio(char*bufferRecibido, t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* infoASerializar){

	int offset =0;

	//Se carga por referencia el directorio a borrar
	memcpy(&infoASerializar->rutaDirectorioABorrar,bufferRecibido+offset,strlen(infoASerializar->rutaDirectorioABorrar));
	offset+=strlen(infoASerializar->rutaDirectorioABorrar);




}
void deserializarMensajeRenombrarArchivo(char*bufferRecibido, t_MensajeRenombrarArchivoPokedexClient_PokedexServer* infoASerializar){

	int offset = 0;

	//Se carga por referencia la ruta del archivo a Renombrar
	memcpy(&infoASerializar->rutaDeArchivo,bufferRecibido+offset, strlen(infoASerializar->rutaDeArchivo));
	offset+=strlen(infoASerializar->rutaDeArchivo);

	//Se carga por referencia el nuevo nombre del archivo
	memcpy(&infoASerializar->nuevoNombre,bufferRecibido+offset, strlen(infoASerializar->nuevoNombre));
	offset+=strlen(infoASerializar->nuevoNombre);

}



void enviarPokemon(int socket, t_pokemon* pokemonDeLista){
		t_pokemon* pokemon = malloc(sizeof(t_pokemon));
		pokemon->level = pokemonDeLista->level;
		string_append(&pokemonDeLista->species, "\0");
		pokemon->species = string_new();
		pokemon->species = pokemonDeLista->species;
		pokemon->type = pokemonDeLista->type;
		pokemon->second_type = pokemonDeLista->second_type;
		//string_append(&pokemon->species, species);
		int speciesLen = strlen(pokemon->species) + 1;

		int payloadSize= sizeof(pokemon->level) + sizeof(pokemon->type) + sizeof(pokemon->second_type)
				+ sizeof(speciesLen) + speciesLen;
		int bufferSize= sizeof(bufferSize) + payloadSize;

		// Serializar y enviar al ENTRENADOR
		char* bufferAEnviar = malloc(bufferSize);
		serializarPokemon(pokemon, bufferAEnviar,payloadSize);
		enviar(&socket, bufferAEnviar, bufferSize);

		//free(pokemon->species);
		//free(pokemon);
		free(bufferAEnviar);
}

void serializarPokemon(t_pokemon* value, char* buffer, int valueSize){
	int offset = 0;

	//0) valueSize
	memcpy(buffer, &valueSize, sizeof(valueSize));
	offset += sizeof(valueSize);

	//1)level
	memcpy(buffer + offset, &value->level, sizeof(value->level));
	offset += sizeof(value->level);

	//2) type
	memcpy(buffer + offset, &value->type, sizeof(value->type));
	offset += sizeof(value->type);

	//3) second_type
	memcpy(buffer + offset, &value->second_type, sizeof(value->second_type));
	offset += sizeof(value->second_type);

	//4) species length
	int speciesLen = strlen(value->species) + 1;
	memcpy(buffer + offset, &speciesLen, sizeof(speciesLen));
	offset += sizeof(speciesLen);

	//5) species
	memcpy(buffer + offset, value->species, speciesLen);

}

t_pokemon* recibirPokemon(int socket){
	t_pokemon* pokemon = malloc(sizeof(t_pokemon));
	pokemon->species = string_new();

	int sizeDatosPokemon = 0;
	recibir(&socket, &sizeDatosPokemon, sizeof(int));

	char *datosPokemon = malloc(sizeDatosPokemon);
	recibir(&socket, datosPokemon, sizeDatosPokemon);
	deserializarPokemon(pokemon, datosPokemon);

	free(datosPokemon);
	return pokemon;
}

void deserializarPokemon(t_pokemon* datos, char* bufferReceived) {
	int offset = 0;

	//1) level
	memcpy(&datos->level, bufferReceived , sizeof(datos->level));
	offset += sizeof(datos->level);

	//2) type
	memcpy(&datos->type, bufferReceived + offset, sizeof(datos->type));
	offset += sizeof(datos->type);

	//3) second_type
	memcpy(&datos->second_type, bufferReceived + offset, sizeof(datos->second_type));
	offset += sizeof(datos->second_type);

	//4) species length.
	int speciesLen = 0;
	memcpy(&speciesLen, bufferReceived + offset, sizeof(speciesLen));
	offset += sizeof(speciesLen);

	//5) species
	datos->species = malloc(speciesLen);
	memcpy(datos->species, bufferReceived + offset, speciesLen);

}

