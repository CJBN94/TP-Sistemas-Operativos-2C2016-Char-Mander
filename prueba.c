/*
 * prueba.c
 *
 *  Created on: 5/9/2016
 *      Author: utnso
 */
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "commons/log.h"
#include "commons/config.h"
#include "commons/collections/queue.h"
#include "commons/string.h"

typedef struct {
	char* nombreMapa;
	char** objetivos;
	char* ip;
	int* puerto;
} t_mapa;



typedef struct {
	char* simbolo;
	char* nombre;
	char* rutaPokedex;
	int cantVidas;
	t_list* hojaDeViaje;
} t_entrenador;

void recorrerEPrintearLista(t_list* unaLista);

int main(){

	t_list* listaDeMapas = list_create();
	t_entrenador* datosEntrenador = malloc(sizeof(t_entrenador));
	t_mapa* mapa = malloc(sizeof(t_mapa));


	t_config* configEntrenador = malloc(sizeof(t_config));

	configEntrenador->path = string_from_format("/home/utnso/Pokedex/Entrenadores/%s/metadata","Red");
	configEntrenador = config_create(configEntrenador->path);

	datosEntrenador->nombre = config_get_string_value(configEntrenador, "nombre");
	datosEntrenador->simbolo = config_get_string_value(configEntrenador, "simbolo");
	datosEntrenador->cantVidas = config_get_int_value(configEntrenador, "vidas");
	char** hojaDeViaje = config_get_array_value(configEntrenador,
			"hojaDeViaje");

	printf("El nombre del Entrenador es: %s \n", datosEntrenador->nombre);
	printf("El simbolo que representa al Entrenador es: %s \n",
			datosEntrenador->simbolo);
	printf("La cantidad de vidas del Entrenador es: %d \n", datosEntrenador->cantVidas);

	int i = 0;
	while (hojaDeViaje[i] != NULL) {

		mapa->nombreMapa = hojaDeViaje[i];

		printf("El mapa que debe recorrer el datosEntrenador: %s \n",
				mapa->nombreMapa);

		char* strConcat = string_new();
		string_append(&strConcat, "obj[");
		string_append(&strConcat, mapa->nombreMapa);
		string_append(&strConcat, "]");

		//entrenador->mapa->objetivos=config_get_array_value(configEntrenador,"obj[PuebloPaleta]");

		mapa->objetivos = config_get_array_value(configEntrenador, strConcat);
		int j = 0;
		while (mapa->objetivos[j] != NULL) {

			if (mapa->objetivos[j + 1] != NULL) {
				printf("%s, ", mapa->objetivos[j]);

			} else {
				printf("%s \n", mapa->objetivos[j]);
			}

			j++;

		}

		t_config* configMapa = malloc(sizeof(t_config));

		configMapa->path = string_from_format("/home/utnso/Pokedex/Mapas/%s/metadata",mapa->nombreMapa);
		configMapa = config_create(configMapa->path);
		mapa->ip = config_get_string_value(configMapa, "IP");
		mapa->puerto = config_get_int_value(configMapa,"Puerto");
		printf("La IP del mapa %s es: %s \n", mapa->nombreMapa,mapa->ip);
		printf("El puerto del mapa %s es: %d \n", mapa->nombreMapa,mapa->puerto);

		list_add(listaDeMapas, (void*)mapa);

		i++;
	}

	printf("La cantidad de mapas a recorrer es: %d \n", listaDeMapas->elements_count);

	recorrerEPrintearLista(listaDeMapas);



 return 0;


}

void recorrerEPrintearLista(t_list* unaLista){
 int i=0;
 t_mapa* unMapa=malloc(sizeof(t_mapa));
 while(unaLista->elements_count!=i){

 unMapa=(t_mapa*)list_get(unaLista,i);
 printf("%s \n",unMapa->nombreMapa);
 i++;
 }
}
