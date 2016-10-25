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

////OPERACIONES DE FILE SYSTEM////

//LEER ARCHIVO
typedef struct{
	const char* rutaArchivo;
	int offset;
	int cantidadDeBytes;
	char* buffer;
} t_MensajeLeerPokedexClient_PokedexServer;


//CREAR ARCHIVO
typedef struct{
	char* rutaDeArchivoACrear;

}t_MensajeCrearArchivoPokedexClient_PokedexServer;


//ESCRIBIR ARCHIVO
typedef struct{
	char* rutaArchivo;
	char* bufferAEscribir;
	int offset;
	int cantidadDeBytes;
}__attribute__((packed))
t_MensajeEscribirArchivoPokedexClient_PokedexServer;

//BORRAR ARCHIVO
typedef struct{
	char* rutaArchivoABorrar;
} t_MensajeBorrarArchivoPokedexClient_PokedexServer;

//CREAR DIRECTORIO
typedef struct{
	char* rutaDirectorioPadre;

} t_MensajeCrearDirectorioPokedexClient_PokedexServer;

//BORRAR DIRECTORIO VACIO
typedef struct{
	char* rutaDirectorioABorrar;
} t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer;

//RENOMBRAR ARCHIVO
typedef struct{
	char* rutaDeArchivo;
	char* nuevoNombre;
} t_MensajeRenombrarArchivoPokedexClient_PokedexServer;


//IMPORTANTE --> Nomeclatura de Serializadores y Deserealizadores
//1) serializar<DesdeProceso>_<HastaProceso> ()
//2) deserialiar<HastaProceso>_<DesdeProceso> ()

void serializarEntrenador_Mapa(t_MensajeEntrenador_Mapa* value, char *buffer);
void deserializarMapa_Entrenador(t_MensajeEntrenador_Mapa* value, char *bufferReceived);

void serializarMensajeLeerArchivo(void* buffer, int tamanioRuta,t_MensajeLeerPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeCrearArchivo(void* buffer, int tamanioRuta, t_MensajeCrearArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeEscribirOModificarArchivo(void* buffer, int tamanioRuta, t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeBorrarArchivo(void* buffer, int tamanioRuta, t_MensajeBorrarArchivoPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeCrearDirectorio(void* buffer, int tamanioRuta, t_MensajeCrearDirectorioPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeBorrarDirectorio(void* buffer, int tamanioRuta, t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* infoASerializar);
void serializarMensajeRenombrarArchivo(void* buffer, int tamanioRuta, t_MensajeRenombrarArchivoPokedexClient_PokedexServer* infoASerializar);

void deserializarMensajeLeerArchivo(void* buffer, int tamanioRuta,t_MensajeLeerPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeCrearArchivo(void* buffer, int tamanioRuta, t_MensajeCrearArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeEscribirOModificarArchivo(void* bufferRecibido,int tamanioRuta, t_MensajeEscribirArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeBorrarArchivo(void* buffer, int tamanioRuta, t_MensajeBorrarArchivoPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeCrearDirectorio(void* buffer, int tamanioRuta, t_MensajeCrearDirectorioPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeBorrarDirectorio(void* buffer, int tamanioRuta, t_MensajeBorrarDirectorioVacioPokedexClient_PokedexServer* infoASerializar);
void deserializarMensajeRenombrarArchivo(void* buffer, int tamanioRuta, t_MensajeRenombrarArchivoPokedexClient_PokedexServer* infoASerializar);

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
