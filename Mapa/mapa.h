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

//Estructura datosCPU
typedef struct {
	char* nombre;
	int numSocket;
	int estadoEntrenador;
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
	char identificador;
	char* tipo;
	t_posicion* posicion;
} t_pokeNest;

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

//Variables de Colas
t_queue* colaListos;
t_queue* colaBloqueados;
t_queue* colaFinalizar;

//Variables Globales
int idProcesos = 1;
int activePID = 0;
int retardo = 0 ;

//flags inicializadas en FALSE
bool alertFlag = false;
bool signalVidas = false;
bool signalMetadata = false;

//Encabezamientos Funciones Principales

void planificarProcesoRR();
void planificarProcesoSRDF();
void liberarEntrenador(int socket);
void procesarEntrenador(char* nombreEntrenador, int socketEntrenador);
void getArchivosDeConfiguracion();


//Encabezamientos Funciones Secundarias


int buscarEntrenadorLibre();
int buscarEntrenador(int socket);
int buscarProceso(char* nombreEntrenador);
void cambiarEstadoProceso(char* nombreEntrenador, int estado);
void inicializarMutex();
void imprimirListaEntrenador();
void sighandler1(int signum);
void sighandler2(int signum);

void ejemploProgramaGui();
void rnd(int *x, int max);

void getMetadataPokeNest(char* pathMetadataPokeNest);
void getMetadataMapa(char* pathMetadataMapa);
void getMetadataPokemon(char* pathPokemon);
void getPosicion(t_config* configuration);

int distanciaEntrePosiciones(t_posicion* posicionEntrenador, t_posicion* posicionItem);
bool estaMasCerca(t_posicion* posicionEntrenador1, t_posicion* posicionEntrenador2, t_posicion* posicionItem);
bool esEntrenador(ITEM_NIVEL* entrenador);
char entrenadorMasCercano(t_list* items, t_posicion* posicionItem);

#endif /* MAPA_H_ */
