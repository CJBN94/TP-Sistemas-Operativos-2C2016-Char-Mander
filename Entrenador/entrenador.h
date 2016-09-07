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
 int cantVidas;
 t_list* hojaDeViaje;
 int mapaActual;
} t_entrenador;

//Logger
t_log* logEntrenador;

//Configuracion
t_config configEntrenador;

//Socket y Conexiones
int socketEntrenador = 0;


//Obtiene los datos desde la metada del entrenador
void getMetadataEntrenador(t_entrenador* datosEntrenador,t_list* listaDeMapas);

void avanzarPosicionInts(int* actualX, int* actualY, int* toX, int* toY);

#endif /* ENTRENADOR1_ENTRENADOR_H_ */
