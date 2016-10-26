/*
 * mapa.c
 *
 */

#include "mapa.h"

int main(int argc, char **argv) {
	fflush(stdin);
	system("clear");
	signal(SIGUSR2, senial);
	//assert(("ERROR - No se pasaron argumentos", argc > 1)); // Verifica que se haya pasado al menos 1 parametro, sino falla
	//pthread_t finalizarMapaThread;
	pthread_t threadPlanificadorRR;
	pthread_t threadPlanificadorSRDF;
	//pthread_t serverThread;
	pthread_t deadlockThread;

	//Parametros
	int i;
	for (i = 0; i < argc; i++) {
		if (i == 0) {
			configMapa.nombre = argv[i + 1];
			//printf("Nombre Mapa: '%s'\n", configMapa.nombre);
		}
		if (i == 1) {
			configMapa.pathPokedex = argv[i + 1];
			//printf("Path Pokedex: '%s'\n", configMapa.pathPokedex);
		}
	}

	//assert(("ERROR - No se paso el nombre del mapa como argumento", configMapa.nombre != NULL));
	//assert(("ERROR - No se paso el path del Pokedex como argumento", configMapa.pathPokedex != NULL));
	//solo para probar
	configMapa.nombre = "PuebloPaleta";
	configMapa.pathPokedex = "/home/utnso/Pokedex";
	char* logFile = "/home/utnso/git/tp-2016-2c-SegmentationFault/Mapa/logMapa";
	//solo para probar
	//Creo el archivo de Log
	logMapa = log_create(logFile, "MAPA", 0, LOG_LEVEL_TRACE);

	//Inicializacion de listas y mutex
	crearListas();
	inicializarSemaforos();

	//Obtengo informacion de archivos e inicializo mapa
	//pthread_t configThread;
	//pthread_create(&configThread, NULL, (void*) getArchivosDeConfiguracion, NULL);
	getArchivosDeConfiguracion();

	//sem_wait(&configOn);
	//sem_wait(&mutex);

	pthread_create(&deadlockThread, NULL, (void*) interbloqueo, NULL);

	//pthread_create(&finalizarMapaThread, NULL, (void*) quitGui, NULL);

	if (strcmp(configMapa.algoritmo, "RR") == 0) {
		pthread_create(&threadPlanificadorRR, NULL, (void*) planificarProcesoRR, NULL);
	} else if (strcmp(configMapa.algoritmo, "SRDF") == 0) {
		pthread_create(&threadPlanificadorSRDF, NULL, (void*) planificarProcesoSRDF, NULL);
	} else {
		exit(-1);
	}


	//Conexion con el entrenador
	//int socketSv = 0;
	//socketEntrenador = ponerAEscuchar(conexion.ip, conexion.puerto);
	//procesarRecibir(socketEntrenador);

	//pthread_create(&serverThread, NULL, (void*) startServer, NULL);

	//todo para debuguear hay que comentar estas 2 lineas
	nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&rows, &cols);

	dibujar();

	startServer();

	//escucharMultiplesConexiones(&socketEntrenador,conexion.puerto);

	//pthread_create(&serverThread, NULL, (void*) startServerProg, NULL);

	//ejemploProgramaGui();

	void destruirItem(ITEM_NIVEL* item) {
		free(item);
	}
	void destruirEntrenador(t_datosEntrenador* entrenador) {
		free(entrenador->objetivoActual);
		free(entrenador);
	}
	void destruirProcesoEntrenador(t_procesoEntrenador* procesoEntrenador) {
		free(procesoEntrenador);
	}

	list_destroy_and_destroy_elements(pokeNests, (void*) destruirItem);
	list_destroy_and_destroy_elements(items, (void*) destruirItem);
	list_destroy_and_destroy_elements(listaEntrenador, (void*) destruirEntrenador);
	list_destroy_and_destroy_elements(listaProcesos, (void*) destruirProcesoEntrenador);

	nivel_gui_terminar();

	//pthread_join(finalizarMapaThread, NULL);
	if (strcmp(configMapa.algoritmo, "RR") == 0) {
		pthread_join(threadPlanificadorRR, NULL);
	} else if (strcmp(configMapa.algoritmo, "SRDF") == 0) {
		pthread_join(threadPlanificadorSRDF, NULL);
	} else {
		exit(-1);
	}

	//pthread_join(serverThread, NULL);
	//pthread_join(configThread, NULL);
	pthread_join(deadlockThread, NULL);

	//close(socketSv);

	return 1;

}
void dibujar(){
	char* mapa = string_new();
	string_append_with_format(&mapa, "%s%s\0", "Mapa: ", configMapa.nombre);
	nivel_gui_dibujar(items, mapa);
}

void startServer() {
	int socketSv = 0;
	abrirConexionDelServer(conexion.ip, conexion.puerto, &socketSv);
	while (1) {
		clienteNuevo((void*) &socketSv);
	}
}

void clienteNuevo(void *parametro) {
	t_server* datosServer = malloc(sizeof(t_server));
	memcpy(&datosServer->socketServer, parametro, sizeof(datosServer->socketServer));
	pthread_attr_t hiloDeAceptarConexiones;
	pthread_attr_init(&hiloDeAceptarConexiones);
	pthread_attr_setdetachstate(&hiloDeAceptarConexiones, PTHREAD_CREATE_DETACHED);
	pthread_t hiloDeAceptarClientes;
	pthread_create(&hiloDeAceptarClientes, &hiloDeAceptarConexiones, (void*) aceptarConexionDeUnClienteHilo, &datosServer);
	pthread_attr_destroy(&hiloDeAceptarConexiones);
	aceptarConexionDeUnCliente(&datosServer->socketCliente, &datosServer->socketServer);

	if (flagBatalla) {
		sem_wait(&entrMuerto);
		sem_wait(&mutexEntr);
		flagBatalla = false;
	}

	log_info(logMapa, "datos del cliente: %i \n", datosServer->socketCliente);
	int socketEntrenador = datosServer->socketCliente;
	recibirInfoInicialEntrenador(socketEntrenador);

	dibujar();

	if (list_size(listaEntrenador) == 1 )  {
		sem_post(&planif);
		sem_post(&mutex);
	}
	//if(contEntr > 0){
	//sem_wait(&mejorEntrenador);
	//sem_wait(&mutex);
	//}

	pthread_attr_t procesarMensajeThread;
	pthread_attr_init(&procesarMensajeThread);
	pthread_attr_setdetachstate(&procesarMensajeThread, PTHREAD_CREATE_DETACHED);

	pthread_t processMessageThread;
	pthread_create(&processMessageThread, NULL, (void*) ejecutarPrograma, NULL);

	//while(1){
	//ejecutarPrograma();
	//}
	pthread_attr_destroy(&procesarMensajeThread);

	//dibujar();
	//pthread_join(processMessageThread, NULL);
}

void ejecutarPrograma() {
	while (1) {

		sem_wait(&recOp);
		sem_wait(&mutexRec);

		char entrenadorID = 0;
		//sem_wait(&mejorEntrenador);
		//sem_wait(&mutex);

		//if(*socketEntrenador != socketEntrenadorActivo){
		//pthread_mutex_lock(&procesoActivo);
		//int search = buscarEntrenador(socketEntrenadorActivo);
		//t_datosEntrenador* entrenador = (t_datosEntrenador*) list_get(listaEntrenador, search);
		//log_info(logMapa,"El entrenador %s de id: %c bloquea ", entrenador->nombre, entrenador->id);
		//}
		//pthread_mutex_lock(&varGlobal);

		entrenadorID = reconocerOperacion();
		//pthread_mutex_unlock(&varGlobal);
		bool noEsta = noEstaEnColaDeListos(entrenadorID);
		if(noEsta){
			int pos = buscarProceso(entrenadorID);
			if (pos != -1) {
				pthread_mutex_lock(&listadoProcesos);
				t_procesoEntrenador* procesoEntrenador = (t_procesoEntrenador*) list_get(listaProcesos, pos);
				pthread_mutex_unlock(&listadoProcesos);
				if (procesoEntrenador->estado != BLOQUEADO){
					t_procesoEntrenador* unEntrenador = malloc(sizeof(t_procesoEntrenador));
					unEntrenador->nombre = string_new();
					strcpy(unEntrenador->nombre, procesoEntrenador->nombre);
					memcpy(unEntrenador, procesoEntrenador, sizeof(t_procesoEntrenador));

					pthread_mutex_lock(&cListos);
					queue_push(colaListos, (void*) unEntrenador);
					pthread_mutex_unlock(&cListos);
					log_info(logMapa,"agrego %c a colaListos", unEntrenador->id);
				}
			}
		}
		//sem_post(&mejorEntrenador);
		//sem_post(&mutex);
		//imprimirColaListos();
		//log_info(logMapa,"El entrenador id: %c desbloquea ", entrenadorID);
		//pthread_mutex_unlock(&procesoActivo);

		sem_post(&planif);
		sem_post(&mutex);
		//log_info(logMapa, "Planificador activado");

		dibujar();

		/*//si chocan los entrenadores se borra el que no estaba en movimiento(!)
			 t_datosEntrenador* otroEntrenador;
			 int j = 0;
			 int cantEntrenadores = list_size(listaEntrenador);
			 while (j < cantEntrenadores) {
			 pthread_mutex_lock(&listadoEntrenador);
			 otroEntrenador = (t_datosEntrenador*) list_get(listaEntrenador, j);
			 pthread_mutex_unlock(&listadoEntrenador);
			 if ((entrenador->id != otroEntrenador->id)
			 && (entrenador->posx == otroEntrenador->posx)
			 && (entrenador->posy == otroEntrenador->posy)) {
			 BorrarItem(items, otroEntrenador->id);
			 BorrarItem(listaEntrenador, otroEntrenador->id);
			 cantEntrenadores -- ;
			 }
			 j++;
			 }*/

	}
}

