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
#include <assert.h>

#include "pkmn/battle.h"
#include <pkmn/factory.h>

typedef struct {
	int socketServer;
	int socketCliente;
} t_server;

typedef struct {
	int socketServer;
	int socketClient;
} t_datosConexion;

typedef struct{
	char* nombreEntrenador;
	char id; //simbolo
	int operacion;
	char objetivoActual;
} t_MensajeEntrenador_Mapa;

typedef struct {
	int nombreLen;
	char* nombreArchivo;
	int pathLen;
	char* pathPokemon;
} t_contextoPokemon;

////OPERACIONES DE FILE SYSTEM////

typedef struct{
	int operacion;
	int tamanioBuffer;

}t_pedidoPokedexCliente;

//LEER ARCHIVO
typedef struct{
	int tamanioRuta;
	const char* rutaArchivo;
	int offset;
	int cantidadDeBytes;
	char* buffer;
}__attribute__((packed))
t_MensajeLeerPokedexClient_PokedexServer;


//CREAR ARCHIVO
typedef struct{
	int tamanioRuta;
	char* rutaDeArchivoACrear;

}__attribute__((packed))
t_MensajeCrearArchivoPokedexClient_PokedexServer;


//ESCRIBIR ARCHIVO
typedef struct{
	int tamanioRuta;
	char* rutaArchivo;
	char* bufferAEscribir;
	int offset;
	int cantidadDeBytes;
}__attribute__((packed))
t_MensajeEscribirArchivoPokedexClient_PokedexServer;

//BORRAR ARCHIVO
typedef struct{
	int tamanioRuta;
	char* rutaArchivoABorrar;
}__attribute__((packed))
t_MensajeBorrarArchivoPokedexClient_PokedexServer;

//CREAR DIRECTORIO
typedef struct{
	int tamanioRuta;
	char* rutaDirectorioPadre;

}__attribute__((packed))
t_MensajeCrearDirectorioPokedexClient_PokedexServer;

//BORRAR DIRECTORIO VACIO
typedef struct{
	int tamanioRuta;
	char* rutaDirectorioABorrar;
}__attribute__((packed))
t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer;

//RENOMBRAR ARCHIVO
typedef struct{
	int tamanioRuta;
	char* rutaDeArchivo;
	int tamanioNuevaRuta;
	char* nuevaRuta;
}__attribute__((packed))
t_MensajeRenombrarArchivoPokedexClient_PokedexServer;


//IMPORTANTE --> Nomeclatura de Serializadores y Deserealizadores
//1) serializar<DesdeProceso>_<HastaProceso> ()
//2) deserialiar<HastaProceso>_<DesdeProceso> ()

void serializarEntrenador_Mapa(t_MensajeEntrenador_Mapa* value, char *buffer);
void deserializarMapa_Entrenador(t_MensajeEntrenador_Mapa* value, char *bufferReceived);

void serializarOperaciones(void* buffer, t_pedidoPokedexCliente* operacion);
void deserializarOperaciones(void* buffer, t_pedidoPokedexCliente* operacion);

void serializarMensajeLeerArchivo(void* buffer,t_MensajeLeerPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeCrearArchivo(void* buffer, t_MensajeCrearArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeEscribirOModificarArchivo(void* buffer, t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeBorrarArchivo(void* buffer, t_MensajeBorrarArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeCrearDirectorio(void* buffer, t_MensajeCrearDirectorioPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeBorrarDirectorio(void* buffer, t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeRenombrarArchivo(void* buffer, t_MensajeRenombrarArchivoPokedexClient_PokedexServer* infoASerializar);

void deserializarMensajeLeerArchivo(void* buffer,t_MensajeLeerPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeCrearArchivo(void* buffer, t_MensajeCrearArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeEscribirOModificarArchivo(void* buffer, t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeBorrarArchivo(void* buffer, t_MensajeBorrarArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeCrearDirectorio(void* buffer, t_MensajeCrearDirectorioPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeBorrarDirectorio(void* buffer, t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeRenombrarArchivo(void* buffer, t_MensajeRenombrarArchivoPokedexClient_PokedexServer* infoASerializar);


int ponerAEscuchar(char* ipServer, int puertoServidor);
int enviar(int* socketAlQueEnvio, void* envio,int tamanioDelEnvio);
int recibir(int* socketReceptor, void* bufferReceptor,int tamanioQueRecibo);
int conectarseA(char* ipDestino,int puertoDestino);
int escucharMultiplesConexiones(int* socketEscucha,int puertoEscucha);

void abrirConexionDelServer(char* ipServer, int puertoServidor,int* socketServidor);
void aceptarConexionDeUnCliente(int* socketCliente,int* socketServidor);
void aceptarConexionDeUnClienteHilo(t_server* parametro);

void enviarPokemon(int socketEntrenador, t_pokemon* pokemonDeLista);
void serializarPokemon(t_pokemon* value, char* buffer, int valueSize);
t_pokemon* recibirPokemon(int socketMapa);
void deserializarPokemon(t_pokemon* datos, char* bufferReceived);

void enviarContextoPokemon(int socket, t_contextoPokemon* contextoDeLista);
void serializarContextoPokemon(t_contextoPokemon* value, char* buffer, int valueSize);
void recibirContextoPokemon(int socket, t_contextoPokemon* contextoPokemon);
void deserializarContextoPokemon(t_contextoPokemon* datos, char* bufferReceived);

#endif /*SOCKET_H_*/
