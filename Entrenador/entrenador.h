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
bool alternateFlag = false;

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
	char* posicion;
} t_entrenador;

//Logger
t_log* logEntrenador;

//Configuracion
t_entrenador datosEntrenador ;

//Socket y Conexiones
int socketEntrenador = 0;


//Obtiene los datos desde la metada del entrenador
void getMetadataEntrenador();

void avanzarPosicionInt(int* actualX, int* actualY, int destinoX, int destinoY);

void avanzarPasosDisponibles(int pasosDisponibles, t_entrenador* unEntrenador, char* posicionPokenest);
char* solicitarUbicacionPokenest(char pokemon);
void conectarseAlMapa(t_mapa* unMapa);
void chequearObjetivos(t_entrenador* unEntrenador,char pokemon);
void chequearVidas(t_entrenador* unEntrenador);
char* avanzarPosicion(char* posicionInicial,char* posicionDestino);
void recorrerEPrintearLista(t_list* unaLista);

#endif /* ENTRENADOR1_ENTRENADOR_H_ */