char reconocerOperacion() {			//todo reconocerOperacion
	t_MensajeEntrenador_Mapa mensaje;

	//Recibo mensaje usando su tamanio
	char* mensajeRcv = malloc(sizeof(t_MensajeEntrenador_Mapa));
	memset(mensajeRcv, '\0', sizeof(t_MensajeEntrenador_Mapa));
	int bytesRecibidos = recibir(&socketEntrenadorActivo, mensajeRcv, sizeof(t_MensajeEntrenador_Mapa));

	if (bytesRecibidos <= 0) {
		log_error(logMapa,"se recibio un tamanio distinto al esperado");
		return -1;
	}
	deserializarMapa_Entrenador(&mensaje, mensajeRcv);
	free(mensajeRcv);
	//log_info(logMapa,"id: %c", mensaje.id);
	//log_info(logMapa,"operacion: %d", mensaje.operacion);
	//log_info(logMapa,"nombre: %s", mensaje.nombreEntrenador);
	t_datosEntrenador* entrenador = searchEntrenador(mensaje.id);
	entrenador->objetivoActual->id = mensaje.objetivoActual;

	switch (mensaje.operacion) {
	case 1: {
		enviarPosPokeNest(entrenador, socketEntrenadorActivo);

		break;
	}
	case 2: {
		int posx = -1;
		int posy = -1;
		bytesRecibidos = recibir(&socketEntrenadorActivo, &posx, sizeof(int));
		bytesRecibidos = recibir(&socketEntrenadorActivo, &posy, sizeof(int));
		entrenador->posx = posx;
		entrenador->posy = posy;
		pthread_mutex_lock(&listadoItems);
		MoverPersonaje(items, entrenador->id, entrenador->posx, entrenador->posy);
		pthread_mutex_unlock(&listadoItems);

		QUANTUM--;
		usleep(configMapa.retardo * 1000);

		break;
	}
	case 3: {
		/*if (mensaje.objetivoActual != entrenador->objetivoActual->id) break;
		 int objx = entrenador->objetivoActual->posx;
		 int objy = entrenador->objetivoActual->posy;
		 bool estaEnPosObjetivo(t_datosEntrenador* unEntrenador) {
		 return ((unEntrenador->posx == objx) && (unEntrenador->posy == objy));
		 }
		 t_list* listaEntrenadoresEnUnObjetivo = list_filter(listaEntrenador, (void*) estaEnPosObjetivo);
		 bool hayOtroEntrenador = false;
		 if((list_size(listaEntrenadoresEnUnObjetivo) - 1) > 0) hayOtroEntrenador = true;*/
		//if (entrenador->objetivoActual->quantity > 0) {
		//if (hayOtroEntrenador > 1 && configMapa.batalla == 0);
		int pos = buscarPosPokeNest(entrenador->objetivoActual->id);
		t_entrenadorBloqueado* entrenadorBloqueado = malloc(sizeof(t_entrenadorBloqueado));
		entrenadorBloqueado->nombre = string_new();
		memcpy(&entrenadorBloqueado->entrenadorID, &entrenador->id, sizeof(char));
		strcpy(entrenadorBloqueado->nombre, entrenador->nombre);
		memcpy(&entrenadorBloqueado->pokeNestID, &entrenador->objetivoActual->id, sizeof(char));
		entrenadorBloqueado->index = pos;

		imprimirColaListos();
		int i;
		int cantListos = queue_size(colaListos);
		t_procesoEntrenador* procEntr;
		pthread_mutex_lock(&cListos);
		for (i = 0; i < cantListos; i++) {
			procEntr = (t_procesoEntrenador*) list_get(colaListos->elements, i);
			if(procEntr->id == entrenador->id){
				list_remove(colaListos->elements, i);
				log_info(logMapa,"Se libera %s de colaListos",procEntr->nombre);
				//free(procEntr->nombre);
				free(procEntr);
				break;
			}
		}
		pthread_mutex_unlock(&cListos);

		log_info(logMapa, "%c SALE DE LISTOS Y ENTRA EN BLOQUEADOS",entrenador->id);

		cambiarEstadoProceso(entrenador->id, BLOQUEADO);

		time_t comienzo;
		comienzo = time(NULL);
		entrenadorBloqueado->tiempoBloqueado = comienzo;
		pthread_mutex_lock(&cBloqueados);
		queue_push(colasBloqueados[pos], (void*) entrenadorBloqueado);
		pthread_mutex_unlock(&cBloqueados);
		imprimirColaListos();
		imprimirColasBloqueados();

		break;
	}
	case 4: {
		//getMetadataMedalla();
		//enviar pathMedalla
		break;
	}
	case 5: {

		liberarRecursos(entrenador);

		break;
	}
	case 6: {
		//finalizaProceso(socketEntrenadorActivo, mensaje.entrenadorID);
		break;
	}
	case 7: {	//ejemplo de como recibir y enviar texto
		//Recibo el tamanio del texto
		int tamanio;
		bytesRecibidos = recibir(&socketEntrenadorActivo, &tamanio, sizeof(int));
		char* texto = malloc(tamanio);

		//Recibo el texto
		bytesRecibidos = recibir(&socketEntrenadorActivo, texto, tamanio);

		// Envia el tamanio del texto al Entrenador
		log_info(logMapa, "Tamanio: '%d'", tamanio);
		string_append(&texto, "\0");
		enviar(&socketEntrenadorActivo, &tamanio, sizeof(int));

		// Envia el texto al proceso Entrenador
		log_info(logMapa, "Texto : '%s'", texto);
		enviar(&socketEntrenadorActivo, texto, tamanio);

		free(texto);
		break;
	}
	case 10: {
		break;
	}
	default: {
		log_error(logMapa, "Mensaje recibido invalido. ");
		//printf("Entrenador desconectado.");
	}
	}
	char entrenadorID = mensaje.id;

	//free(mensajeRcv);

	return entrenadorID;
}

bool noEstaEnColaDeListos(char entrenadorID){
	int i;
	int cantListos = queue_size(colaListos);
	for (i = 0; i < cantListos; i++){
		pthread_mutex_lock(&cListos);
		t_procesoEntrenador* proceso = (t_procesoEntrenador*) list_get(colaListos->elements, i);
		pthread_mutex_unlock(&cListos);
		if (entrenadorID == proceso->id) {
			return false;
		}
	}
	return true;
}

void funcionTime() {
	time_t comienzo, final;

	comienzo = time( NULL);
	final = time( NULL);

	printf("Comienzo: %d\n", (int) comienzo);
	printf("Final: %d\n", (int) final);
	printf("Número de segundos transcurridos desde el comienzo del programa: %f s\n",
			difftime(final, comienzo));
}

void procesarRecibir(int socketEntrenador) {
	char* mensaje = "holaM";
	enviar(&socketEntrenador, mensaje, 6);
	char* respuesta = malloc(6);
	recibir(&socketEntrenador, respuesta, 6);
	printf("mensaje recibido desde el entrenador: %s\n", respuesta);
	free(respuesta);
}

void procesarEntrenador(char entrenadorID, char* nombreEntrenador) {
	t_procesoEntrenador* procesoEntrenador = malloc(sizeof(t_procesoEntrenador));
	procesoEntrenador->nombre = string_new();
	procesoEntrenador->id = entrenadorID;
	strcpy(procesoEntrenador->nombre, nombreEntrenador);
	procesoEntrenador->estado = LISTO;
	procesoEntrenador->finalizar = 0;

	pthread_mutex_lock(&listadoProcesos);
	list_add(listaProcesos, (void*) procesoEntrenador);
	pthread_mutex_unlock(&listadoProcesos);

	t_procesoEntrenador* unEntrenador = malloc(sizeof(t_procesoEntrenador));
	unEntrenador->nombre = string_new();
	strcpy(unEntrenador->nombre, procesoEntrenador->nombre);
	memcpy(unEntrenador, procesoEntrenador, sizeof(t_procesoEntrenador));

	//Agrego a la Cola de Listos
	pthread_mutex_lock(&cListos);
	queue_push(colaListos, (void*) unEntrenador);
	pthread_mutex_unlock(&cListos);

	log_info(logMapa, "se agrego el entrenador %s a", nombreEntrenador);
	imprimirColaListos();
}

void planificarProcesoSRDF() {
	while (1) {
		//Veo si hay procesos para planificar en la cola de Listos
		if (queue_is_empty(colaListos) && (queue_is_empty(colaFinalizar))) {

		} else {
			sem_wait(&planif);
			sem_wait(&mutex);
			//log_info(logMapa, "Cola de Listos al comenzar la planificacion:");
			//imprimirColaListos();
			//pthread_mutex_lock(&varGlobal);

			t_datosEntrenador* unEntrenador = entrenadorMasCercano();
			//log_info(logMapa, "entr+Cercano es: %c, %s. Socket: %d. ObjID: %c ",
			//		unEntrenador->id, unEntrenador->nombre,
			//		unEntrenador->numSocket, unEntrenador->objetivoActual->id);
			//socketEntrenadorActivo = unEntrenador->numSocket;
			socketEntrenadorActivo = buscarSocketEntrenador(unEntrenador->nombre);

			//enviarMensajeTurnoConcedido();
			bool esTuTurno = true;
			enviar(&socketEntrenadorActivo, &esTuTurno, sizeof(bool));

			//pthread_mutex_unlock(&varGlobal);

			//Saco el elemento de la cola del socketEntrenadorActivo, porque ya lo planifique.
			int i;
			pthread_mutex_lock(&cListos);
			int cantElem = queue_size(colaListos);
			for (i = 0; i < cantElem; i++) {
				t_procesoEntrenador* proceso = (t_procesoEntrenador*) list_get(colaListos->elements, i);
				if (proceso->id == unEntrenador->id) {
					log_info(logMapa,
							"Se libera el proceso ('%c') del Entrenador: '%s' de la cola de listos: ",
							 proceso->id, proceso->nombre);
					list_remove(colaListos->elements, i);
					//free(proceso->nombre);
					//free(proceso);
					break;
				}
			}
			pthread_mutex_unlock(&cListos);
			imprimirColaListos();

			sem_post(&recOp);
			sem_post(&mutexRec);
		}
	}
}

void planificarProcesoRR() { //todo algoritmo RR
	while (1) {
		//Veo si hay procesos para planificar en la cola de Listos
		if (queue_is_empty(colaListos) && (queue_is_empty(colaFinalizar))) {

		} else {
			sem_wait(&planif);
			sem_wait(&mutex);

			log_info(logMapa,"QUANTUM: %d",QUANTUM);
			t_procesoEntrenador* procesoEntrenador;
			if(QUANTUM == 0){
				pthread_mutex_lock(&cListos);
				procesoEntrenador = (t_procesoEntrenador*) queue_pop(colaListos);
				pthread_mutex_unlock(&cListos);
			}else{
				pthread_mutex_lock(&cListos);
				procesoEntrenador = (t_procesoEntrenador*) queue_peek(colaListos);
				pthread_mutex_unlock(&cListos);
				/*if(procesoEntrenador->estado == BLOQUEADO){
					pthread_mutex_lock(&cListos);
					procesoEntrenador = (t_procesoEntrenador*) queue_pop(colaListos);
					pthread_mutex_unlock(&cListos);
				}*/
			}

			socketEntrenadorActivo = buscarSocketEntrenador(procesoEntrenador->nombre);

			if (QUANTUM == 0) {
				QUANTUM = configMapa.quantum;
				if(queue_size(colaListos) > 1 && procesoEntrenador->estado != BLOQUEADO){
					t_procesoEntrenador* unEntrenador = malloc(sizeof(t_procesoEntrenador));
					unEntrenador->nombre = string_new();
					strcpy(unEntrenador->nombre, procesoEntrenador->nombre);
					memcpy(unEntrenador, procesoEntrenador, sizeof(t_procesoEntrenador));

					pthread_mutex_lock(&cListos);
					queue_push(colaListos, (void*) unEntrenador);
					pthread_mutex_unlock(&cListos);
				}else{
					log_info(logMapa,
							"Se libera de la cola de listos el proceso (%c) del Entrenador: '%s'",
							procesoEntrenador->id, procesoEntrenador->nombre);
					//free(procesoEntrenador->nombre);
					free(procesoEntrenador);
				}
				imprimirColaListos();
			}

			if (socketEntrenadorActivo == -1) return;

			bool esTuTurno = true;
			enviar(&socketEntrenadorActivo, &esTuTurno, sizeof(bool));

			sem_post(&recOp);
			sem_post(&mutexRec);
		}
	}
}

