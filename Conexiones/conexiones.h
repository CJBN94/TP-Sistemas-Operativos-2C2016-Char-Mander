#ifndef SOCKET_H_
#define SOCKET_H_

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
#include <commons/string.h>
#include <pthread.h>

typedef struct {
	int socketServer;
	struct sockaddr_in addr;
	int tamanioDireccion;
} t_server;

typedef enum{
	ACCEPTED=0,
	ENTRENADOR,
	MAPA,
	POKEDEX_CLIENT,
	POKEDEX_SERVER
} enum_procesos;

typedef struct {
	int socketServer;
	int socketClient;
} t_datosConexion;


typedef struct{
	enum_procesos proceso;
	char *mensaje;
} t_MessageGenericHandshake;


typedef struct{
	int operacion;
	int programCounter;
	int quantum;
} t_MensajeMapa_Entrenador;

typedef struct{
	char* nombreEntrenador;
	int operacion;
} t_MensajeEntrenador_Mapa;

typedef struct{
	int operacion;
} t_MensajePokedexClient_PokedexServer;

typedef struct{
	int operacion;
} t_MensajePokedexServer_PokedexClient;

//IMPORTANTE --> Nomeclatura de Serializadores y Deserealizadores
//1) serializar<DesdeProceso>_<HastaProceso> ()
//2) deserialiar<HastaProceso>_<DesdeProceso> ()

void serializarEntrenador_Mapa(t_MensajeEntrenador_Mapa *value, char *buffer, int valueSize);
void deserializarMapa_Entrenador(t_MensajeEntrenador_Mapa *value, char *bufferReceived);

void serializarMapa_Entrenador(t_MensajeMapa_Entrenador *value, char *buffer, int valueSize);
void deserializarEntrenador_Mapa(t_MensajeMapa_Entrenador *value, char *bufferReceived);

void serializarPokedexClient_PokedexServer(t_MensajePokedexClient_PokedexServer *value, char *buffer, int valueSize);
void deserializarPokedexServer_PokedexClient(t_MensajePokedexClient_PokedexServer *value, char *bufferReceived);

void serializarPokedexServer_PokedexClient(t_MensajePokedexServer_PokedexClient *value, char *buffer, int valueSize);
void deserializarPokedexCliente_PokedexServer(t_MensajePokedexServer_PokedexClient *value, char * bufferReceived);

void serializeHandShake(t_MessageGenericHandshake *value, char *buffer, int valueSize);
void deserializeHandShake(t_MessageGenericHandshake *value, char *bufferReceived);


int ponerAEscuchar(int puertoServidor);
int enviar(int* socketAlQueEnvio, void* envio,int tamanioDelEnvio);
int recibir(int* socketReceptor, void* bufferReceptor,int tamanioQueRecibo);
int conectarseA(char* ipDestino,int puertoDestino);
int escucharMultiplesConexiones(int socketEscucha,int puertoEscucha);
char *getprocessString (enum_procesos proceso);
int sendClientAcceptation(int *socketClient);
int sendClientHandShake(int *socketClient, enum_procesos proceso);
int openServerConnection(int newSocketServerPort, int *socketServer);
int acceptClientConnection(int *socketServer, int *socketClient);
int sendClientHandShake(int *socketClient, enum_procesos proceso);
int openClientConnection(char *IPServer, int PortServer, int *socketClient);

#endif /*SOCKET_H_*/
