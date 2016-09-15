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
#include <dirent.h>
#include <sys/stat.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/temporal.h>
#include "conexiones.h"
#include <time.h>

#include "nivel.h"
#include "tad_items.h"

#include <tad_items.h>
#include <stdlib.h>
#include <curses.h>

#include "pkmn/battle.h"
#include <pkmn/factory.h>

typedef struct{
	int puerto;
	char* ip;
}t_conexion;

typedef enum{
	NUEVO = 0,
	LISTO,
	EJECUTANDO,
	BLOQUEADO,
	FINALIZANDO
} enum_EstadoProceso;

//Estructura Procesos en cola
typedef struct {
	char id;
	char* nombre;
	int programCounter;
	enum_EstadoProceso estado;
	int finalizar;
} t_procesoEntrenador;

typedef struct {
	char id;
	char* tipo;
	int posx;
	int posy;
} t_pokeNest;

//Estructura datosEntrenador
typedef struct {
	char id;
	char* nombre;
	int numSocket;
	ITEM_NIVEL* objetivoActual;
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
	int entrenadorID;
	char* nombre;
	time_t tiempoBloqueado;
	int index;
	int pokeNestID;
} t_entrenadorBloqueado;

//Semaforos
pthread_mutex_t listadoProcesos;
pthread_mutex_t listadoEntrenador;
pthread_mutex_t cListos;
pthread_mutex_t cBloqueados;
pthread_mutex_t cFinalizar;
pthread_mutex_t varGlobal;
pthread_mutex_t procesoActivo;
pthread_mutex_t listadoPokeNests;


//Configuracion
t_mapa configMapa;
t_conexion conexion;
t_pokemon configPokemon;

//Logger
t_log* logMapa;

//Variables de Listas
t_list* listaProcesos;
t_list* listaEntrenador;
t_list* items;
t_list* pokeNests;

//Variables de Colas
t_queue* colaListos;
t_queue** colasBloqueados;
t_queue* colaFinalizar;

//Variables Globales
int socketMapa;
int idProcesos = 1;
int activePID = 0;
int socketEntrenadorActivo = 0;

//flags inicializadas en FALSE
bool alertFlag = false;
bool signalVidas = false;
bool signalMetadata = false;
bool alternateFlag = false;//avanza alternando eje X y eje Y
int flagPlanificar = -1;

//Conexiones
void startServer();
void newClients (void *parameter);
void handShake (void *parameter);
int clienteNuevo(void *parametro);


//Procesamiento de mensajes
int reconocerOperacion();
void procesarRecibir(int socketEntrenador);
void recibirInfoInicialEntrenador(int socketEntrenador);
void enviarMensajeTurnoConcedido();
void enviarPosPokeNest(t_datosEntrenador* entrenador,int socketEntrenador);
void notificarFinDeObjetivos(char* pathMapa, int socketEntrenador);

//Encabezamientos Funciones Principales

void planificarProcesoRR();
void planificarProcesoSRDF();

void procesarEntrenador(char entrenadorID, char* nombreEntrenador);

void getArchivosDeConfiguracion();
int entrenadorMasCercano();

void ejecutarPrograma();

void actualizarPC(char entrenadorID, int programCounter) ;
void atenderFinDeQuantum(int socketEntrenador,char id);


//Encabezamientos Funciones Secundarias


int buscarEntrenador(int socket);
int buscarSocketEntrenador(char* nombre);
int buscarProceso(char id);
t_datosEntrenador* searchEntrenador(char id);
int buscarPosPokeNest(char id) ;

void cambiarEstadoProceso(char id, int estado);
void inicializarMutex();
void crearListas();
void imprimirListaEntrenador();

void imprimirColaListos();
void imprimirColasBloqueados();


void sighandler1(int signum);
void sighandler2(int signum);

void ejemploProgramaGui();
void rnd(int *x, int max);

t_pokeNest getMetadataPokeNest(char* pathMetadataPokeNest);
void getMetadataMapa(char* pathMetadataMapa);
void getMetadataPokemon(char* pathPokemon);

int distanciaAObjetivo(t_datosEntrenador* entrenador);
bool estaMasCerca(t_datosEntrenador* entrenador1, t_datosEntrenador* entrenador2);
bool esEntrenador(ITEM_NIVEL* entrenador);
void avanzarPosicion(int* actualX, int* actualY, int destinoX, int destinoY);
void agregarEntrenador(char id, char* nombreEntrenador, int socketEntrenador, char objetivoID);
void quitGui();

void procesarDirectorios(char* pathMapa);
int cantidadDePokemones(char* pathPokeNests) ;

void batallar();

ITEM_NIVEL* _search_item_by_id(t_list* items, char id);

t_list* filtrarPokeNests();

void funcionTime();
void *initialize(int tamanio);


#endif /* MAPA_H_ */
