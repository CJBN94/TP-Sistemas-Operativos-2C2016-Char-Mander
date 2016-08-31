/*
 * mapa.c
 *
 */

#include "mapa.h"

int main(int argc, char **argv) {
	t_mapa* datosMapa = malloc(sizeof(t_mapa));
	char *logFile = NULL;

	assert(("ERROR - No se pasaron argumentos", argc > 1)); // Verifica que se haya pasado al menos 1 parametro, sino falla

	//Parametros
	int i;
	for( i = 0; i < argc; i++){
		if (strcmp(argv[i], "") == 0){
			datosMapa->nombre = argv[i+1];
			printf("Nombre Mapa: '%s'\n",datosMapa->nombre);
		}
		if (strcmp(argv[i], "") == 0){
			datosMapa->rutaPokedex = argv[i+1];
			printf("Ruta Pokedex: '%s'\n",datosMapa->rutaPokedex);
		}
		if (strcmp(argv[i], "") == 0){
			logFile = argv[i+1];
			printf("Log File: '%s'\n",logFile);
		}
	}

	//assert(("ERROR - No se paso el nombre del entrenador como argumento", datosEntrenador->nombre != NULL));

	//assert(("ERROR - No se paso el archivo de log como argumento", logFile != NULL));//Verifies if was passed the Log file as parameter, if DONT FAILS

	//Creo el archivo de Log
	//logMapa = log_create(logFile, "MAPA", 0, LOG_LEVEL_TRACE);

	return 0;

}
