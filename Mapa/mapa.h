/*
 * mapa.h
 *
 */

#ifndef MAPA_H_
#define MAPA_H_

#include <string.h>
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

typedef struct {
	int puertoEntrenador;
	char* ipEntrenador;
} t_conexion;

//Estructura Procesos en cola
typedef struct {
	int PID;
	int ProgramCounter;
} t_proceso;

//Estructura datosCPU
typedef struct {
	int numSocket;
	int estadoEntrenador;
	char* nombre;
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
	char* posicion;
	char identificador;
} t_pokeNest;

typedef struct {
	int nivel; //Creo que lleva algo mas
} t_pokemon;

//Semaforos

pthread_mutex_t listadoProcesos;
pthread_mutex_t listadoEntrenadores;//ver si usar el mutex listadoProcesos
pthread_mutex_t cListos;
pthread_mutex_t cBloqueados;
pthread_mutex_t varGlobal;
pthread_mutex_t procesoActivo;

//Configuracion
t_config configMapa;

//Logger
t_log* logMapa;

//Variables de Listas
t_list* listaProcesos;
t_list* listaEntrenador;

//Variables de Colas
t_queue* colaListos;
t_queue* colaBloqueados;

//Variables Globales
int idProcesos = 1;
int activePID = 0;

//Encabezamientos Funciones Principales

void planificarProceso();

//Encabezamientos Funciones Secundarias


int buscarEntrenadorLibre();
void inicializarMutex();
void imprimirListaEntrenador();


#endif /* MAPA_H_ */