void cambiarEstadoProceso(char entrenadorID, enum_EstadoProceso estado) {
	int pos = buscarProceso(entrenadorID);
	if (pos != -1) {
		t_procesoEntrenador* procesoEntrenador;
		pthread_mutex_lock(&listadoProcesos);
		procesoEntrenador = (t_procesoEntrenador*) list_get(listaProcesos,pos);
		pthread_mutex_unlock(&listadoProcesos);
		procesoEntrenador->estado = estado;
	} else {
		log_error(logMapa, "Error al cambiar estado de proceso, proceso no encontrado en la lista.");
	}
}

void imprimirListaEntrenador() {

	pthread_mutex_lock(&listadoEntrenador);
	t_list* aux = listaEntrenador;
	pthread_mutex_unlock(&listadoEntrenador);

	log_info(logMapa, "lista de entrenadores: ");
	int i = 0;
	int cantEntrenadores = list_size(aux);
	while (i < cantEntrenadores) {
		t_datosEntrenador* datosEntrenador;
		datosEntrenador = (t_datosEntrenador*) list_get(aux, i);

		log_info(logMapa, "Entrenador %d: '%c'", i, datosEntrenador->id);

		i++;
	}
	if (i == 0)
		log_warning(logMapa, "La lista está vacía");

	if (aux == NULL)
		log_info(logMapa, "Se llego al final de la lista");
}

void imprimirListaPokeNests() {
	pthread_mutex_lock(&listadoPokeNests);
	t_list* aux = pokeNests;
	pthread_mutex_unlock(&listadoPokeNests);

	log_info(logMapa, "lista de PokeNests: ");
	int i = 0;
	int cantPokeNests = list_size(aux);
	while (i < cantPokeNests) {
		ITEM_NIVEL* datosPokeNest;
		datosPokeNest = (ITEM_NIVEL*) list_get(aux, i);

		log_info(logMapa, "PokeNest %d: '%c'. Disponibles: %d", i, datosPokeNest->id, datosPokeNest->quantity);

		i++;
	}
	if (i == 0)
		log_warning(logMapa, "La lista está vacía");

	if (aux == NULL)
		log_info(logMapa, "Se llego al final de la lista");
}

void imprimirListaItems() {
	pthread_mutex_lock(&listadoItems);
	t_list* aux = items;
	pthread_mutex_unlock(&listadoItems);

	log_info(logMapa, "lista de Items: ");
	int i = 0;
	int cantPokeNests = list_size(aux);
	while (i < cantPokeNests) {
		ITEM_NIVEL* datosPokeNest;
		datosPokeNest = (ITEM_NIVEL*) list_get(aux, i);

		log_info(logMapa, "PokeNest %d: '%c'. Disponibles: %d", i, datosPokeNest->id, datosPokeNest->quantity);

		i++;
	}
	if (i == 0)
		log_warning(logMapa, "La lista está vacía");

	if (aux == NULL)
		log_info(logMapa, "Se llego al final de la lista");
}

void imprimirColaListos() {
	pthread_mutex_lock(&cListos);
	t_queue* aux = colaListos;
	pthread_mutex_unlock(&cListos);

	int i = 0;
	int cantEntrenadores = queue_size(aux);
	if (cantEntrenadores == 0) {
		log_info(logMapa, "No hay Entrenadores en Cola de Listos");
		return;
	}
	log_info(logMapa, "Cola de Listos: ");
	while (i < cantEntrenadores) {
		t_procesoEntrenador* datosEntrenador;
		datosEntrenador = (t_procesoEntrenador*) list_get(aux->elements, i);

		log_info(logMapa, "Entrenador %d: '%s'. ID: %c", i, datosEntrenador->nombre, datosEntrenador->id);
		i++;
	}

	if (aux == NULL)
		log_info(logMapa, "-----------------");
}

void imprimirColasBloqueados() {
	pthread_mutex_lock(&cBloqueados);
	t_queue** aux = colasBloqueados;
	pthread_mutex_unlock(&cBloqueados);

	log_info(logMapa, "Cola de Bloqueados: ");
	int i = 0;
	int cantPokeNests = list_size(pokeNests);
	while (i < cantPokeNests) {
		if (!queue_is_empty(aux[i])) {
			int j = 0;
			while(j < aux[i]->elements->elements_count){
				t_entrenadorBloqueado* entrenador= (t_entrenadorBloqueado*) list_get(aux[i]->elements, j);
				log_info(logMapa, "Entrenador pos '%d': '%s'. ID: '%c'", i, entrenador->nombre, entrenador->entrenadorID);
				j++;
			}
		}
		i++;
	}
	if (i == 0)
		log_info(logMapa, "No hay Entrenadores en Cola de Bloqueados");

	if (aux == NULL)
		log_info(logMapa, "-----------------");
}

void crearListas() {
	//Creo la lista de Entrenadores
	listaEntrenador = list_create();
	//Creo la lista de items en interfaz
	items = list_create();
	//Creo Lista Procesos
	listaProcesos = list_create();
	//Creo Lista de Pokemones
	listaPokemones = list_create();
	//Creo la Cola de Listos
	colaListos = queue_create();
	//Creo cola de Procesos a Finalizar.
	colaFinalizar = queue_create();
	//Creo diccionario recursos por entrenador
	recursosxEntr = dictionary_create();
	//Creo Lista de Entrenadores muertos en batalla
	listaEntrMuertosxBatalla = list_create();
}

void *initialize(int tamanio) {
	int i;
	void * retorno = malloc(tamanio);
	for (i = 0; i < tamanio; i++) {
		((char*) retorno)[i] = 0;
	}
	return retorno;
}

void inicializarSemaforos() {
	pthread_mutex_init(&listadoEntrenador, NULL);
	pthread_mutex_init(&listadoProcesos, NULL);
	pthread_mutex_init(&cListos, NULL);
	pthread_mutex_init(&cListosSinDestino, NULL);
	pthread_mutex_init(&cBloqueados, NULL);
	pthread_mutex_init(&cFinalizar, NULL);
	pthread_mutex_init(&varGlobal, NULL);
	pthread_mutex_init(&procesoActivo, NULL);
	pthread_mutex_init(&listadoPokeNests, NULL);
	pthread_mutex_init(&listadoPokemones, NULL);
	pthread_mutex_init(&listadoItems, NULL);
	pthread_mutex_init(&listadoEntrMuertosxBatalla,NULL);
	pthread_mutex_init(&listadoBloqueados,NULL);
	pthread_mutex_init(&mutexRecursosxEntr,NULL);

	sem_init(&mutex, 0, 1);
	sem_init(&mutexEntr, 0, 1);
	sem_init(&mutexRec, 0, 1);
	sem_init(&planif, 0, 0);
	sem_init(&mejorEntrenador, 0, 0);
	sem_init(&recOp, 0, 0);
	sem_init(&entrMuerto, 0, 0);
}

void senial(int sig) {
	log_info(logMapa,
			"Signal capturada %d para leer un archivo de metadata del Pokedex ", sig);
	//Flag activada para leer archivo de metadata
	if (sig == SIGUSR2) getArchivosDeConfiguracion();
}

int buscarEntrenador(int socket) {
	t_datosEntrenador* datosEntrenador;
	int i = 0;
	int cantEntrenadores = list_size(listaEntrenador);
	for (i = 0; i < cantEntrenadores; i++) {
		pthread_mutex_lock(&listadoEntrenador);
		datosEntrenador = (t_datosEntrenador*) list_get(listaEntrenador, i);
		pthread_mutex_unlock(&listadoEntrenador);
		if (datosEntrenador->numSocket == socket) {
			return i;
		}
	}
	return -1;
}

int buscarProceso(char id) {
	t_procesoEntrenador* procesoEntrenador;
	int i = 0;
	int cantProcesos = list_size(listaProcesos);
	for (i = 0; i < cantProcesos; i++) {
		pthread_mutex_lock(&listadoProcesos);
		procesoEntrenador = (t_procesoEntrenador*) list_get(listaProcesos, i);
		pthread_mutex_unlock(&listadoProcesos);
		if (procesoEntrenador->id == id) {
			return i;
		}
	}
	return -1;
}

int buscarPosPokeNest(char id) {
	ITEM_NIVEL* pokeNest;
	int i = 0;
	int cantPokeNests = list_size(pokeNests);
	for (i = 0; i < cantPokeNests; i++) {
		pthread_mutex_lock(&listadoPokeNests);
		pokeNest = (ITEM_NIVEL*) list_get(pokeNests, i);
		pthread_mutex_unlock(&listadoPokeNests);
		if (pokeNest->id == id) {
			return i;
		}
	}
	return -1;
}

void getArchivosDeConfiguracion() {
	log_info(logMapa, "Obteniendo configuracion del mapa: %s ", configMapa.nombre);

	char* pathMapa = string_new();
	string_append(&pathMapa, configMapa.pathPokedex);
	string_append(&pathMapa, "/Mapas/");
	string_append(&pathMapa, configMapa.nombre);

	//  Path:   /Mapas/[nombre]/metadata
	char* pathMetadataMapa = string_new();
	pathMetadataMapa = string_from_format("%s/metadata\0", pathMapa);
	getMetadataMapa(pathMetadataMapa);
	free(pathMetadataMapa);

	//  Path:	/Mapas/[nombre]/PokeNests/[nombre-de-PokeNest]/metadata
	procesarDirectorios(pathMapa);
	imprimirListaItems();

	//Crear Lista PokeNests
	pthread_mutex_lock(&listadoPokeNests);
	pokeNests = filtrarPokeNests();
	int cantPokeNests = list_size(pokeNests);
	totalRecursos = cantPokeNests;
	pthread_mutex_unlock(&listadoPokeNests);

	imprimirListaPokeNests();

	//Creo colas de Procesos Bloqueados
	colasBloqueados = initialize(cantPokeNests * sizeof(char*));
	int i = 0;
	while (i < cantPokeNests) {
		colasBloqueados[i] = initialize(sizeof(t_queue*));
		colasBloqueados[i] = queue_create();
		i++;
	}

	//sem_post(&configOn);
	//sem_post(&mutex);
}

