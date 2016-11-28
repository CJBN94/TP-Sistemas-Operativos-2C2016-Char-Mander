/*
 * mapa.h
 *
 */

#ifndef MAPA_H_
#define MAPA_H_

#include <string.h>
#include <stdlib.h>
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
#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/temporal.h>
#include "conexiones.h"
#include <time.h>

#include "nivel.h"
#include "tad_items.h"

#include <stdlib.h>
#include <curses.h>

#include "pkmn/battle.h"
#include <pkmn/factory.h>

#define MAXENTR 20
#define MAXREC 20


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
	enum_EstadoProceso estado;
} t_procesoEntrenador;

typedef struct {
	char id;
	t_pokemon_type type;
	t_pokemon_type second_type;
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
	char entrenadorID;
	char* nombre;
	time_t tiempoBloqueado;
	int index;
	char pokeNestID;
} t_entrenadorBloqueado;

typedef struct {
	int recurso[100];
	int total;
} t_vecRecursos;

//Semaforos
pthread_mutex_t listadoProcesos;
pthread_mutex_t listadoEntrenador;
pthread_mutex_t cListos;
pthread_mutex_t cListosSinDestino;
pthread_mutex_t cFinalizar;
pthread_mutex_t cBloqueados;
pthread_mutex_t listadoBloqueados;
pthread_mutex_t varGlobal;
pthread_mutex_t procesoActivo;
pthread_mutex_t listadoPokeNests;
pthread_mutex_t listadoPokemones;
pthread_mutex_t listadoItems;
pthread_mutex_t listadoEntrMuertos;
pthread_mutex_t mutexRecursosxEntr;

//Configuracion
t_mapa configMapa;
t_conexion conexion;
t_pokeNest configPokenest;
//t_pokemon configPokemon;

//Logger
t_log* logMapa;

//Variables de Listas
t_list* listaProcesos;
t_list* listaEntrenador;
t_list* items;
t_list* pokeNests;
t_list* listaPokemones;
t_list* listaContextoPokemon;
t_list* listaEntrMuertos;

//Variables de Colas
t_queue* colaListos;
t_queue* colaListosSinDestino;
t_queue** colasBloqueados;
t_queue* colaFinalizar;

// Diccionario de recursos asignados clave=idEntrenador data=t_vecRecursos
t_dictionary *recursosxEntr;

//Variables Globales
int socketEntrenadorActivo = 0;
int contEntr = 0;
int rows = 50;//posiciones en y
int cols = 100;//posiciones en x
int QUANTUM = 0;

//flags inicializadas en FALSE
bool signalMetadata = false;
bool flagBatalla = false;

sem_t configOn, mutex, mutexRec, mutexEntr;
sem_t mejorEntrenador, planif, recOp, entrMuerto;

//Conexiones
void startServer();
void newClients (void *parameter);
void handShake (void *parameter);
void clienteNuevo(void *parametro);

char reconocerOperacion();
bool noEstaEnColaDeListos(int* pos, char entrenadorID);


//Procesamiento de mensajes
void procesarRecibir(int socketEntrenador);
void recibirInfoInicialEntrenador(int socketEntrenador);
void enviarMensajeTurnoConcedido();
void enviarPosPokeNest(t_datosEntrenador* entrenador,int socketEntrenador);

//Encabezamientos Funciones Principales
void planificarProcesoRR();
void planificarProcesoSRDF();
void planificarProceso();

int distanciaAObjetivo(t_datosEntrenador* entrenador);
bool estaMasCerca(t_datosEntrenador* entrenador1, t_datosEntrenador* entrenador2);
bool esEntrenador(ITEM_NIVEL* entrenador);

void procesarEntrenador(char entrenadorID, char* nombreEntrenador);
void agregarEntrenador(char id, char* nombreEntrenador, int socketEntrenador, char objetivoID);
void agregarRecursoxEntrenador(t_MensajeEntrenador_Mapa *entrenador, t_vecRecursos *vec);

void getArchivosDeConfiguracion();
t_datosEntrenador* entrenadorMasCercano();

void ejecutarPrograma();

void terminarMapa();


// DEADLOCK

int T[MAXREC]; // vector temporal de disponibles
int matAsignacion[MAXENTR][MAXREC]; // Asignacion [entrenador][recurso]
int matSolicitud[MAXENTR][MAXREC]; // Solicitud [entrenador][recurso]
int vecDisponibles[MAXREC];
int vecEntrenadoresEnMapa[MAXENTR][2];
int vecRecursos[MAXREC];
int vecRecursosCantTotal[MAXREC];
int totalRecursos = 0;
int totalEntrenadores = 0;
char interbloqueados[MAXENTR];

int detectarDeadLock();
void interbloqueo();
t_datosEntrenador* ejecutarBatalla(int cantInterbloqueados);
void resolverSolicitudDeCaptura();
void quitarEntrBloqueado(t_datosEntrenador* entrenador);

void inicializarMatrices();
void imprimirMatrices();
int obtenerPosEntrenador(char e);
int obtenerPosRecurso(char id);
void llenarVecEntrEnMapa();
void llenarRecursos();
void llenarMatAsignacion(t_dictionary *recursosEntrenador);
void llenarMatSolicitud();

void marcarEntrSinRecursosAsig();
void copiarDisponiblesAT();
void marcarNoBloqueados();
int contarEntrSinMarcar();
bool noEsInanicion(int i);

void incrementarRecursoxEntrenador(t_datosEntrenador *entrenador, char idRecurso);
t_vecRecursos* removerRecursoxEntrenador(t_datosEntrenador *entrenador);
void liberarRecursos(t_datosEntrenador* entrenadorMuerto);
void agregarAListaPorMuerte(t_datosEntrenador* entrenadorMuerto);

t_dictionary* crearDiccRecursosxEntr();
t_vecRecursos* crearVecRecursos();
void destruirVecRecursos(t_vecRecursos *vecRecursos);
int quantity(int k);

//Encabezamientos Funciones Secundarias
void dibujar();
int buscarEntrenador(int socket);
int buscarSocketEntrenador(char* nombre);
int buscarProceso(char id);
t_datosEntrenador* searchEntrenador(char id);
ITEM_NIVEL* searchItem(char id);
int buscarPosPokeNest(char id) ;

void cambiarEstadoProceso(char id, enum_EstadoProceso estado);
void inicializarSemaforos();
void crearListas();
void imprimirListaEntrenador();
void filtrarPokeNests();
void imprimirListaPokeNests();
void imprimirListaItems();

void imprimirColaListos();
void imprimirColasBloqueados();
bool restoEntrenadoresBloqueados();

void senial(int sig);
void meterEnListos(t_procesoEntrenador* infoProceso);
void activarPlanificador();
void funcionTime();
void *initialize(int tamanio);

void ejemploProgramaGui();
void rnd(int *x, int max);
void batallaDePrueba();
void quitGui();

//Funciones para metadata
t_pokeNest getMetadataPokeNest(char* pathMetadataPokeNest);
void getMetadataMapa(char* pathMetadataMapa);
int getLevelPokemon(char* pathPokemon);
void getPokemones(char* pathPokeNest, char* nombrePokeNest);
t_pokemon_type reconocerTipo(char* tipo);
bool estaACuatroPosiciones(t_pokeNest* pokeNest);
bool estaEnAreaDeJuego(t_pokeNest* pokeNest);
void procesarDirectorios(char* pathMapa);
int cantidadDePokemones(char* pathPokeNests) ;
void liberarConfig(t_config* configuration);

ITEM_NIVEL* _search_item_by_id(t_list* items, char id);

#endif /* MAPA_H_ */
