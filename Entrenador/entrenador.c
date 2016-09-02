/*
 * entrenador.c
 *
 */

#include "entrenador.h"

int main(int argc, char **argv) {
	t_entrenador* datosEntrenador = malloc(sizeof(t_entrenador));

	datosEntrenador->nombre = NULL;
	datosEntrenador->rutaPokedex = NULL;
	char *logFile = NULL;

	//assert(("ERROR - No se pasaron argumentos", argc > 1)); // Verifica que se haya pasado al menos 1 parametro, sino falla

	//Parametros
	int i;
	for( i = 0; i < argc; i++){
		if (strcmp(argv[i], "") == 0){
			datosEntrenador->nombre = argv[i+1];
			printf("Nombre Entrenador: '%s'\n",datosEntrenador->nombre);
		}
		if (strcmp(argv[i], "") == 0){
			datosEntrenador->rutaPokedex = argv[i+1];
			printf("Ruta Pokedex: '%s'\n",datosEntrenador->rutaPokedex);
		}
		if (strcmp(argv[i], "") == 0){
			logFile = argv[i+1];
			printf("Log File: '%s'\n",logFile);
		}
	}

	//assert(("ERROR - No se paso el nombre del entrenador como argumento", datosEntrenador->nombre != NULL));

	//assert(("ERROR - No se paso el archivo de log como argumento", logFile != NULL));//Verifies if was passed the Log file as parameter, if DONT FAILS

	//Creo el archivo de Log
	//logEntrenador = log_create(logFile, "ENTRENADOR", 0, LOG_LEVEL_TRACE);

	//CONFIGURACION DEL ENTRENADOR

	char* path; //FALTA INICIALIZAR. CUANDO SE TENGA LA RUTA ACTUALIZAR.

	t_config* configEntrenador;

	configEntrenador = config_create(path);

	datosEntrenador->nombre = config_get_string_value(configEntrenador, "nombre");

	datosEntrenador->simbolo = config_get_string_value(configEntrenador, "simbolo");

	datosEntrenador->hojaDeViaje = config_get_array_value(configEntrenador, "hojaDeViaje");

	datosEntrenador->cantVidas = config_get_int_value(configEntrenador, "vidas");

	//faltan los objetivos


	free(datosEntrenador);
	return EXIT_SUCCESS;

}