void getMetadataMapa(char* pathMetadataMapa) {
	//log_info(logMapa, "Metadata del mapa: %s ", pathMetadataMapa);
	t_config* configuration;

	configuration = config_create(pathMetadataMapa);

	configMapa.tiempoChequeoDeadlock = config_get_int_value(configuration, "TiempoChequeoDeadlock");
	configMapa.batalla = config_get_int_value(configuration, "Batalla");
	configMapa.algoritmo = config_get_string_value(configuration, "algoritmo");
	configMapa.quantum = config_get_int_value(configuration, "quantum");
	memcpy(&QUANTUM, &configMapa.quantum, sizeof(int));
	configMapa.retardo = config_get_int_value(configuration, "retardo");

	conexion.ip = config_get_string_value(configuration, "IP");
	conexion.puerto = config_get_int_value(configuration, "Puerto");

}

t_pokeNest getMetadataPokeNest(char *pathMetadataPokeNest) {
	//log_info(logMapa, "metadata de la Pokenest: %s ", pathMetadataPokeNest);
	t_config* configuration;
	configuration = config_create(pathMetadataPokeNest);

	char* tipo = string_new();
	tipo = config_get_string_value(configuration, "Tipo");
	configPokenest.type = reconocerTipo(tipo);
	configPokenest.second_type = 0;//todo agregar en caso de existir segundo tipo
	char* unaPos = config_get_string_value(configuration, "Posicion");
	char** posicionXY;
	posicionXY = string_split(unaPos, ";");
	configPokenest.posx = atoi(posicionXY[0]);
	configPokenest.posy = atoi(posicionXY[1]);

	char* identificador = config_get_string_value(configuration,"Identificador");
	memcpy(&configPokenest.id, identificador, sizeof(char));
	log_info(logMapa,
			"POKENEST - type: '%d', posx: '%d', posy: '%d', id: '%c' ",
			configPokenest.type, configPokenest.posx, configPokenest.posy,configPokenest.id);
	return configPokenest;

}

t_pokemon_type reconocerTipo(char* tipo){
	if (strcmp(tipo, "Normal") == 0) return NORMAL;
	if (strcmp(tipo, "Fuego") == 0) return FIRE;
	if (strcmp(tipo, "Agua") == 0) return WATER;
	if (strcmp(tipo, "Electrico") == 0) return ELECTRIC;
	if (strcmp(tipo, "Planta") == 0) return GRASS;
	if (strcmp(tipo, "Hielo") == 0) return ICE;
	if (strcmp(tipo, "Lucha") == 0) return FIGHT;
	if (strcmp(tipo, "Veneno") == 0) return POISON;
	if (strcmp(tipo, "Tierra") == 0) return GROUND;
	if (strcmp(tipo, "Aire") == 0) return FLYING;
	if (strcmp(tipo, "Psiquico") == 0) return PSYCHC;
	if (strcmp(tipo, "Error") == 0) return BUG;
	if (strcmp(tipo, "Roca") == 0) return ROCK;
	if (strcmp(tipo, "Fantasma") == 0) return GHOST;
	if (strcmp(tipo, "Dragon") == 0) return DRAGON;
	if (strcmp(tipo, "Oscuro") == 0) return DARK;
	if (strcmp(tipo, "Acero") == 0) return STEEL;
	if (strcmp(tipo, "Magia") == 0) return FAIRY;
	return 0;
}

int getMetadataPokemon(char* pathPokemon) {
	log_info(logMapa, "metadata del pokemon: %s ", pathPokemon);
	t_config* configuration;

	configuration = config_create(pathPokemon);

	return config_get_int_value(configuration, "Nivel");

	/*char* ascii = string_new();
	 ascii = config_get_string_value(configuration,""); //
	 string_split(ascii,"[");
	 string_split(ascii,"]");
	 memcpy(ascii, &configPokemon.ascii, strlen(ascii));*/
	//log_info(logMapa, "POKEMON - nivel: %s, ascii: %c ", configPokemon.nivel, configPokemon.ascii);
}

/*
 * @NAME: rnd
 * @DESC: Modifica el numero en +1,0,-1, sin pasarse del maximo dado
 */

/*
void rnd(int *x, int max) {
	*x += (rand() % 3) - 1;
	*x = (*x < max) ? *x : max - 1;
	*x = (*x > 0) ? *x : 1;
}

 void ejemploProgramaGui() {
 pthread_t finalizarMapaThread;
 pthread_create(&finalizarMapaThread, NULL, (void*) quitGui, NULL);

 int rows = 19, cols = 78;
 int q, p;

 int x = 45, y = 2;

 int a = 35, b = 5;

 int ex1 = 10, ey1 = 14;
 int ex2 = 20, ey2 = 3;

 int cx1 = 60, cy1 = 17;
 //int cx2 = 8, cy2 = 15;
 int cx3 = 50, cy3 = 2;

 nivel_gui_inicializar();
 nivel_gui_get_area_nivel(&rows, &cols);
 //Dimensiones normales = 78 x 19
 log_info(logMapa,"En X(cantCol):'%d'. En Y(cantFilas):'%d'",cols,rows);

 p = cols;
 q = rows;

 CrearCaja(items, 'H', cx1, cy1, 5);
 //CrearCaja(items, 'M', cx2, cy2, 3);
 CrearCaja(items, 'F', cx3, cy3, 2);

 char idObjetivo = 'H';
 ITEM_NIVEL* item = _search_item_by_id(items, idObjetivo);
 //agregarEntrenador('@', p, q, item);
 CrearPersonaje(items, '@', p, q);

 char idObjetivo2= 'B';//Esto lo debe recibir del entrenador
 ITEM_NIVEL* item2 = _search_item_by_id(items, idObjetivo2);
 //agregarEntrenador('#', x, y, item2);
 CrearPersonaje(items, '#', x, y);

 char idObjetivo3 = 'F';
 ITEM_NIVEL* item3 = _search_item_by_id(items, idObjetivo3);
 //agregarEntrenador('=', a, b, item3);
 CrearPersonaje(items, '=', a, b);
 //CrearPersonaje(items, '!', 20, 13);
 //CrearPersonaje(items, '?', 28, 1);

 CrearEnemigo(items, '1', ex1, ey1);
 CrearEnemigo(items, '2', ex2, ey2);

 nivel_gui_dibujar(items, "Test Mapa SegmentationFault");

 t_datosEntrenador* entrenador = entrenadorMasCercano();

 //ITEM_NIVEL* pokeNest = _search_item_by_id(items,entrenador->objetivoActual->id);

 while (1){
 int objx = entrenador->objetivoActual->posx;
 int objy = entrenador->objetivoActual->posy;
 bool estaEnPosObjetivo(t_datosEntrenador* unEntrenador) {
 return ((unEntrenador->posx == objx) && (unEntrenador->posy == objy));
 }
 bool hayOtroEntrenador = list_any_satisfy(listaEntrenador, (void*) estaEnPosObjetivo);

 avanzarPosicion(&entrenador->posx, &entrenador->posy, objx, objy);
 usleep(configMapa.retardo*1000);

 rnd(&ex1, cols);
 rnd(&ey1, rows);
 rnd(&ex2, cols);
 rnd(&ey2, rows);
 MoverPersonaje(items, '1', ex1, ey1);
 MoverPersonaje(items, '2', ex2, ey2);

 MoverPersonaje(items, entrenador->id, entrenador->posx, entrenador->posy);
 //enviar el informe del uso de una unidad de tiempo

 if (estaEnPosObjetivo(entrenador) && entrenador->objetivoActual->quantity > 0) {//esto ya lo valida el entrenador

 if (hayOtroEntrenador && configMapa.batalla == 0) {
 int pos = buscarPosPokeNest(entrenador->objetivoActual->id);
 pthread_mutex_lock(&cBloqueados);
 queue_push(colasBloqueados[pos], (void*) entrenador);
 pthread_mutex_unlock(&cBloqueados);
 //contar tiempo bloqueado

 } else if (!hayOtroEntrenador) {
 restarRecurso(items, entrenador->objetivoActual->id);
 //informar al entrenador de objetivo cumplido y recibir otro nuevo objetivo
 //actualizo objetivo
 entrenador->objetivoActual = _search_item_by_id(items, 'H');
 objx = entrenador->objetivoActual->posx;
 objy = entrenador->objetivoActual->posy;
 if(entrenador->id =='@'){//solo para probar
 entrenador->objetivoActual = _search_item_by_id(items, 'P');
 objx = entrenador->objetivoActual->posx;
 objy = entrenador->objetivoActual->posy;
 }
 }else{
 batallaDePrueba();
 return;//solo para probar
 }
 }
 //si chocan los entrenadores se borra el que no estaba en movimiento(!)
 t_datosEntrenador* otroEntrenador;
 int j = 0;
 int cantEntrenadores = list_size(listaEntrenador);
 while (j < cantEntrenadores) {
 pthread_mutex_lock(&listadoEntrenador);
 otroEntrenador = (t_datosEntrenador*) list_get(listaEntrenador, j);
 pthread_mutex_unlock(&listadoEntrenador);
 if ((entrenador->id != otroEntrenador->id)
 && (entrenador->posx == otroEntrenador->posx)
 && (entrenador->posy == otroEntrenador->posy)) {
 BorrarItem(items, otroEntrenador->id);
 BorrarItem(listaEntrenador, otroEntrenador->id);
 cantEntrenadores -- ;
 }
 j++;
 }

 t_datosEntrenador* nuevoEntrenador = entrenadorMasCercano();
 if(entrenador->id != nuevoEntrenador->id){
 entrenador = nuevoEntrenador;
 log_info(logMapa,"el nuevo entrenador mas cercano es:'%c'", entrenador->id);//
 }
 nivel_gui_dibujar(items, "Test Mapa SegmentationFault");

 }
 void destruirItem(ITEM_NIVEL* item) {
 free(item);
 }
 list_destroy_and_destroy_elements(items, (void*) destruirItem);
 nivel_gui_terminar();

 pthread_join(finalizarMapaThread, NULL);

 }

 */
