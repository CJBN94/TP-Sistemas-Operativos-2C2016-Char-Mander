/*
 * entrenador.c
 *
 */

#include "entrenador.h"

int main(int argc, char **argv) {


	t_entrenador* datosEntrenador = malloc(sizeof(t_entrenador));



	//datosEntrenador->nombre = NULL;
	//datosEntrenador->rutaPokedex = NULL;
	char *logFile = NULL;

	//assert(("ERROR - No se pasaron argumentos", argc > 1)); // Verifica que se haya pasado al menos 1 parametro, sino falla

	//Parametros
	int i;
	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "") == 0) {
			datosEntrenador->nombre = argv[i + 1];
			printf("Nombre Entrenador: '%s'\n", datosEntrenador->nombre);
		}
		if (strcmp(argv[i], "") == 0) {
			datosEntrenador->rutaPokedex = argv[i + 1];
			printf("Ruta Pokedex: '%s'\n", datosEntrenador->rutaPokedex);
		}
		if (strcmp(argv[i], "") == 0) {
			logFile = argv[i + 1];
			printf("Log File: '%s'\n", logFile);
		}
	}

	//assert(("ERROR - No se paso el nombre del entrenador como argumento", datosEntrenador->nombre != NULL));

	//assert(("ERROR - No se paso el archivo de log como argumento", logFile != NULL));//Verifies if was passed the Log file as parameter, if DONT FAILS

	//Creo el archivo de Log
	//logEntrenador = log_create(logFile, "ENTRENADOR", 0, LOG_LEVEL_TRACE);

	t_mapa* mapas = malloc(sizeof(t_mapa));

	//Levanto los datos del metadata de Entrenador

	getMetadataEntrenador(datosEntrenador, mapas);
	//CONFIGURACION DEL ENTRENADOR


	//faltan los objetivos



	free(datosEntrenador);
	return EXIT_SUCCESS;

}


//Funcion que levanta los datos del entrenador

void getMetadataEntrenador(t_entrenador* datosEntrenador, t_mapa* mapas) {

	//t_entrenador* datosEntrenador = malloc(sizeof(t_entrenador));
	t_config* configEntrenador = malloc(sizeof(t_config));

	configEntrenador->path = "/home/utnso/metadataEntrenador";
	configEntrenador = config_create(configEntrenador->path);

	datosEntrenador->nombre = config_get_string_value(configEntrenador, "nombre");
	datosEntrenador->simbolo = config_get_string_value(configEntrenador, "simbolo");
	datosEntrenador->cantVidas = config_get_int_value(configEntrenador, "vidas");
	char** hojaDeViaje = config_get_array_value(configEntrenador,
			"hojaDeViaje");

	printf("El nombre del datosEntrenador es: %s \n", datosEntrenador->nombre);
	printf("El simbolo que representa al datosEntrenador es: %s \n",
			datosEntrenador->simbolo);
	printf("La cantidad de vidas del datosEntrenador es: %d \n", datosEntrenador->cantVidas);

	int i = 0;
	while (hojaDeViaje[i] != NULL) {
		printf("El mapa que debe recorrer el datosEntrenador: %s \n",
				hojaDeViaje[i]);

		char* strConcat = string_new();
		string_append(&strConcat, "obj[");
		string_append(&strConcat, hojaDeViaje[i]);
		string_append(&strConcat, "]");

		//entrenador->mapa->objetivos=config_get_array_value(configEntrenador,"obj[PuebloPaleta]");

		mapas->objetivos = config_get_array_value(configEntrenador, strConcat);
		int j = 0;
		while (mapas->objetivos[j] != NULL) {

			if (mapas->objetivos[j + 1] != NULL) {
				printf("%s, ", mapas->objetivos[j]);

			} else {
				printf("%s \n", mapas->objetivos[j]);
			}

			j++;

		}

		//char** objetivo = config_get_array_value(configEntrenador,"obj["+ hojaDeViaje[i] +"]");

		i++;
	}
	printf("La cantidad de mapas a recorrer es: %d \n", i);



}

//Cambia la posicion del entrenador segun determine el mapa.

char* avanzarPosicion(char* unaPosicion,char* posicionDestino){
	char* miPosicion=string_new();
	char* posicionQueQuieroLlegar=string_new();
	string_append(&miPosicion,unaPosicion);
	string_append(&posicionQueQuieroLlegar,posicionDestino);
	char** posicionXY;
	char** posicionDestinoXY;
	posicionXY=string_split(miPosicion,";");
	int posicionX=atoi(posicionXY[0]);
	int posicionY=atoi(posicionXY[1]);

	posicionDestinoXY=string_split(posicionQueQuieroLlegar,";");
	int posicionXDestino=atoi(posicionDestinoXY[0]);
	int posicionYDestino=atoi(posicionDestinoXY[1]);
	if(posicionX>posicionXDestino){
		posicionX--;
	}
	if(posicionX<posicionXDestino){
		posicionX++;
	}
	if(posicionY>posicionYDestino){
		posicionY--;
	}
	if(posicionY<posicionYDestino){
		posicionY++;
	}
	char* nuevaPosicion=string_new();
	string_append_with_format(&nuevaPosicion,"%i",posicionX);
	string_append(&nuevaPosicion,";");
	string_append_with_format(&nuevaPosicion,"%i",posicionY);
	strcpy(miPosicion,nuevaPosicion);
	return miPosicion;

}


t_posicion* avanzarPosicionInts(t_posicion* posicionActual,t_posicion* posicionDestino){
	int posicionX = posicionActual->X;
	int posicionY = posicionActual->Y;
	int posicionXDestino = posicionDestino->X;
	int posicionYDestino = posicionDestino->Y;

	if(posicionX>posicionXDestino){
		posicionX--;
	}
	if(posicionX<posicionXDestino){
		posicionX++;
	}
	if(posicionY>posicionYDestino){
		posicionY--;
	}
	if(posicionY<posicionYDestino){
		posicionY++;
	}
	t_posicion* nuevaPosicion;
	nuevaPosicion->X = posicionX;
	nuevaPosicion->Y = posicionY;
	return nuevaPosicion;
}

