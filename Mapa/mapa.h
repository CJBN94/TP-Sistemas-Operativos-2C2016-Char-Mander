/*
 * mapa.h
 *
 */

#ifndef MAPA_H_
#define MAPA_H_

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <commons/log.h>
#include "conexiones.h"

#include "nivel.h"
#include "tad_items.h"

#include <tad_items.h>
#include <stdlib.h>
#include <curses.h>

typedef struct {
	int puerto;
	char* ip;
} t_conexion;

typedef enum{
	NUEVO = 0,
	LISTO,
	EJECUTANDO,
	BLOQUEADO,
	FINALIZANDO
} enum_EstadoProceso;

//Estructura Procesos en cola
typedef struct {
	char* nombre;
	int programCounter;
	enum_EstadoProceso estado;
	int finalizar;
} t_proceso;

typedef struct {
	char id;
	char* tipo;
	int posx;
	int posy;
} t_pokeNest;

//Estructura datosEntrenador
typedef struct {
	char id;
	int numSocket;
	int estadoEntrenador;
	t_pokeNest* objetivoActual;
	int posx;
	int posy;
} t_datosEntrenador;

typedef struct {
	char* nombre;
	char* pathPokedex;
	int tiempoChequeoDeadlock;
	int batalla;
	char* algoritmo;
	int quantum;
	int retardo;
}t_mapa;


typedef struct {
	int nivel;
	char ascii;
} t_pokemon;

//Semaforos
pthread_mutex_t listadoProcesos;
pthread_mutex_t listadoEntrenador;//ver si usar el mutex listadoProcesos
pthread_mutex_t cListos;
pthread_mutex_t cBloqueados;
pthread_mutex_t cFinalizar;
pthread_mutex_t varGlobal;
pthread_mutex_t procesoActivo;

//Configuracion
t_mapa configMapa;
t_conexion conexion;
t_pokeNest configPokenest;
t_pokemon configPokemon;

//Logger
t_log* logMapa;

//Variables de Listas
t_list* listaProcesos;
t_list* listaEntrenador;
t_list* items;

//Variables de Colas
t_queue* colaListos;
t_queue* colaBloqueados;
t_queue* colaFinalizar;

//Variables Globales
int idProcesos = 1;
int activePID = 0;

//flags inicializadas en FALSE
bool alertFlag = false;
bool signalVidas = false;
bool signalMetadata = false;
bool alternateFlag = false;//avanza alternando eje X y eje Y

//Encabezamientos Funciones Principales

void planificarProcesoRR();
void planificarProcesoSRDF();
void liberarEntrenador(int socket);
void procesarEntrenador(char* nombreEntrenador, int socketEntrenador);
void getArchivosDeConfiguracion();
t_datosEntrenador* entrenadorMasCercano();


//Encabezamientos Funciones Secundarias


int buscarEntrenadorLibre();
int buscarEntrenador(int socket);
int buscarProceso(char* nombreEntrenador);
t_pokeNest* buscarObjetivo(char id);
t_datosEntrenador* searchEntrenador(char id);
void cambiarEstadoProceso(char* nombreEntrenador, int estado);
void inicializarMutex();
void crearListas();
void imprimirListaEntrenador();
void sighandler1(int signum);
void sighandler2(int signum);

void ejemploProgramaGui();
void rnd(int *x, int max);

void getMetadataPokeNest(char* pathMetadataPokeNest);
void getMetadataMapa(char* pathMetadataMapa);
void getMetadataPokemon(char* pathPokemon);
void getPosicion(t_config* configuration);

int distanciaAObjetivo(t_datosEntrenador* entrenador);
bool estaMasCerca(t_datosEntrenador* entrenador1, t_datosEntrenador* entrenador2);
bool esEntrenador(ITEM_NIVEL* entrenador);
void avanzarPosicion(int* actualX, int* actualY, int destinoX, int destinoY);
void agregarEntrenador(char id, int x, int y, t_pokeNest* objetivo);
void quitGui();


#endif /* MAPA_H_ */
