/*
 * entrenador.hs
 *
 */

#ifndef ENTRENADOR1_ENTRENADOR_H_
#define ENTRENADOR1_ENTRENADOR_H_

#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "commons/log.h"
#include "commons/config.h"
#include "commons/collections/queue.h"
#include "conexiones.h"

int socketDeMapa;
int socketEntrenador;
int flagUltimoMovimiento=0; //Si esta en 0 se mueve en x,si esta en 1 en y

void recorrerEPrintearLista(t_list* unaLista);


typedef struct {
	int port_Mapa;
	char* ip_Mapa;
} t_conexion;

typedef struct {
char* nombreMapa;
 char** objetivos;
 char* ip;
 int puerto;
} t_mapa;


typedef struct {
 char* simbolo;
 char* nombre;
 char* rutaPokedex;
 unsigned int cantVidas;
 t_list* hojaDeViaje;
 int mapaActual;
 int posicion[2];

} t_entrenador;

//Logger
t_log* logEntrenador;

//Configuracion
t_config configEntrenador;

//Socket y Conexiones
int socketEntrenador = 0;

bool alternateFlag = false;


//Obtiene los datos desde la metada del entrenador
void getMetadataEntrenador(t_entrenador* datosEntrenador);

void avanzarPosicionInts(int* actualX, int* actualY, int toX, int toY);

void avanzarPasosDisponibles(int pasosDisponibles, t_entrenador* unEntrenador, char* posicionPokenest);
int solicitarUbicacionPokenest();
void conectarseAlMapa(t_mapa* unMapa);
void chequearObjetivos(t_entrenador* unEntrenador,char pokemon);
void chequearVidas(t_entrenador* unEntrenador);
void recorrerEPrintearLista(t_list* unaLista);
void atraparUnPokemon(char pokemon,t_entrenador* unEntrenador);


#endif /* ENTRENADOR1_ENTRENADOR_H_ */