void batallaDePrueba() {
	//Se crea una instancia de la Factory para crear pokemons con solo el nombre y el nivel
	t_pkmn_factory* pokemon_factory = create_pkmn_factory();

	//Factory, Nombre empieza con letra mayúscula (sin errores) y Nivel
	t_pokemon * pikachu = create_pokemon(pokemon_factory, "Pikachu", 30);
	t_pokemon * rhyhorn = create_pokemon(pokemon_factory, "Rhyhorn", 6);

	//Si el nombre del Pokémon no existe en los primeros 151 o está mal escrito
	//Retornará el puntero NULL (0x0)
	char* nombrePokemon = "MissingNo";
	t_pokemon * missigno = create_pokemon(pokemon_factory, nombrePokemon, 128);
	if (missigno == NULL)
		printf("El Pokémon %s no existe! El puntero de retorno de la factory fue: %p\n",
				nombrePokemon, missigno);

	//solo para visualizar el resultado -------------
	void destruirItem(ITEM_NIVEL* item) {
		free(item);
	}
	list_destroy_and_destroy_elements(items, (void*) destruirItem);
	nivel_gui_terminar();
	//solo para visualizar el resultado --------------

	printf("========Batalla!========\n");
	printf("Primer Pokemon: %s[%s/%s] Nivel: %d\n", pikachu->species,
			pkmn_type_to_string(pikachu->type),
			pkmn_type_to_string(pikachu->second_type), pikachu->level);
	printf("Segundo Pokemon: %s[%s/%s] Nivel: %d\n", rhyhorn->species,
			pkmn_type_to_string(rhyhorn->type),
			pkmn_type_to_string(rhyhorn->second_type), rhyhorn->level); //Función que sirve para ver el Tipo de Enum como un String

	//La batalla propiamente dicha
	t_pokemon * loser = pkmn_battle(pikachu, rhyhorn);

	printf("El Perdedor es: %s\n", loser->species); //species es el nombre del pokemon

	//Liberemos los recursos
	//Como el puntero loser apunta a alguno de los otros 2, no se lo libera
	free(pikachu);
	free(rhyhorn);
	//Hay que destruir la instancia de la Factory
	destroy_pkmn_factory(pokemon_factory);
}

t_datosEntrenador* searchEntrenador(char id) {
	bool search_by_id(t_datosEntrenador* entrenador) {
		return entrenador->id == id;
	}

	return list_find(listaEntrenador, (void*) search_by_id);
}
ITEM_NIVEL* searchItem(char id) {

	bool search_by_id(ITEM_NIVEL* item) {
		return item->id == id;
	}
	return list_find(items, (void*) search_by_id);

}

void agregarEntrenador(char id, char* nombreEntrenador, int socketEntrenador, char objetivoID) {
	//imprimirListaPokeNests();
	ITEM_NIVEL* item = _search_item_by_id(items, objetivoID);

	t_datosEntrenador* datosEntrenador = malloc(sizeof(t_datosEntrenador));
	datosEntrenador->objetivoActual = malloc(sizeof(ITEM_NIVEL));
	datosEntrenador->id = id;
	datosEntrenador->nombre = string_new();
	strcpy(datosEntrenador->nombre, nombreEntrenador);
	datosEntrenador->numSocket = socketEntrenador;
	memcpy(datosEntrenador->objetivoActual, item, sizeof(ITEM_NIVEL));

	datosEntrenador->posx = 1;
	datosEntrenador->posy = 1;

	pthread_mutex_lock(&listadoEntrenador);
	list_add(listaEntrenador, (void*) datosEntrenador);
	pthread_mutex_unlock(&listadoEntrenador);

}

void quitGui() {
	while (1) {

		int key;
		key = getch();

		if (key == 'Q' || key == 'q') {

			void destruirItem(ITEM_NIVEL* item) {
				free(item);
			}
			list_destroy_and_destroy_elements(items, (void*) destruirItem);

			nivel_gui_terminar();
			exit(0);
		}
	}
}

int distanciaAObjetivo(t_datosEntrenador* entrenador) {
	ITEM_NIVEL* objetivo;

	objetivo = _search_item_by_id(items, entrenador->objetivoActual->id);

	int distanciaX = entrenador->posx - objetivo->posx;
	int distanciaY = entrenador->posy - objetivo->posy;
	int distanciaTotal = abs(distanciaX) + abs(distanciaY);

	return distanciaTotal;
}

bool estaMasCerca(t_datosEntrenador* entrenador1, t_datosEntrenador* entrenador2) {
	int distanciaEntrenador1 = distanciaAObjetivo(entrenador1);
	int distanciaEntrenador2 = distanciaAObjetivo(entrenador2);
	//log_info(logMapa, "dist %d de %c menor a dist %d de %c",
	//		distanciaEntrenador1, entrenador1->id, distanciaEntrenador2,entrenador2->id);

	if (distanciaEntrenador1 < distanciaEntrenador2) {
		return true;
	}
	return false;
}
/*
 bool esEntrenador(ITEM_NIVEL* entrenador){
 return entrenador->item_type == PERSONAJE_ITEM_TYPE;
 }
 */

t_list* filtrarPokeNests() {
	bool esPokeNest(ITEM_NIVEL* pokeNest) {
		return pokeNest->item_type == RECURSO_ITEM_TYPE;
	}
	pthread_mutex_lock(&listadoItems);
	t_list* pokeNests = list_filter(items, (void*) esPokeNest);
	pthread_mutex_unlock(&listadoItems);

	return pokeNests;

}

t_datosEntrenador* entrenadorMasCercano() {
	pthread_mutex_lock(&cListos);
	t_procesoEntrenador* procesoEntrenadorCercano = (t_procesoEntrenador*) queue_peek(colaListos);
	pthread_mutex_unlock(&cListos);
	int i = 0;
	int cantEntrenadores = queue_size(colaListos);

	t_datosEntrenador* entr = searchEntrenador(procesoEntrenadorCercano->id);
	//log_info(logMapa, "size colaListos %d dentro de la func entr+Cerc",cantEntrenadores);
	while (i < cantEntrenadores) {
		i++;
		if (i == cantEntrenadores) return entr;
		pthread_mutex_lock(&listadoEntrenador);
		t_procesoEntrenador* procesoOtroEntrenador = (t_procesoEntrenador*) list_get(colaListos->elements, i);
		pthread_mutex_unlock(&listadoEntrenador);

		t_datosEntrenador* entrenadorMasCercano = searchEntrenador(procesoEntrenadorCercano->id);
		t_datosEntrenador* otroEntrenador = searchEntrenador(procesoOtroEntrenador->id);
		bool flag = estaMasCerca(entrenadorMasCercano, otroEntrenador);

		if (flag == false) {
			procesoEntrenadorCercano = procesoOtroEntrenador;
			entr = searchEntrenador(procesoEntrenadorCercano->id);
		}
	}
	return entr;
}

void enviarPosPokeNest(t_datosEntrenador* entrenador, int socketEntrenador) {
	ITEM_NIVEL* item = _search_item_by_id(items,entrenador->objetivoActual->id);
	memcpy(entrenador->objetivoActual, item, sizeof(ITEM_NIVEL));

	if (item != NULL) {
		int x = entrenador->objetivoActual->posx;
		int y = entrenador->objetivoActual->posy;
		enviar(&socketEntrenador, &x, sizeof(int));
		enviar(&socketEntrenador, &y, sizeof(int));
	}
}

void enviarMensajeTurnoConcedido() {
	char turnoConcedido[] = "turno concedido";
	int turnoLen = -1;
	turnoLen = strlen(turnoConcedido) + 1; // +1(solo en arrays) es porque strlen no cuenta el \0

	// Envia el tamanio del texto Entrenador
	enviar(&socketEntrenadorActivo, &turnoLen, sizeof(turnoLen));
	enviar(&socketEntrenadorActivo, turnoConcedido, turnoLen);

}

void notificarFinDeObjetivos(char* pathMapa, int socketEntrenador) {

	//  Path:   /Mapas/[nombre]/medalla-[nombre].jpg
	char* pathMetadataMedalla = string_new();
	pathMetadataMedalla = string_from_format("%s/medalla-%s.jpg\0", pathMapa, configMapa.nombre);
	int pathLen = -1;
	pathLen = strlen(pathMetadataMedalla) + 1;
	enviar(&socketEntrenador, pathMetadataMedalla, pathLen);
	//al Entrenador el pathMetadataMedalla cuando pida por fin de objetivos
	free(pathMetadataMedalla);

}

int buscarSocketEntrenador(char* nombre) {
	t_datosEntrenador* datosEntrenador;
	int i = 0;
	int cantEntrenadores = list_size(listaEntrenador);
	for (i = 0; i < cantEntrenadores; i++) {
		pthread_mutex_lock(&listadoEntrenador);
		datosEntrenador = list_get(listaEntrenador, i);
		pthread_mutex_unlock(&listadoEntrenador);
		if (strcmp(datosEntrenador->nombre, nombre) == 0) {
			return datosEntrenador->numSocket;
		}
	}
	return -1;
}

void recibirInfoInicialEntrenador(int socketEntrenador) {
	t_MensajeEntrenador_Mapa mensaje;

	//Recibo mensaje usando su tamanio
	char* mensajeRcv = malloc(sizeof(t_MensajeEntrenador_Mapa));
	memset(mensajeRcv, '\0', sizeof(t_MensajeEntrenador_Mapa));
	recibir(&socketEntrenador, mensajeRcv, sizeof(t_MensajeEntrenador_Mapa));

	deserializarMapa_Entrenador(&mensaje, mensajeRcv);
	free(mensajeRcv);

	char entrenadorID = mensaje.id;
	agregarEntrenador(entrenadorID, mensaje.nombreEntrenador, socketEntrenador,	mensaje.objetivoActual);
	CrearPersonaje(items, entrenadorID, 1, 1);

	procesarEntrenador(entrenadorID, mensaje.nombreEntrenador);
	//log_info(logMapa, "se creo el estado del proceso para el entrenador: %s",mensaje.nombreEntrenador);

	t_vecRecursos *vec = crearVecRecursos();
	agregarRecursoxEntrenador(&mensaje, vec);

	//imprimirListaEntrenador();
}

void agregarRecursoxEntrenador(t_MensajeEntrenador_Mapa *entrenador, t_vecRecursos *vec) {
	pthread_mutex_lock (&mutexRecursosxEntr);
	char entrenadorID[2] = {0};
	entrenadorID[0] = entrenador->id;
	dictionary_put(recursosxEntr, entrenadorID, vec);
	pthread_mutex_unlock (&mutexRecursosxEntr);
}

