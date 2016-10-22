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
	int operacion;
	int programCounter;
	int quantum;
} t_MensajeMapa_Entrenador;

typedef struct{
	char* nombreEntrenador;
	char id; //simbolo
	int operacion;
	char objetivoActual;
} t_MensajeEntrenador_Mapa;

////OPERACIONES DE FILE SYSTEM////

//LEER ARCHIVO
typedef struct{
	int operacion;
	const char* rutaArchivo;
	int offset;
	int cantidadDeBytes;
	char* buffer;
} t_MensajeLeerPokedexClient_PokedexServer;


//CREAR ARCHIVO
typedef struct{
	int operacion;
	char* rutaDeArchivoACrear;

}t_MensajeCrearArchivoPokedexClient_PokedexServer;


//ESCRIBIR ARCHIVO
typedef struct{
	int operacion;
	char* rutaArchivo;
	char* bufferAEscribir;
	int offset;
	int cantidadDeBytes;
} t_MensajeEscribirArchivoPokedexClient_PokedexServer;

//BORRAR ARCHIVO

typedef struct{
	int operacion;
	char* rutaArchivoABorrar;
} t_MensajeBorrarArchivoPokedexClient_PokedexServer;

//CREAR DIRECTORIO
typedef struct{
	int operacion;
	char* rutaDirectorioPadre;

} t_MensajeCrearDirectorioPokedexClient_PokedexServer;

//BORRAR DIRECTORIO VACIO
typedef struct{
	int operacion;
	char* rutaDirectorioABorrar;
} t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer;




//RENOMBRAR ARCHIVO
typedef struct{
	int operacion;
	char* rutaDeArchivo;
	char* nuevoNombre;
} t_MensajeRenombrarArchivoPokedexClient_PokedexServer;



typedef struct{
	int operacion;
} t_MensajePokedexClient_PokedexServer;

typedef struct{
	int operacion;
} t_MensajePokedexServer_PokedexClient;

//IMPORTANTE --> Nomeclatura de Serializadores y Deserealizadores
//1) serializar<DesdeProceso>_<HastaProceso> ()
//2) deserialiar<HastaProceso>_<DesdeProceso> ()

void serializarEntrenador_Mapa(t_MensajeEntrenador_Mapa* value, char *buffer);
void deserializarMapa_Entrenador(t_MensajeEntrenador_Mapa* value, char *bufferReceived);

void serializarMapa_Entrenador(t_MensajeMapa_Entrenador *value, char *buffer);
void deserializarEntrenador_Mapa(t_MensajeMapa_Entrenador *value, char *bufferReceived);

void serializarPokedexClient_PokedexServer(t_MensajePokedexClient_PokedexServer *value, char *buffer);
void deserializarPokedexServer_PokedexClient(t_MensajePokedexClient_PokedexServer *value, char *bufferReceived);

void serializarPokedexServer_PokedexClient(t_MensajePokedexServer_PokedexClient *value, char *buffer);
void deserializarPokedexCliente_PokedexServer(t_MensajePokedexServer_PokedexClient *value, char * bufferReceived);

void serializarMensajeLeerArchivo(char* buffer,t_MensajeLeerPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeCrearArchivo(char* buffer, t_MensajeCrearArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeEscribirOModificarArchivo(char*buffer, t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeBorrarArchivo(char*buffer, t_MensajeBorrarArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeCrearDirectorio(char*buffer, t_MensajeCrearDirectorioPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeBorrarDirectorio(char*buffer, t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeRenombrarArchivo(char*buffer, t_MensajeRenombrarArchivoPokedexClient_PokedexServer* infoASerializar);

void deserializarMensajeLeerArchivo(char* bufferRecibido,t_MensajeLeerPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeCrearArchivo(char* bufferRecibido, t_MensajeCrearArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeEscribirOModificarArchivo(char*bufferRecibido, t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeBorrarArchivo(char*bufferRecibido, t_MensajeBorrarArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeCrearDirectorio(char*bufferRecibido, t_MensajeCrearDirectorioPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeBorrarDirectorio(char*bufferRecibido, t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeRenombrarArchivo(char*bufferRecibido, t_MensajeRenombrarArchivoPokedexClient_PokedexServer* infoASerializar);

void serializarCadena(char* cadena, char* buffer);
void deserializarCadena(char* cadena, char* bufferRecibido);



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



#endif /*SOCKET_H_*/
