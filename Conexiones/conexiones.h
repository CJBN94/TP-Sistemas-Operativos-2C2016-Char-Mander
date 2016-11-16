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
	char* pathArchivo;
} t_contextoPokemon;

////OPERACIONES DE FILE SYSTEM////

typedef struct{
	int operacion;
	int tamanioBuffer;

}t_pedidoPokedexCliente;

////MENSAJES A POKEDEX SERVER/////


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

//LISTAR ARCHIVOS
typedef struct{

	int tamanioRuta;
	const char* rutaDeArchivo;

}__attribute__((packed))
t_MensajeListarArchivosPokedexClient_PokedexServer;

//TRUNCAR ARCHIVO

typedef struct{

	int nuevoTamanio;
	int tamanioRuta;
	char* rutaDeArchivo;


}__attribute__((packed))
t_MensajeTruncarArchivoPokedexClient_PokedexServer;

typedef struct{
	int tamanioRuta;
	char* rutaDeArchivo;
	int tamanioNuevaRuta;
	char* nuevaRuta;
}__attribute__((packed))
t_MensajeMoverArchivoPokedexClient_PokedexServer;

typedef struct{
	int tamanioRuta;
	const char* rutaArchivo;
}t_MensajeAtributosArchivoPokedexClient_PokedexServer;


////MENSAJES A POKEDEX CLIENTE////

typedef struct{
	int estado;
	int tamanio;

}t_MensajeAtributosArchivoPokedexServer_PokedexClient;

typedef struct{
	int resultado;
	int tamanio;

}t_RespuestaPokedexCliente;




//IMPORTANTE --> Nomeclatura de Serializadores y Deserealizadores
//1) serializar<DesdeProceso>_<HastaProceso> ()
//2) deserialiar<HastaProceso>_<DesdeProceso> ()

void serializarEntrenador_Mapa(t_MensajeEntrenador_Mapa* value, char *buffer, int payloadSize);
void deserializarMapa_Entrenador(t_MensajeEntrenador_Mapa* value, char *bufferReceived);

void serializarOperaciones(void* buffer, t_pedidoPokedexCliente* operacion);
void deserializarOperaciones(void* buffer, t_pedidoPokedexCliente* operacion);

void serializarRespuestaOperaciones(void* buffer, t_RespuestaPokedexCliente* operacion);
void deserializarRespuestaOperaciones(void* buffer, t_RespuestaPokedexCliente* operacion);

void serializarMensajeLeerArchivo(void* buffer,t_MensajeLeerPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeCrearArchivo(void* buffer, t_MensajeCrearArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeEscribirOModificarArchivo(void* buffer, t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeBorrarArchivo(void* buffer, t_MensajeBorrarArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeCrearDirectorio(void* buffer, t_MensajeCrearDirectorioPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeBorrarDirectorio(void* buffer, t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeRenombrarArchivo(void* buffer, t_MensajeRenombrarArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeListarArchivos(void* buffer, t_MensajeListarArchivosPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeTruncarArchivo(void* buffer, t_MensajeTruncarArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeMoverArchivo(void* buffer, t_MensajeMoverArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeAtributosArchivo(void* buffer, t_MensajeAtributosArchivoPokedexClient_PokedexServer* infoASerializar);

void deserializarMensajeLeerArchivo(void* buffer,t_MensajeLeerPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeCrearArchivo(void* buffer, t_MensajeCrearArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeEscribirOModificarArchivo(void* buffer, t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeBorrarArchivo(void* buffer, t_MensajeBorrarArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeCrearDirectorio(void* buffer, t_MensajeCrearDirectorioPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeBorrarDirectorio(void* buffer, t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeRenombrarArchivo(void* buffer, t_MensajeRenombrarArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeListarArchivos(void* buffer, t_MensajeListarArchivosPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeTruncarArchivo(void* buffer, t_MensajeTruncarArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeMoverArchivo(void* buffer, t_MensajeMoverArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeAtributosArchivo(void* buffer, t_MensajeAtributosArchivoPokedexClient_PokedexServer* infoASerializar);


//POKEDEX SERVER

void serializarAtributos(void* buffer, t_MensajeAtributosArchivoPokedexServer_PokedexClient* atributos);

void deserializarAtributos(void* buffer, t_MensajeAtributosArchivoPokedexServer_PokedexClient* atributos);


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
void recibirPokemon(int socket, t_pokemon* pokemon);
void deserializarPokemon(t_pokemon* datos, char* bufferReceived);

void enviarContextoPokemon(int socket, t_contextoPokemon* contextoDeLista);
void serializarContextoPokemon(t_contextoPokemon* value, char* buffer, int valueSize);
void recibirContextoPokemon(int socket, t_contextoPokemon* contextoPokemon);
void deserializarContextoPokemon(t_contextoPokemon* datos, char* bufferReceived);

#endif /*SOCKET_H_*/
