
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

	/////////SERIALIZADORES Y DESERIALIZADORES POKEDEX//////////

void serializarOperaciones(void* buffer, t_pedidoPokedexCliente* operacion){
	size_t offset=0;


	//Se carga la operacion a Realizar
	memcpy(buffer+offset,&(operacion->operacion),sizeof(int));
	offset+=sizeof(int);

	//Se carga el tamaño del buffer a almacenar
	memcpy(buffer+offset,&(operacion->tamanioBuffer),sizeof(int));
	offset+=sizeof(int);


}

void serializarMensajeLeerArchivo(void* buffer, t_MensajeLeerPokedexClient_PokedexServer* infoASerializar){
	size_t offset=0;


	//Se carga el tamaño de la ruta del archivo
	memcpy(buffer+offset,&(infoASerializar->tamanioRuta),sizeof(int));
	offset+=sizeof(int);

	//Se carga el offset desde donde se comenzara a leer el archivo
	memcpy(buffer+offset,&(infoASerializar->offset),sizeof(int));
	offset+=sizeof(int);

	//Se carga la cantidad de bytes a escribir
	memcpy(buffer+offset,&(infoASerializar->cantidadDeBytes),sizeof(int));
	offset+=sizeof(int);

	//Se carga el tamaño del archivo a serializar
	memcpy(buffer+offset,(infoASerializar->buffer), infoASerializar->cantidadDeBytes);
	offset+=infoASerializar->cantidadDeBytes;

	//Se carga la ruta del archivo
	memcpy(buffer+offset,(infoASerializar->rutaArchivo),infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;


}

void serializarMensajeCrearArchivo(void* buffer,  t_MensajeCrearArchivoPokedexClient_PokedexServer* infoASerializar){

	size_t offset=0;

	//Se carga el tamaño de la ruta del archivo

	memcpy(buffer+offset,&(infoASerializar->tamanioRuta),sizeof(int));
	offset+=sizeof(int);


	//Se carga la ruta del archivo a crear como ultimo parametro su nombre
	memcpy(buffer+offset,(infoASerializar->rutaDeArchivoACrear),infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;



}

void serializarMensajeEscribirOModificarArchivo(void* buffer, t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoASerializar){

	size_t offset=0;

	//Se carga el tamaño de la ruta del archivo

	memcpy(buffer+offset,&(infoASerializar->tamanioRuta),sizeof(int));
	offset+=sizeof(int);

	//Se carga desde donde se comenzara a escribir en el archivo

	memcpy(buffer+offset,&(infoASerializar->offset),sizeof(int));
	offset+=sizeof(int);


	//Se carga la cantidad de bytes a escribir en el archivo

	memcpy(buffer+offset,&(infoASerializar->cantidadDeBytes),sizeof(int));
	offset+=sizeof(int);


	//Se carga la ruta del archivo a modificar

	memcpy(buffer+offset,(infoASerializar->rutaArchivo),infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;


	//Se carga el buffer de escritura

	memcpy(buffer+offset,(infoASerializar->bufferAEscribir), infoASerializar->cantidadDeBytes);
	offset+=infoASerializar->cantidadDeBytes;


}


void serializarMensajeBorrarArchivo(void* buffer, t_MensajeBorrarArchivoPokedexClient_PokedexServer* infoASerializar){

	size_t offset = 0;

	//Se carga el tamaño de la ruta del archivo
	memcpy(buffer+offset,&(infoASerializar->tamanioRuta),sizeof(int));
	offset+=sizeof(int);


	//Se carga la ruta del archivo a borrar
	memcpy(buffer+offset, (infoASerializar->rutaArchivoABorrar), infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;


}

void serializarMensajeCrearDirectorio(void* buffer, t_MensajeCrearDirectorioPokedexClient_PokedexServer* infoASerializar){
	size_t offset = 0;

	//Se carga el tamaño de la ruta del archivo
	memcpy(buffer+offset,&(infoASerializar->tamanioRuta),sizeof(int));
	offset+=sizeof(int);

	//Se carga la ruta a crear
	memcpy(buffer+offset, (infoASerializar->rutaDirectorioPadre),infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;


}
void serializarMensajeBorrarDirectorio(void* buffer, t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* infoASerializar){
	size_t offset =0;

	//Se carga el tamaño de la ruta del archivo
	memcpy(buffer+offset,&(infoASerializar->tamanioRuta),sizeof(int));
	offset+=sizeof(int);

	//Se carga el directorio a borrar
	memcpy(buffer+offset, (infoASerializar->rutaDirectorioABorrar), infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;


}

void serializarMensajeRenombrarArchivo(void* buffer, t_MensajeRenombrarArchivoPokedexClient_PokedexServer* infoASerializar){
	size_t offset = 0;

	//Se carga el tamaño de la ruta del archivo
	memcpy(buffer+offset,&(infoASerializar->tamanioRuta),sizeof(int));
	offset+=sizeof(int);


	//Se carga la ruta del archivo a Renombrar
	memcpy(buffer+offset, (infoASerializar->rutaDeArchivo), infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;

	//Se carga el tamaño de la ruta a donde se va a mover el archivo
	memcpy(buffer+offset,&(infoASerializar->tamanioNuevaRuta),sizeof(int));
	offset+=sizeof(int);

	//Se carga el nuevo nombre del archivo
	memcpy(buffer+offset, &(infoASerializar->nuevaRuta), 18);
	offset+=18;


}

void serializarMensajeListarArchivos(void* buffer, t_MensajeListarArchivosPokedexClient_PokedexServer* infoASerializar){

	size_t offset =0;

	//Se carga el tamaño de la ruta del archivo
	memcpy(buffer+offset,&(infoASerializar->tamanioRuta),sizeof(int));
	offset+=sizeof(int);

	//Se carga el directorio a borrar
	memcpy(buffer+offset, (infoASerializar->rutaDeArchivo), infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;


}

void serializarMensajeTruncarArchivo(void* buffer, t_MensajeTruncarArchivoPokedexClient_PokedexServer* infoASerializar){

	size_t offset =0;

	//Se carga el nuevo tamaño del archivo
	memcpy(buffer+offset,&(infoASerializar->nuevoTamanio),sizeof(int));
	offset+=sizeof(int);


	//Se carga el tamaño de la ruta del archivo
	memcpy(buffer+offset,&(infoASerializar->tamanioRuta),sizeof(int));
	offset+=sizeof(int);

	//Se carga la ruta del archivo a Truncar
	memcpy(buffer+offset, (infoASerializar->rutaDeArchivo), infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;



}

void serializarMensajeMoverArchivo(void* buffer, t_MensajeMoverArchivoPokedexClient_PokedexServer* infoASerializar){


		size_t offset = 0;

		//Se carga el tamaño de la ruta del archivo
		memcpy(buffer+offset,&(infoASerializar->tamanioRuta),sizeof(int));
		offset+=sizeof(int);


		//Se carga la ruta del archivo a Renombrar
		memcpy(buffer+offset, (infoASerializar->rutaDeArchivo), infoASerializar->tamanioRuta);
		offset+=infoASerializar->tamanioRuta;

		//Se carga el tamaño de la ruta a donde se va a mover el archivo
		memcpy(buffer+offset,&(infoASerializar->tamanioNuevaRuta),sizeof(int));
		offset+=sizeof(int);

		//Se carga el nuevo nombre del archivo
		memcpy(buffer+offset, (infoASerializar->nuevaRuta), infoASerializar->tamanioNuevaRuta);
		offset+=infoASerializar->tamanioNuevaRuta;



}

void deserializarOperaciones(void* bufferRecibido, t_pedidoPokedexCliente* pedido){

	size_t offset = 0;

	//Se carga por referencia la operacion a realizar
	memcpy(&(pedido->operacion),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);

	//Se carga por referencia el tamanio del buffer a recibir
	memcpy(&(pedido->tamanioBuffer), bufferRecibido+offset, sizeof(int));
	offset+=sizeof(int);



}


void deserializarMensajeLeerArchivo(void* bufferRecibido, t_MensajeLeerPokedexClient_PokedexServer* infoASerializar){

	size_t offset = 0;

	//Se carga por referencia el tamaño de la ruta
	memcpy(&(infoASerializar->tamanioRuta),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);

	//Se carga por referencia el offset donde comenzara a leer el archivo
	memcpy(&(infoASerializar->offset), bufferRecibido+offset, sizeof(int));
	offset+=sizeof(int);

	//Se carga por referencia la cantidad de bytes a escribir en el archivo
	memcpy(&(infoASerializar->cantidadDeBytes), bufferRecibido+offset, sizeof(int));
	offset+=sizeof(int);

	//Se carga por referencia el tamaño del buffer
	infoASerializar->buffer=malloc(infoASerializar->cantidadDeBytes);
	memcpy(infoASerializar->buffer, bufferRecibido+offset, infoASerializar->cantidadDeBytes);
	offset+=infoASerializar->cantidadDeBytes;

	//Se carga la ruta del archivo
	infoASerializar->rutaArchivo = malloc(infoASerializar->tamanioRuta);
	memcpy(infoASerializar->rutaArchivo,bufferRecibido+offset,infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;
}
void deserializarMensajeCrearArchivo(void* bufferRecibido, t_MensajeCrearArchivoPokedexClient_PokedexServer* infoASerializar){

	size_t offset=0;

	//Se carga por referencia el tamaño de la ruta
	memcpy(&(infoASerializar->tamanioRuta),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);

	//Se carga por referencia la ruta del archivo a crear como ultimo parametro su nombre
	infoASerializar->rutaDeArchivoACrear = malloc(infoASerializar->tamanioRuta);
	memcpy(infoASerializar->rutaDeArchivoACrear,bufferRecibido+offset,infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;


}

void deserializarMensajeEscribirOModificarArchivo(void* bufferRecibido, t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoASerializar){

	size_t offset = 0;

	//Se carga por referencia el tamaño de la ruta
	memcpy(&(infoASerializar->tamanioRuta),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);

	//Se carga por referencia desde donde se comenzara a escribir en el archivo
	memcpy(&(infoASerializar->offset),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);

	//Se carga por referencia la cantidad de bytes a escribir en el archivo
	memcpy(&(infoASerializar->cantidadDeBytes),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);

	//Se carga la ruta del archivo a modificar
	infoASerializar->rutaArchivo=malloc(infoASerializar->tamanioRuta);
	memcpy(infoASerializar->rutaArchivo,bufferRecibido+offset,infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;

	//Se carga el buffer de escritura
	infoASerializar->bufferAEscribir = malloc(infoASerializar->cantidadDeBytes);
	memcpy(infoASerializar->bufferAEscribir,bufferRecibido+offset, infoASerializar->cantidadDeBytes);
	offset+=infoASerializar->cantidadDeBytes;




}
void deserializarMensajeBorrarArchivo(void* bufferRecibido, t_MensajeBorrarArchivoPokedexClient_PokedexServer* infoASerializar){

	size_t offset = 0;

	//Se carga por referencia el tamaño de la ruta
	memcpy(&(infoASerializar->tamanioRuta),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);


	//Se carga por referencia la ruta del archivo a borrar
	infoASerializar->rutaArchivoABorrar = malloc(infoASerializar->tamanioRuta);
	memcpy(infoASerializar->rutaArchivoABorrar,bufferRecibido+offset, infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;



}
void deserializarMensajeCrearDirectorio(void* bufferRecibido, t_MensajeCrearDirectorioPokedexClient_PokedexServer* infoASerializar){

	size_t offset = 0;

	//Se carga por referencia el tamaño de la ruta
	memcpy(&(infoASerializar->tamanioRuta),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);


	//Se carga la ruta a crear
	infoASerializar->rutaDirectorioPadre = malloc(infoASerializar->tamanioRuta);
	memcpy(infoASerializar->rutaDirectorioPadre,bufferRecibido+offset,infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;


}
void deserializarMensajeBorrarDirectorio(void* bufferRecibido, t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* infoASerializar){

	size_t offset =0;

	//Se carga por referencia el tamaño de la ruta
	memcpy(&(infoASerializar->tamanioRuta),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);


	//Se carga por referencia el directorio a borrar
	infoASerializar->rutaDirectorioABorrar = malloc(infoASerializar->tamanioRuta);
	memcpy(infoASerializar->rutaDirectorioABorrar,bufferRecibido+offset,infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;




}
void deserializarMensajeRenombrarArchivo(void* bufferRecibido, t_MensajeRenombrarArchivoPokedexClient_PokedexServer* infoASerializar){

	size_t offset = 0;

	//Se carga por referencia el tamaño de la ruta
	memcpy(&(infoASerializar->tamanioRuta),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);

	//Se carga la ruta del archivo a Renombrar
	infoASerializar->rutaDeArchivo = malloc(infoASerializar->tamanioRuta);
	memcpy(infoASerializar->rutaDeArchivo,bufferRecibido+offset, infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;

	//Se carga por referencia el tamaño de la ruta
	memcpy(&(infoASerializar->tamanioNuevaRuta),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);

	//Se carga la nueva ruta
	infoASerializar->nuevaRuta = malloc(infoASerializar->tamanioNuevaRuta);
	memcpy(infoASerializar->nuevaRuta,bufferRecibido+offset, infoASerializar->tamanioNuevaRuta);
	offset+=infoASerializar->tamanioNuevaRuta;

}

void deserializarMensajeListarArchivos(void* bufferRecibido, t_MensajeListarArchivosPokedexClient_PokedexServer* infoASerializar){

	size_t offset =0;

	//Se carga por referencia el tamaño de la ruta
	memcpy(&(infoASerializar->tamanioRuta),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);


	//Se carga el tamaño de la ruta a listar y se la copia
	infoASerializar->rutaDeArchivo = malloc(infoASerializar->tamanioRuta);
	memcpy(infoASerializar->rutaDeArchivo,bufferRecibido+offset,infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;


}

void deserializarMensajeTruncarArchivo(void* bufferRecibido, t_MensajeTruncarArchivoPokedexClient_PokedexServer* infoASerializar){

	size_t offset =0;

	//Se carga por referencia el nuevo tamaño del archivo
	memcpy(&(infoASerializar->nuevoTamanio),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);



	//Se carga por referencia el tamaño de la ruta
	memcpy(&(infoASerializar->tamanioRuta),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);


	//Se carga el tamaño de la ruta del archivo a truncar
	infoASerializar->rutaDeArchivo = malloc(infoASerializar->tamanioRuta);
	memcpy(infoASerializar->rutaDeArchivo,bufferRecibido+offset,infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;




}
void deserializarMensajeMoverArchivo(void* bufferRecibido, t_MensajeMoverArchivoPokedexClient_PokedexServer* infoASerializar){

	size_t offset = 0;

	//Se carga por referencia el tamaño de la ruta
	memcpy(&(infoASerializar->tamanioRuta),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);

	//Se carga la ruta del archivo a Renombrar
	infoASerializar->rutaDeArchivo = malloc(infoASerializar->tamanioRuta);
	memcpy(infoASerializar->rutaDeArchivo,bufferRecibido+offset, infoASerializar->tamanioRuta);
	offset+=infoASerializar->tamanioRuta;

	//Se carga por referencia el tamaño de la ruta
	memcpy(&(infoASerializar->tamanioNuevaRuta),bufferRecibido+offset,sizeof(int));
	offset+=sizeof(int);

	//Se carga la nueva ruta
	infoASerializar->nuevaRuta = malloc(infoASerializar->tamanioNuevaRuta);
	memcpy(infoASerializar->nuevaRuta,bufferRecibido+offset, infoASerializar->tamanioNuevaRuta);
	offset+=infoASerializar->tamanioNuevaRuta;

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

	//4) species length
	int speciesLen = 0;
	memcpy(&speciesLen, bufferReceived + offset, sizeof(speciesLen));
	offset += sizeof(speciesLen);

	//5) species
	datos->species = malloc(speciesLen);
	memcpy(datos->species, bufferReceived + offset, speciesLen);

}

void enviarContextoPokemon(int socket, t_contextoPokemon* contextoDeLista){
	int cadenasLen = 0;
	int nombreLen = 0;
	int textoLen = 0;
	memcpy(&nombreLen, &contextoDeLista->nombreLen, sizeof(int));
	memcpy(&textoLen, &contextoDeLista->textoLen, sizeof(int));
	cadenasLen = nombreLen + textoLen;

	int payloadSize = sizeof(cadenasLen) + cadenasLen + sizeof(int) * 2;
	int bufferSize = sizeof(bufferSize) + payloadSize;
	enviar(&socket, &payloadSize, sizeof(int));

	// Serializar y enviar al ENTRENADOR
	char* bufferAEnviar = malloc(bufferSize);
	serializarContextoPokemon(contextoDeLista, bufferAEnviar,payloadSize);
	enviar(&socket, bufferAEnviar, bufferSize);

	free(bufferAEnviar);
}

void serializarContextoPokemon(t_contextoPokemon* value, char* buffer, int valueSize){
	int offset = 0;

	//0) valueSize
	memcpy(buffer, &valueSize, sizeof(valueSize));
	offset += sizeof(valueSize);

	//1) nombreArchivo length
	int nombreArchivoLen = value->nombreLen;
	memcpy(buffer + offset, &nombreArchivoLen, sizeof(nombreArchivoLen));
	offset += sizeof(nombreArchivoLen);

	//2) nombreArchivo
	memcpy(buffer + offset, value->nombreArchivo, nombreArchivoLen);
	offset += nombreArchivoLen;

	//3) textoArch length
	int textoLen = value->textoLen;
	memcpy(buffer + offset, &textoLen, sizeof(textoLen));
	offset += sizeof(textoLen);

	//4) nombreArchivo
	memcpy(buffer + offset, value->textoArch, textoLen);

}

void recibirContextoPokemon(int socket, t_contextoPokemon* contextoPokemon){
	int sizeDatosPokemon = 0;
	recibir(&socket, &sizeDatosPokemon, sizeof(int));

	char *datosContextoPokemon = malloc(sizeDatosPokemon);
	recibir(&socket, datosContextoPokemon, sizeDatosPokemon);
	deserializarContextoPokemon(contextoPokemon, datosContextoPokemon);

	free(datosContextoPokemon);

}

void deserializarContextoPokemon(t_contextoPokemon* datos, char* bufferReceived) {
	int offset = 0;

	//1) nombreArchivo length
	memcpy(&datos->nombreLen, bufferReceived + offset, sizeof(datos->nombreLen));
	offset += sizeof(datos->nombreLen);

	//2) nombreArchivoLen
	datos->nombreArchivo = malloc(datos->nombreLen);
	memcpy(datos->nombreArchivo, bufferReceived + offset, datos->nombreLen);
	offset += datos->nombreLen;

	//3) textoArch length
	memcpy(&datos->textoLen, bufferReceived + offset, sizeof(datos->textoLen));
	offset += sizeof(datos->textoLen);

	//4) textoArch
	datos->textoArch = malloc(datos->textoLen);
	memcpy(datos->textoArch, bufferReceived + offset, datos->textoLen);

}