void procesarDirectorios(char* pathMapa) {
	//  pathMetadataPokeNest:	/Mapas/[nombre]/PokeNests/[nombre-de-PokeNest]/metadata

	char* pathPokeNest = string_new();
	char* pathPokeNests = string_new();
	int bytes = 0;
	int cantPokemones = 0;
	struct stat estru;
	DIR* dir;

	pathPokeNests = string_from_format("%s/PokeNests\0", pathMapa);

	dir = opendir(pathPokeNests);
	struct dirent* directorio = NULL;
	while ((directorio = readdir(dir)) != NULL) {
		char* nombrePokeNest = directorio->d_name;
		pathPokeNest = string_from_format("%s/%s\0", pathPokeNests, nombrePokeNest);
		stat(directorio->d_name, &estru);
		if (strcmp(nombrePokeNest, ".") == 1 && strcmp(nombrePokeNest, "..") == 1) {
			char* pathMetadataPokeNest = string_new();
			string_append(&pathMetadataPokeNest, pathPokeNest);
			string_append(&pathMetadataPokeNest, "/metadata");
			t_pokeNest pokeNest = getMetadataPokeNest(pathMetadataPokeNest);
			cantPokemones = cantidadDePokemones(pathPokeNest);
			pthread_mutex_lock(&listadoItems);
			CrearCaja(items, pokeNest.id, pokeNest.posx, pokeNest.posy, cantPokemones);
			pthread_mutex_unlock(&listadoItems);
			getPokemones(pathPokeNest, nombrePokeNest);
		}
		bytes = bytes + estru.st_size;
	}
	closedir(dir);
}

int cantidadDePokemones(char* pathPokeNest) {
	//  pathPokeNest:	/Mapas/[nombre]/PokeNests/[nombre-de-PokeNest]
	int bytes = 0, cantPokemones = 0;
	struct stat estru;
	DIR* dir;
	struct dirent* directorio = NULL;
	dir = opendir(pathPokeNest);
	while ((directorio = readdir(dir)) != NULL) {
		char* name = directorio->d_name;
		stat(name, &estru);
		bool esPokemon = false;
		esPokemon = strcmp(name, ".") == 1 && strcmp(name, "..") == 1;
		if (esPokemon) {
			cantPokemones++;
		}
		bytes = bytes + estru.st_size;
	}
	cantPokemones = cantPokemones - 1; //resto 1 por el archivo metadata
	closedir(dir);
	return cantPokemones;
}

void getPokemones(char* pathPokeNest, char* nombrePokeNest){
	//  pathPokemon: 	/Mapas/[nombre]/PokeNests/[PokeNest]/[PokeNest]NNN.dat
	char* pathPokemon = string_new();
	int cantPokemones = cantidadDePokemones(pathPokeNest);
	int numInt = 0;
	if (cantPokemones == 0) return;
	for (numInt = 1; numInt <= cantPokemones; numInt++) {
		//memcpy(numero, &numInt, 1);
		if (numInt < 10) {
			pathPokemon = string_from_format("%s/%s%s%d.dat\0", pathPokeNest,
					nombrePokeNest, "00", numInt);
		} else if (numInt < 100){
			pathPokemon = string_from_format("%s/%s%s%d.dat\0", pathPokeNest,
					nombrePokeNest, "0", numInt);
		} else {
			pathPokemon = string_from_format("%s/%s%d.dat\0", pathPokeNest,
					nombrePokeNest, numInt);
		}
		t_pokemon* unPokemon = malloc(sizeof(t_pokemon));
		unPokemon->species = string_new();
		strcpy(unPokemon->species, nombrePokeNest);
		unPokemon->level = getMetadataPokemon(pathPokemon);
		unPokemon->type = configPokenest.type;
		unPokemon->second_type = configPokenest.second_type;

		pthread_mutex_lock(&listadoPokemones);
		list_add(listaPokemones,(void*) unPokemon);
		pthread_mutex_unlock(&listadoPokemones);
	}
	//free(pathPokemon);
	//free(pathPokeNest);
}

//************* DEADLOCK *************//

void inicializarMatrices() {
	int i, j;

	for (i = 0; i < MAXENTR; i++) {
		vecEntrenadoresEnMapa[i][0] = 0;
		vecEntrenadoresEnMapa[i][1] = 0;
		interbloqueados[i] = '\0';
		for (j = 0; j < MAXREC; j++) {
			matAsignacion[i][j] = 0;
			matSolicitud[i][j] = 0;
			T[j] = 0;
			vecRecursos[j] = 0;
			vecDisponibles[j] = 0;
			vecRecursosCantTotal[j] = 0;
		}
	}
}

void imprimirMatrices() {
	int i, j;
	char a[50] = { 0 };
	char r[50] = { 0 };
	//imprimirListaItems();
	//imprimirListaPokeNests();

	log_debug(logMapa, "\n");
	log_debug(logMapa, "\t\t  vecDisponibles ");
	for (i = 0; i < totalRecursos; i++) {
		r[i] = vecRecursos[i];
		log_debug(logMapa, "\t  recurso: %c   disponible: %d   ", vecRecursos[i],
				vecDisponibles[i]);
	}
	log_debug(logMapa, "\n");
	log_debug(logMapa, "\t copia T de vecDisponibles ");
	for (i = 0; i < totalRecursos; i++) {
		log_debug(logMapa, "\t recurso: %c   disponible: %d  ", vecRecursos[i],
				T[i]);
	}
	log_debug(logMapa, "\n");

	log_debug(logMapa, " matAsignacion ");
	log_debug(logMapa, "   %s ", r);
	totalEntrenadores = list_size(listaEntrenador);
	for (i = 0; i < totalEntrenadores; i++) {
		for (j = 0; j < totalRecursos; j++) {
			a[j] = matAsignacion[i][j] + 48;
		}
		log_debug(logMapa, " %c %s ",vecEntrenadoresEnMapa[i][0], a);
	}
	log_debug(logMapa, "\n");

	log_debug(logMapa, " matSolicitud ");
	log_debug(logMapa, "   %s ", r);
	for (i = 0; i < totalEntrenadores; i++) {
		for (j = 0; j < totalRecursos; j++) {
			//log_debug(logMapa, "\r     matSolicitud[%d][%d]:  %d \n", i, j,  matSolicitud[i][j]);
			a[j] = matSolicitud[i][j] + 48;
		}
		log_debug(logMapa, " %c %s ",vecEntrenadoresEnMapa[i][0], a);
	}
	log_debug(logMapa, "\n");

	log_debug(logMapa, " vecEntrenadoresEnMapa ");
	for (i = 0; i < totalEntrenadores; i++) {
		log_debug(logMapa, " Entrenador: %c   marca: %d",
				vecEntrenadoresEnMapa[i][0], vecEntrenadoresEnMapa[i][1]);
	}
	log_debug(logMapa, "\n");

}

int obtenerPosEntrenador(char e) {
	int i;
	totalEntrenadores = list_size(listaEntrenador);
	for (i = 0; i < totalEntrenadores; i++)
		if (vecEntrenadoresEnMapa[i][0] == e)
			return i;
	return -1;
}

int obtenerPosRecurso(char id) {
	int i;
	for (i = 0; i < totalRecursos; i++)
		if (vecRecursos[i] == id)
			return i;
	return -1;
}

void llenarVecEntrenadorEnMapa() {
	int i, total = list_size(listaEntrenador);
	t_datosEntrenador *entrenador;

	for (i = 0; i < total; i++) {
		entrenador = (t_datosEntrenador*) list_get(listaEntrenador,i);
		vecEntrenadoresEnMapa[i][0] = entrenador->id;
		vecEntrenadoresEnMapa[i][1] = 0;
	}
}

void llenarRecursos() {
	int i = 0;

	void _fillvec(ITEM_NIVEL *caja) {
		vecRecursos[i] = caja->id;
		vecRecursosCantTotal[i] = caja->quantity;
		vecDisponibles[i] = caja->quantity;
		i++;
	}

	list_iterate(pokeNests, (void*) _fillvec);
}

void llenarMatAsignacion(t_dictionary *recursosEntrenador) {
	int p = 0, r = 0, i;

	void _fillvec(char *entrenador, t_vecRecursos *vec) {
		p = obtenerPosEntrenador(entrenador[0]);
		if (p != -1) {
			for (i = 0; i < vec->total; i++) {
				r = obtenerPosRecurso(vec->recurso[i]);
				if (r != -1) {
					matAsignacion[p][r] += 1;
					//vecDisponibles[r] -= 1;
					vecDisponibles[r] = quantity(r);

				}
			}
		}
	}
	dictionary_iterator(recursosEntrenador, (void*) _fillvec);
}

void llenarMatSolicitudAux() {
	int posE = 0, id = 0;

	void _fillvec(t_datosEntrenador *entrenador) {

		posE = obtenerPosEntrenador(entrenador->id);
		if (posE != -1) {
			id = obtenerPosRecurso(entrenador->objetivoActual->id);
			if (id != -1)
				matSolicitud[posE][id] += 1;
		}
	}
	list_iterate(listaEntrenadoresBloqueados, (void*) _fillvec);
}

void llenarMatSolicitud() {
	int posE = 0, id = 0;
	int i = 0;
	int cantPokeNests = list_size(pokeNests);
	while (i < cantPokeNests) {
		int cantEntrenadoresxCola = colasBloqueados[i]->elements->elements_count;
		int j = 0;
		while (j < cantEntrenadoresxCola) {
			if (queue_is_empty(colasBloqueados[i])) {
				break;
			}
			pthread_mutex_lock(&cBloqueados);
			t_entrenadorBloqueado* entr = (t_entrenadorBloqueado*) list_get(colasBloqueados[i]->elements, j);
			pthread_mutex_unlock(&cBloqueados);

			posE = obtenerPosEntrenador(entr->entrenadorID);
			if (posE != -1) {
				id = obtenerPosRecurso(entr->pokeNestID);//"id" deberia ser igual a "i"
				if (id != -1)
					matSolicitud[posE][id] += 1;
			}else{
				log_error(logMapa, "No se encontro el entrenador o el recurso(objetivo) en los vectores");
			}
			j++;
		}
		i++;
	}
}

void interbloqueo() {
	int fin = false;
	int batallaOn = configMapa.batalla;
	int hayDeadLock = 0;

	log_info(logMapa, "HILO DE INTERBLOQUEO: Iniciado.");

	while(!fin) {
		sleep(configMapa.tiempoChequeoDeadlock / 1000);
		int cantBloqueados = 0;
		int j;
		for (j = 0; j < totalRecursos; j++) {
			cantBloqueados = queue_size(colasBloqueados[j]);
			if(cantBloqueados > 0){
				break;
			}
		}
		if (cantBloqueados > 0){

			hayDeadLock = detectarDeadLock();
			imprimirMatrices();

			log_info(logMapa, "hayDeadLock: %d  - batallaOn: %d", hayDeadLock, batallaOn);
			if (hayDeadLock && batallaOn) {
				log_info(logMapa, "Hay interbloqueo y el batalla esta activada...");
				ejecutarBatalla(hayDeadLock);
			}else{
				resolverSolicitudDeCaptura();
			}

			//imprimirMatrices();

			log_info(logMapa, "FINALIZA ALGORITMO DE INTERBLOQUEO...\n");
		}
	}//Fin del While

	//imprimirListaPokeNests();
	//pthread_exit(NULL);

}

