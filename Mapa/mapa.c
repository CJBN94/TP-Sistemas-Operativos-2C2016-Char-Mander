/*
 * mapa.c
 *
 */

#include "mapa.h"

int main(int argc, char **argv) {
	t_mapa* datosMapa = malloc(sizeof(t_mapa));
	char *logFile = NULL;
	pthread_t planificadorThread;

	//assert(("ERROR - No se pasaron argumentos", argc > 1)); // Verifica que se haya pasado al menos 1 parametro, sino falla

	//Parametros
	int i;
	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "") == 0) {
			datosMapa->nombre = argv[i + 1];
			printf("Nombre Mapa: '%s'\n", datosMapa->nombre);
		}
		if (strcmp(argv[i], "") == 0) {
			datosMapa->rutaPokedex = argv[i + 1];
			printf("Ruta Pokedex: '%s'\n", datosMapa->rutaPokedex);
		}
		if (strcmp(argv[i], "") == 0) {
			logFile = argv[i + 1];
			printf("Log File: '%s'\n", logFile);
		}
	}

	//assert(("ERROR - No se paso el nombre del entrenador como argumento", datosEntrenador->nombre != NULL));

	//assert(("ERROR - No se paso el archivo de log como argumento", logFile != NULL));//Verifies if was passed the Log file as parameter, if DONT FAILS

	//Creo el archivo de Log
	//logMapa = log_create(logFile, "MAPA", 0, LOG_LEVEL_TRACE);

	//Creo la lista de Entrenadores
	listaEntrenador = list_create();
	//Creo Lista Procesos
	listaProcesos = list_create();
	//Creo la Cola de Listos
	colaListos = queue_create();
	//Creo cola de Procesos Bloqueados.
	colaBloqueados = queue_create();

	//Inicializacion de los mutex
	inicializarMutex();

	pthread_create(&planificadorThread, NULL, (void*) planificarProceso, NULL);


	pthread_join(planificadorThread, NULL);

	return 0;

}

void planificarProceso() {
	//Veo si hay procesos para planificar en la cola de Listos
	if (queue_is_empty(colaListos)) {
		return;
	}
	//Veo si hay ENTRENADOR libre para procesar
	int libreEntrenador = buscarEntrenadorLibre();

	if (libreEntrenador == -1) {
		log_error(logMapa, "No hay ENTRENADOR libre \n");
		return;
	}
	log_info(logMapa, "Se tomo el socket ENTRENADOR '%d' ",libreEntrenador);

	t_datosConexion* datosConexion = malloc(sizeof(t_datosConexion));
	datosConexion->socketClient = libreEntrenador;

	//Saco el primer elemento de la cola, porque ya lo planifique.
	pthread_mutex_lock(&cListos);
	t_proceso* proceso = queue_pop(colaListos);
	pthread_mutex_unlock(&cListos);

	log_info(logMapa, "Se libera de la cola de listos el proceso de PID: '%d'", proceso->PID);
	free(proceso);

	imprimirListaEntrenador();

}

int buscarEntrenadorLibre() {
	int cantEntrenadores, i = 0;
	t_datosEntrenador* datosEntrenador;
	cantEntrenadores = list_size(listaEntrenador);
	log_info(logMapa, "size de listaEntrenador: %d",list_size(listaEntrenador));
	for (i = 0; i < cantEntrenadores; i++) {
		pthread_mutex_lock(&listadoEntrenadores);
		datosEntrenador = (t_datosEntrenador*) list_get(listaEntrenador, i);
		pthread_mutex_unlock(&listadoEntrenadores);

		if (datosEntrenador->estadoEntrenador== 0) {
			datosEntrenador->estadoEntrenador = 1;
			return datosEntrenador->numSocket;
		}
	}
	return -1;
}

void imprimirListaEntrenador() {

	pthread_mutex_lock(&listadoEntrenadores);
	t_list* aux = listaEntrenador;
	pthread_mutex_unlock(&listadoEntrenadores);

	log_info(logMapa, "lista de entrenadores: ");
	int i = 0;
	while (aux != NULL) {
		t_datosEntrenador* datosEntrenador;
		datosEntrenador = (t_datosEntrenador*) list_get(aux, i);

		log_info(logMapa, "Entrenador %d: '%d'\n", i, datosEntrenador->nombre);

		i++;
	}
	if (i == 0) log_info(logMapa, "La lista está vacía");

	if (aux == NULL) log_info(logMapa, "Se llego al final de la lista");
}


void inicializarMutex() {
	pthread_mutex_init(&listadoEntrenadores, NULL);
	pthread_mutex_init(&listadoProcesos, NULL);
	pthread_mutex_init(&cListos, NULL);
	pthread_mutex_init(&cBloqueados, NULL);
	pthread_mutex_init(&varGlobal, NULL);
	pthread_mutex_init(&procesoActivo, NULL);
}

