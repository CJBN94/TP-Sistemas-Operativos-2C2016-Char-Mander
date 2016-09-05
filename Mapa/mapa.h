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
	int puertoEntrenador;
	char* ipEntrenador;
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
	char* rutaPokedex;
	int tiempoChequeoDeadlock;
	int batalla;
	char* algoritmo;
	int quantum;
	int retardo;
}t_mapa;

typedef struct {
	char* tipo;
	char identificador;
} t_pokeNest;

typedef struct {
	int nivel; //Creo que lleva algo mas
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
t_mapa* configMapa;

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
int QUANTUM = 0;
int retardo = 0 ;

//flags inicializadas en FALSE
bool alertFlag = false;
bool signalVidas = false;
bool signalMetadata = false;

//Encabezamientos Funciones Principales

void planificarProcesoRR();
void liberarEntrenador(int socket);
void procesarEntrenador(char* nombreEntrenador, int socketEntrenador);



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



#endif /* MAPA_H_ */