void resolverSolicitudDeCaptura(){
	t_entrenadorBloqueado *entr = NULL;
	int i = 0;
	while(i < totalRecursos){
		if(!queue_is_empty(colasBloqueados[i])){
			pthread_mutex_lock(&cBloqueados);
			entr = (t_entrenadorBloqueado*) list_get(colasBloqueados[i]->elements, 0);
			pthread_mutex_unlock(&cBloqueados);
			int posE = obtenerPosEntrenador(entr->entrenadorID);
			int marca = vecEntrenadoresEnMapa[posE][1];
			int cantDisp = vecDisponibles[i];
			if(marca == 0 || cantDisp == 0){
				break;
			}

			pthread_mutex_lock(&cBloqueados);
			entr = (t_entrenadorBloqueado*) list_remove(colasBloqueados[i]->elements, 0);
			pthread_mutex_unlock(&cBloqueados);
			time_t final;
			final = time(NULL);
			double timer = difftime(final, entr->tiempoBloqueado);
			log_info(logMapa,
					"Segundos transcurridos desde que %s estuvo bloqueado: %.2f s",
					entr->nombre,
					timer);
			int socketEntr = buscarSocketEntrenador(entr->nombre);
			enviar(&socketEntr, &timer, sizeof(double));
			//Cambio el proceso a estado LISTO
			log_info(logMapa,
					"Saco el procesoEntrenador %s de cola Bloqueados %d ",
					entr->nombre, i);
			cambiarEstadoProceso(entr->entrenadorID, LISTO);
			int pos = buscarProceso(entr->entrenadorID);
			pthread_mutex_lock(&listadoProcesos);
			t_procesoEntrenador* infoProceso = (t_procesoEntrenador*) list_get(listaProcesos, pos);
			pthread_mutex_unlock(&listadoProcesos);

			t_datosEntrenador* entrenador = searchEntrenador(infoProceso->id);
			int socketEntrenador = entrenador->numSocket;
			int resolucionCaptura;
			int posRec = obtenerPosRecurso(entrenador->objetivoActual->id);
			int cantRecurso = quantity(posRec);
			if (cantRecurso > 0) {
				restarRecurso(items, entrenador->objetivoActual->id);
				entrenador->objetivoActual->quantity--;
				incrementarRecursoxEntrenador(entrenador, entrenador->objetivoActual->id);

				//0 es un aviso para que capture pokemon y le envio el pokemon
				resolucionCaptura = 0;
				enviar(&socketEntrenador, &resolucionCaptura, sizeof(int));
				int k = 0;
				while(k < list_size(listaPokemones)){

					pthread_mutex_lock (&listadoPokemones);
					t_pokemon* pokemonDeLista = (t_pokemon*) list_get(listaPokemones, k);
					pthread_mutex_unlock (&listadoPokemones);

					if(pokemonDeLista->species[0] == entr->pokeNestID){

						log_info(logMapa, "Entrenador: %s captura Pokemon: %s", entrenador->nombre, pokemonDeLista->species);
						enviarPokemon(socketEntrenador, pokemonDeLista);
						break;
					}
					k++;
				}
			}

			t_procesoEntrenador* procEntrenador = malloc(sizeof(t_procesoEntrenador));
			procEntrenador->nombre = string_new();
			strcpy(procEntrenador->nombre, infoProceso->nombre);
			memcpy(procEntrenador, infoProceso, sizeof(t_procesoEntrenador));

			pthread_mutex_lock(&cListos);
			queue_push(colaListos, (void*) procEntrenador);
			pthread_mutex_unlock(&cListos);
			log_info(logMapa,"%s (%c) Sale de Bloqueados y entra en Listos",infoProceso->nombre, infoProceso->id);
			imprimirColaListos();
			//break;
		}
		i++;
	}
}

void incrementarRecursoxEntrenador(t_datosEntrenador *entrenador, char idRecurso) {
	pthread_mutex_lock (&mutexRecursosxEntr);
	t_vecRecursos *vec;
	char entrenadorID[2] = {0};
	entrenadorID[0] = entrenador->id;
	vec = dictionary_get(recursosxEntr, entrenadorID);
	vec->recurso[vec->total++] = idRecurso;
	pthread_mutex_unlock (&mutexRecursosxEntr);
}

t_vecRecursos* removerRecursoxEntrenador(t_datosEntrenador *entrenador) {
	pthread_mutex_lock (&mutexRecursosxEntr);
	t_vecRecursos *vec;
	char entrenadorID[2] = {0};
	entrenadorID[0] = entrenador->id;
	vec = dictionary_remove(recursosxEntr, entrenadorID);
	pthread_mutex_unlock (&mutexRecursosxEntr);
	return vec;
}

/**
 * 1) Se marca cada proceso que tenga una fila de la matriz de Asignacion completamente a cero
 * 2) Se inicia un vector temporal T asignandole el vector de disponibles
 * 3) Se busca un indice i tal que el proceso i no este marcado actualmente y la fila i-esima de matSolicitud
 * 	  sea menor o igual a T (disponibles).
 *    Es decir, Se ejecuta Tk = Tk + Aik, para 1 <= k <= m. A continuacion se vuelve al 3er paso.
 */
int detectarDeadLock() {
	log_info(logMapa, "INICIO DE DETECCION DE DEADLOCK ...");
	int hayDeadLock;

	hayDeadLock = 0;

	inicializarMatrices();

	// Algoritmo de deteccion de interbloqueo
	// Lleno las matrices y los vectores necesarios.

	//imprimirListaPokeNests();
	//imprimirListaItems();
	t_dictionary *recursosxEntr = crearDiccRecursosxEntr();

	llenarVecEntrEnMapa();
	llenarRecursos();
	llenarMatAsignacion(recursosxEntr);
	llenarMatSolicitud();

	totalEntrenadores = list_size(listaEntrenador);
	log_info(logMapa, "\n INTERBLOQUEO: totalEntrenadores: %d, totalRecursos: %d", totalEntrenadores, totalRecursos);
	//imprimirMatrices();

	// 1) Se marca cada proceso que tenga una fila de la matriz de Asignacion completamente a cero
	// Tomo lista de entrenadores en Mapa que no tengan asignado ningun recurso, y lo marco
	marcarEntrSinRecursosAsig();

	// 2) Se inicia un vector temporal T asignandole el vector de disponibles
	copiarDisponiblesAT();

	// 3) Se busca un indice i tal que el proceso i no este marcado actualmente (que tenga asignado algun rec)
	//    y la fila i-esima de la matSolicitud sea menor o igual a T (disponibles).
	marcarNoBloqueados();

	// Existe un interbloqueo si y solo si hay procesos sin marcar al final del algoritmo
	hayDeadLock = contarEntrSinMarcar();

	if (hayDeadLock > 1){
		log_info(logMapa, "HAY INTERBLOQUEO:");
		return hayDeadLock;
	}

	//queue_destroy_and_destroy_elements(colasBloqueados, (void*)destruirEntrenador);
	//dictionary_destroy_and_destroy_elements(recursosxEntr, (void*)destruirVecRecursos);
	//TODO agregar destructores

	return 0;// 0 no hay deadlock

}

void destruirVecRecursos(t_vecRecursos *vecRecursos) {
	free(vecRecursos);
}


void llenarVecEntrEnMapa() {
	int i;
	int total = list_size(listaEntrenador);
	t_datosEntrenador *entr ;
	for (i = 0; i < total; i++) {
		pthread_mutex_lock(&listadoEntrenador);
		entr = (t_datosEntrenador*) list_get(listaEntrenador,i);
		pthread_mutex_unlock(&listadoEntrenador);
		vecEntrenadoresEnMapa[i][0] = entr->id;
		vecEntrenadoresEnMapa[i][1] = 0;
	}
}

t_dictionary* crearDiccRecursosxEntr() {
	pthread_mutex_lock (&mutexRecursosxEntr);
	t_dictionary *dicc = dictionary_create();
	t_vecRecursos *vecRecursos;
	void _addToDicc(char *key, t_vecRecursos *vec) {
		vecRecursos = crearVecRecursos();
		memcpy(vecRecursos->recurso, vec->recurso, sizeof(vec->recurso));
		vecRecursos->total = vec->total;
		dictionary_put(dicc, key, vecRecursos);
	}
	dictionary_iterator(recursosxEntr, (void*)_addToDicc);
	pthread_mutex_unlock (&mutexRecursosxEntr);
	return dicc;
}

t_vecRecursos* crearVecRecursos() {
	t_vecRecursos *vec = calloc(1, sizeof(t_vecRecursos));
	vec->total = 0;
	return vec;
}

