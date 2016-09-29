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
#include <signal.h>

int socketEntrenador;
bool alternateFlag = false;
bool esMiTurno = false;

void recorrerEPrintearLista(t_list* unaLista);

typedef struct datosRespuesta{
	int operacion;
	int pid;
}t_respuesta;

void serializarRespuesta(t_respuesta respuesta, char** buffer);

typedef struct {
	char* nombrePokemon;
	int nivel;
}t_pokemon;

typedef struct {
	char* nombreMapa;
	char** objetivos;
	char* ip;
	int puerto;
} t_mapa;

typedef struct {
	char simbolo;
	char* nombre;
	char* rutaPokedex;
	unsigned int cantVidas;
	t_list* hojaDeViaje;
	int mapaActual;
	int posicion[2];
	char objetivoActual;
	t_pokemon pokemonMasFuerte;
} t_entrenador;

//Logger
t_log* logEntrenador;

//Configuracion
t_entrenador entrenador ;
t_mapa mapa;

//Socket y Conexiones
int socketMapa = 0;


//Obtiene los datos desde la metada del entrenador
void getMetadataEntrenador();

void avanzarPosicion(int* actualX, int* actualY, int destinoX, int destinoY);

void avanzarPasosDisponibles(int pasosDisponibles, char* posicionPokenest);
void solicitarUbicacionPokenest(int* posx, int* posy);
void conectarseAlMapa(t_mapa* unMapa);
void chequearObjetivos(char pokemon);
void chequearVidas();
void recorrerEPrintearLista(t_list* unaLista);
void atraparUnPokemon(char pokemon);
int connectTo(enum_procesos processToConnect, int* socketClient);

void procesarRecibir();
void enviarInfoAlMapa();
void verificarTurno();
void interactuarConMapa();
void compararPokemon(t_pokemon unPokemon);
void manejoDeSeniales();
void controladorDeSeniales(int signo);
void quitarVida();
void agregarVida();
void perdiElJuego();
void seniales();
#endif /* ENTRENADOR1_ENTRENADOR_H_ */
