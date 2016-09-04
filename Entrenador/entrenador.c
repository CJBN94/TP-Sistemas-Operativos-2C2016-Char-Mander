/*
 * entrenador.c
 *
 */

#include "entrenador.h"

int main(int argc, char **argv) {
	t_entrenador* datosEntrenador = malloc(sizeof(t_entrenador));

//	datosEntrenador->nombre = NULL;
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



	//Levanto los datos del metadata de Entrenador

	getMetadataEntrenador(datosEntrenador);


	free(datosEntrenador);
	return EXIT_SUCCESS;

}

//Funcion que levanta los datos del entrenador

void getMetadataEntrenador(t_entrenador* datosEntrenador) {

	//t_entrenador* datosEntrenador = malloc(sizeof(t_entrenador));
	t_config* configEntrenador = malloc(sizeof(t_config));
	t_mapa* mapas = malloc(sizeof(t_mapa));
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