t_datosEntrenador* ejecutarBatalla(int cantInterbloqueados) {	//todo BATALLA
	log_info(logMapa, "Incio proceso de batalla deadlock... cantidad Entrenadores INTERBLOQUEADOS: %d", cantInterbloqueados);
	t_datosEntrenador* entrenador = NULL;
	t_datosEntrenador* entrenadorDeLoser = NULL;
	int totalEntrenadoresEnMapa = 0;
	int i, j = 0;
	bool encontreLoser = false;
	t_pkmn_factory* pokemon_factory = create_pkmn_factory();
	t_pokemon* loser = NULL;
	int resolucionCaptura = 1;

	//						B A T A L L A
	// 1- Recibo el pokemon del primer entrenador interbloqueado que entro al mapa
	// 2- Recibo el pokemon del segundo entrenador interbloqueado que entro al mapa
	// 3- Se efectua la batalla y el pokemon perdedor batalla con el
	//    siguiente entrenador interbloqueado que entro al mapa.
	// 4- Asi sucesivamente hasta seleccionar la victima (del ultimo pokemon perdedor)

	// 				R E S O L U C I O N     D E      D E A D L O C K
	// 1- Seleccionar la victima (es la entrenador que perdio la ultima batalla)
	// 2- Mover al entrenador seleccionado de las listas (deberia estar en bloqueados solamente)
	//    y agregarlo a la lista muertosxBatalla.
	// 3- Informar al Entrenador que esta muerto

	totalEntrenadoresEnMapa = list_size(listaEntrenador);
	log_debug(logMapa, "totalEntrenadoresEnMapa: %d: ", totalEntrenadoresEnMapa);

	for (i=0; i < totalEntrenadoresEnMapa; i++){
		pthread_mutex_lock(&listadoEntrenador);
		entrenador = list_get(listaEntrenador, i);
		pthread_mutex_unlock(&listadoEntrenador);
		double tiempoBloqueado = 0;
		for (j=0; j < cantInterbloqueados; j++){
			if (entrenador->id == interbloqueados[j]){
				log_debug(logMapa,"Entrenador: %c ", interbloqueados[j]);
				if(!encontreLoser){
					entrenadorDeLoser = entrenador;
					enviar(&entrenadorDeLoser->numSocket, &tiempoBloqueado, sizeof(double));
					enviar(&entrenadorDeLoser->numSocket, &resolucionCaptura, sizeof(int));
					loser = recibirPokemon(entrenadorDeLoser->numSocket);

					loser = create_pokemon(pokemon_factory,loser->species, loser->level);
					log_info(logMapa, "Pokémon 1: %s, [%s/%s] Nivel: %d",
							loser->species, pkmn_type_to_string(loser->type),
							pkmn_type_to_string(loser->second_type), loser->level);
					if (loser == NULL){
						log_error(logMapa,
								"El Pokémon 1 %s no existe! El puntero de retorno de la factory fue: %p \n",
								loser->species, loser);
					}
					encontreLoser = true;
					break;
				}else{
					enviar(&entrenador->numSocket, &tiempoBloqueado, sizeof(double));
					enviar(&entrenador->numSocket, &resolucionCaptura, sizeof(int));
					t_pokemon* pokemon2 = recibirPokemon(entrenador->numSocket);

					pokemon2 = create_pokemon(pokemon_factory,pokemon2->species, pokemon2->level);
					log_info(logMapa, "Pokémon 2: %s, [%s/%s] Nivel: %d",
							pokemon2->species, pkmn_type_to_string(pokemon2->type),
							pkmn_type_to_string(pokemon2->second_type), pokemon2->level);
					if (pokemon2 == NULL){
						log_error(logMapa,
								"El Pokémon %s no existe! El puntero de retorno de la factory fue: %p \n",
								pokemon2->species, pokemon2);
					}

					log_debug(logMapa,"BATALLA entre: %s y %s", loser->species, pokemon2->species);
					loser = pkmn_battle(loser, pokemon2);

					int ganaste = 1;
					int perdiste;
					t_datosEntrenador* entrenadorGanador = entrenador;
					if ( j + 1 == cantInterbloqueados){//verifico si hay mas interbloqueados
						perdiste = 0;
						flagBatalla = true;

					}else{
						perdiste = -1;// -1 es porque queda al menos 1 batalla mas
					}

					if (strcmp(loser->species, pokemon2->species) == 0) {
						entrenadorGanador = entrenadorDeLoser;
						entrenadorDeLoser = entrenador;// actualizo el nuevo entrenador que perdio
					}
					enviar(&entrenadorGanador->numSocket, &ganaste, sizeof(int));
					enviar(&entrenadorDeLoser->numSocket, &perdiste, sizeof(int));

					log_info(logMapa,
							"El Perdedor es: %s del entrenador(%c): %s\n",
							loser->species, entrenadorDeLoser->id,
							entrenadorDeLoser->nombre);

					if (perdiste == 0){
						quitarEntrBloqueado(entrenadorDeLoser);
						pthread_mutex_lock(&listadoEntrMuertosxBatalla);
						list_add(listaEntrMuertosxBatalla, (void*) entrenadorDeLoser);
						pthread_mutex_unlock(&listadoEntrMuertosxBatalla);
						liberarRecursos(entrenadorDeLoser);
						//Marco al entrenador ganador en 1 para q capture
						t_datosEntrenador * unEntrenador;
						int total = list_size(listaEntrenador);
						for (i = 0; i < total; i++) {
							unEntrenador = (t_datosEntrenador*) list_get(listaEntrenador,i);
							if(unEntrenador->id == entrenadorGanador->id){
								vecEntrenadoresEnMapa[i][1] = 1;
							}
						}

						resolverSolicitudDeCaptura();
						sem_post(&entrMuerto);
						sem_post(&mutexEntr);
					}
					free(pokemon2);//loser no se libera porque puede volver a utilizarse

					break;
				}
			}
		}

	}
	destroy_pkmn_factory(pokemon_factory);

	/*for (i=0; i < totalEntrenadoresEnMapa; i++) {
		if (encontreVictima == 0) {
			for (j = 0; j < cantInterbloqueados; j++) {
				if(entrenador->id == interbloqueados[j]) {
					entrenadorMuerto = entrenador;
					//quitarEntrBloqueado(entrenadorMuerto);
					pthread_mutex_lock(&listadoEntrMuertosxBatalla);
					list_add(listaEntrMuertosxBatalla, (void*) entrenadorMuerto);
					pthread_mutex_unlock(&listadoEntrMuertosxBatalla);
					encontreVictima = 1;
					break;
				}
			}
		}
	}*/

	return entrenadorDeLoser;
}

void liberarRecursos(t_datosEntrenador* entrenadorMuerto){

	int i = 0, cantPokemones = 0;
	recibir(&entrenadorMuerto->numSocket, &cantPokemones, sizeof(int));
	while (i < cantPokemones){
		t_pokemon* pokemon = recibirPokemon(entrenadorMuerto->numSocket);
		pthread_mutex_lock(&listadoPokemones);
		list_add(listaPokemones, (void*) pokemon);
		pthread_mutex_unlock(&listadoPokemones);
		sumarRecurso(items, pokemon->species[0]);
		dibujar();

		log_trace(logMapa, "nombre del pokemon recibido: %s", pokemon->species);
		i++;
	}
	t_vecRecursos *vec;
	vec = removerRecursoxEntrenador(entrenadorMuerto);
	if (vec == NULL){
		log_error(logMapa,
				"ERROR entrenador %s no tiene recursos en recursosxEntrenador",
				entrenadorMuerto->nombre);
		exit(-1);
	}
	free(vec);
	log_info(logMapa, "Se elimina a %s (%c) de todas las listas", entrenadorMuerto->nombre, entrenadorMuerto->id);

	int posE = buscarEntrenador(entrenadorMuerto->numSocket);
	pthread_mutex_lock(&listadoEntrenador);
	t_datosEntrenador* entrenador = list_remove(listaEntrenador, posE);
	pthread_mutex_unlock(&listadoEntrenador);

	pthread_mutex_lock(&listadoItems);
	BorrarItem(items, entrenadorMuerto->id);
	pthread_mutex_unlock(&listadoItems);

	pthread_mutex_lock(&listadoProcesos);
	BorrarItem(listaProcesos, entrenadorMuerto->id);
	pthread_mutex_unlock(&listadoProcesos);

	free(entrenador->objetivoActual);
	free(entrenador);

}

void quitarEntrBloqueado(t_datosEntrenador* entrenador) {
	pthread_mutex_lock (&cBloqueados);
	t_entrenadorBloqueado *entr = NULL;
	int posRec = obtenerPosRecurso(entrenador->objetivoActual->id);
	int j = 0 ;
	while(j < totalEntrenadores){
		if(queue_is_empty(colasBloqueados[posRec])){
			break;
		}
		entr = (t_entrenadorBloqueado*) list_get(colasBloqueados[posRec]->elements, j);
		if (entr->entrenadorID == entrenador->id){
			list_remove(colasBloqueados[posRec]->elements, j);
		}

		j++;
	}
	pthread_mutex_unlock (&cBloqueados);
}


void marcarEntrSinRecursosAsig(){
	int i,j, total=0;
	totalEntrenadores = list_size(listaEntrenador);
	for (i=0; i < totalEntrenadores; i++){
		total = 0;
		for (j=0; j < totalRecursos; j++){
			 total +=matAsignacion[i][j];
		}
		if (total == 0){
			vecEntrenadoresEnMapa[i][1] = 1;
		}
	}
}

void copiarDisponiblesAT(){
	int i;
	for (i=0; i < totalRecursos; i++){
		T[i] = vecDisponibles[i];
		//T[i] = quantity(i);
	}
}

void marcarNoBloqueados() {
	int i, k;
	bool solicitudMIDisp;
	int continuar = true;

	/* 3) Se busca un indice i tal que el proceso i no este marcado actualmente
	      y la fila i-esima de S(olicitud) sea menor o igual a T (disponibles).
	      Es decir, Sik <= Tk, para 1<=k<=m. Si no se encuentra ninguna fila el algoritmo termina
	*/
	while(continuar) {
		continuar = false;
		totalEntrenadores = list_size(listaEntrenador);
		for (i = 0; i < totalEntrenadores; i ++) {

			// Se busca un indice i tal que el proceso i no este marcado actualmente
			if (vecEntrenadoresEnMapa[i][1] == 0) {
				solicitudMIDisp = true;

				// y la fila i-esima de S(olicitud) sea menor o igual a T (disponibles).
				// Toda la fila debe cumplir este requerimiento!
				for (k=0; k < totalRecursos; k++) {
					solicitudMIDisp = solicitudMIDisp && (matSolicitud[i][k] <= T[k]);
				}

				if (solicitudMIDisp) {
					// 4) Si se encuentra una fila que lo cumpla, se marca el proceso i
					vecEntrenadoresEnMapa[i][1] = 1;

					// y se suma la fila correspondiente de la matriz de Asignacion a T
					// Es decir, Se ejecuta Tk = Tk + Aik, para 1 <= k <= m.
					for (k=0; k < totalRecursos; k++) {
						vecDisponibles[k] = quantity(k);
						T[k] = vecDisponibles[k];
					}

					// A continuacion se vuelve al tercer paso
					continuar = true;
					break;
				}
			}
		}
	}
}

int quantity(int k){
	pthread_mutex_lock(&listadoPokeNests);
	ITEM_NIVEL* unaPokeNest = (ITEM_NIVEL*) list_get(pokeNests, k);
	pthread_mutex_unlock(&listadoPokeNests);
	return unaPokeNest->quantity;
}

int contarEntrSinMarcar() {
	int i, cont = 0;
	totalEntrenadores = list_size(listaEntrenador);
	for (i = 0; i < totalEntrenadores; i ++) {
		// Se busca un indice i tal que el proceso i no este marcado actualmente
		if (vecEntrenadoresEnMapa[i][1] == 0) {
			interbloqueados[cont] = vecEntrenadoresEnMapa[i][0];
			cont++;
		}
	}
	if(cont > 1){
		for (i = 0; i < cont; i ++) {
			log_debug(logMapa, "\n ----- INTERBLOQUEADO %d: %c\n", i + 1, interbloqueados[i] );
		}
	}
	return cont;
}
