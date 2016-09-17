/*
 * mapa.c
 *
 */

#include "mapa.h"

int main(int argc, char **argv) {
	fflush(stdin);
	system("clear");
	//assert(("ERROR - No se pasaron argumentos", argc > 1)); // Verifica que se haya pasado al menos 1 parametro, sino falla
	pthread_t finalizarMapaThread;
	pthread_t planificadorRRThread;
	pthread_t planificadorSRDFThread;
	pthread_t serverThread;
	sem_init(&configOn, 0, 0);
	sem_init(&mutex, 0, 1);
	//sem_init(&mutex5, 0, 4);
	sem_init(&planif, 0, 2);
	sem_init(&mejorEntrenador, 0, 3);

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
	//solo para probar todo
	configMapa.nombre = "PuebloPaleta";
	configMapa.pathPokedex = "/home/utnso/Pokedex";
	//char* logFile = "/home/utnso/git/tp-2016-2c-SegmentationFault/Mapa/logMapa";
	//solo para probar todo
	//Creo el archivo de Log
	logMapa = log_create("logMapa", "MAPA", 0, LOG_LEVEL_TRACE);

	//Inicializacion de listas y mutex
	crearListas();
	inicializarMutex();

	//Obtengo informacion de archivos e inicializo mapa
	pthread_t configThread;
	pthread_create(&configThread, NULL, (void*) getArchivosDeConfiguracion, NULL);
	//getArchivosDeConfiguracion();

	sem_wait(&configOn);
	sem_wait(&mutex);

	pthread_create(&finalizarMapaThread, NULL, (void*) quitGui, NULL);
	if (strcmp(configMapa.algoritmo, "RR") == 0) {
			pthread_create(&planificadorRRThread, NULL, (void*) planificarProcesoRR,NULL);
		} else {
			pthread_create(&planificadorSRDFThread, NULL,(void*) planificarProcesoSRDF, NULL);
	}

	//Conexion con el entrenador
	//int socketSv = 0;
	//socketEntrenador = ponerAEscuchar(conexion.ip, conexion.puerto);//todo
	//procesarRecibir(socketEntrenador);

	//pthread_create(&serverThread, NULL, (void*) startServer, NULL);

	int rows = 19, cols = 78;
	nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&rows, &cols);
	char* mapa = string_new();
	string_append_with_format(&mapa,"%s%s\0","Mapa: ",configMapa.nombre);
	nivel_gui_dibujar(items, mapa);

	startServer();

	//escucharMultiplesConexiones(&socketEntrenador,conexion.puerto);

	//pthread_create(&serverThread, NULL, (void*) startServerProg, NULL);

	//ejemploProgramaGui();

	void destruirItem(ITEM_NIVEL* item) {
		free(item);
	}
	void destruirEntrenador(t_datosEntrenador* entrenador) {
		free(entrenador);
		free(entrenador->objetivoActual);
	}
	void destruirProcesoEntrenador(t_procesoEntrenador* procesoEntrenador) {
		free(procesoEntrenador);
	}
	list_destroy_and_destroy_elements(items, (void*) destruirItem);
	list_destroy_and_destroy_elements(listaEntrenador, (void*) destruirEntrenador);
	list_destroy_and_destroy_elements(listaProcesos, (void*) destruirProcesoEntrenador);

	nivel_gui_terminar();

	pthread_join(finalizarMapaThread, NULL);
	if (strcmp(configMapa.algoritmo, "RR") == 0){
		pthread_join(planificadorRRThread, NULL);
	} else {
		pthread_join(planificadorSRDFThread, NULL);
	}

	//pthread_join(serverThread, NULL);
	pthread_join(configThread, NULL);


	//close(socketSv);

	return 1;

}

void startServer(){
	int socketSv = 0;
	abrirConexionDelServer(conexion.ip, conexion.puerto,&socketSv);
	while(1){

		clienteNuevo((void*)&socketSv);

	}
}

void clienteNuevo(void *parametro){
	t_server* datosServer=malloc(sizeof(t_server));
	memcpy(&datosServer->socketServer,parametro,sizeof(datosServer->socketServer));
	pthread_attr_t hiloDeAceptarConexiones;
	pthread_attr_init(&hiloDeAceptarConexiones);
	pthread_attr_setdetachstate(&hiloDeAceptarConexiones,PTHREAD_CREATE_DETACHED);
	pthread_t hiloDeAceptarClientes;
	pthread_create(&hiloDeAceptarClientes,&hiloDeAceptarConexiones,(void*)aceptarConexionDeUnClienteHilo,&datosServer);
	pthread_attr_destroy(&hiloDeAceptarConexiones);
	aceptarConexionDeUnCliente(&datosServer->socketCliente,&datosServer->socketServer);
	log_info(logMapa,"datos del cliente: %i \n",datosServer->socketCliente);
	int socketEntrenador = datosServer->socketCliente;

	enum_procesos fromProcess;
	recibir(&socketEntrenador, &fromProcess, sizeof(fromProcess));
	if (fromProcess == ENTRENADOR) recibirInfoInicialEntrenador(socketEntrenador);

	char* mapa = string_new();
	string_append_with_format(&mapa,"%s%s\0","Mapa: ",configMapa.nombre);
	nivel_gui_dibujar(items, mapa);

	flagPlanificar = 1;

	pthread_attr_t procesarMensajeThread;
	pthread_attr_init(&procesarMensajeThread);
	pthread_attr_setdetachstate(&procesarMensajeThread, PTHREAD_CREATE_DETACHED);

	pthread_t processMessageThread;
	pthread_create(&processMessageThread, NULL, (void*) ejecutarPrograma, &socketEntrenador);

	//while(1){
	//ejecutarPrograma();
	//}
	pthread_attr_destroy(&procesarMensajeThread);

	//nivel_gui_dibujar(items, mapa);
	//pthread_join(processMessageThread, NULL);
}

void ejecutarPrograma(int* socketEntrenador){
	while (1){
		if (flagPlanificar == 0){
			int bytesRecibidos = 0;
			char entrenadorID = 0;
			//sem_wait(&mejorEntrenador);
			//sem_wait(&mutex);

			//sem_wait(&planif);
			//sem_wait(&mutex);
			if (*socketEntrenador == socketEntrenadorActivo){
				pthread_mutex_lock(&procesoActivo);
			}
			enum_procesos fromProcess;
			recibir(&socketEntrenadorActivo, &fromProcess, sizeof(fromProcess));
			//if (fromProcess == ENTRENADOR) bytesRecibidos = reconocerOperacion(socketEntrenadorActivo);
			if (fromProcess == ENTRENADOR) entrenadorID = reconocerOperacion(&bytesRecibidos);

			if (*socketEntrenador == socketEntrenadorActivo){
				pthread_mutex_unlock(&procesoActivo);
			}

			//sem_post(&mejorEntrenador);
			//sem_post(&mutex);

			int pos = buscarProceso(entrenadorID);
			if( pos != -1){
				//todo
				pthread_mutex_lock(&listadoProcesos);
				t_procesoEntrenador* procesoEntrenador = (t_procesoEntrenador*) list_get(listaProcesos, pos);
				pthread_mutex_unlock(&listadoProcesos);

				t_procesoEntrenador* unEntrenador = malloc(sizeof(t_procesoEntrenador));
				memcpy(unEntrenador, procesoEntrenador, sizeof(t_procesoEntrenador));


				pthread_mutex_lock(&cListos);
				queue_push(colaListos, (void*) unEntrenador);
				pthread_mutex_unlock(&cListos);
				//log_info(logMapa,"agrego a cola de Listos unEntrenador de id: %c", unEntrenador->id);
			}
			imprimirColaListos();
			if (bytesRecibidos == -1){
				flagPlanificar = -1;
			}else{
				flagPlanificar = 1;
			}

			char* mapa = string_new();
			string_append_with_format(&mapa,"%s%s\0","Mapa: ",configMapa.nombre);

			nivel_gui_dibujar(items, mapa);


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


}


char reconocerOperacion(int* bytesRecibidos){//todo
	t_MensajeEntrenador_Mapa mensaje;

	//Recibo mensaje usando su tamanio
	char* mensajeRcv = malloc(sizeof(t_MensajeEntrenador_Mapa));
	memset(mensajeRcv, '\0', sizeof(t_MensajeEntrenador_Mapa));
	*bytesRecibidos = recibir(&socketEntrenadorActivo, mensajeRcv, sizeof(t_MensajeEntrenador_Mapa));

	if (*bytesRecibidos <= 0) {
		//log_error(logMapa,"se recibio un tamanio distinto al esperado");
		return -1;
	}
	deserializarMapa_Entrenador(&mensaje, mensajeRcv);
	free(mensajeRcv);
	//log_info(logMapa,"id: %c\n", mensaje.id);
	//log_info(logMapa,"operacion: %d\n", mensaje.operacion);
	//log_info(logMapa,"nombre: %s\n", mensaje.nombreEntrenador);

	t_datosEntrenador* entrenador = searchEntrenador(mensaje.id);

	switch (mensaje.operacion) {
	case 1:{
		enviarPosPokeNest(entrenador,socketEntrenadorActivo);
		*bytesRecibidos = -1;
		//flagPlanificar = -1;
		break;
	}
	case 2:{
		int posx = -1;
		int posy = -1;
		*bytesRecibidos = recibir(&socketEntrenadorActivo, &posx, sizeof(int));
		*bytesRecibidos = recibir(&socketEntrenadorActivo, &posy, sizeof(int));
		entrenador->posx = posx;
		entrenador->posy = posy;
		pthread_mutex_lock(&listadoItems);
		MoverPersonaje(items, entrenador->id, entrenador->posx, entrenador->posy);
		pthread_mutex_unlock(&listadoItems);

		//todo enviar el informe del uso de una unidad de tiempo
		usleep(configMapa.retardo*1000);

		break;
	}
	case 3:{
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

			//if (hayOtroEntrenador > 1 && configMapa.batalla == 0) {//todo ver si hay q bloquear siempre (o solo aca)
		int pos = buscarPosPokeNest(entrenador->objetivoActual->id);
		t_entrenadorBloqueado* entrenadorBloqueado = malloc(sizeof(t_entrenadorBloqueado));
		entrenadorBloqueado->index = pos;
		entrenadorBloqueado->entrenadorID = entrenador->id;
		entrenadorBloqueado->nombre = entrenador->nombre;
		entrenadorBloqueado->pokeNestID = entrenador->objetivoActual->id;
		time_t comienzo;
		comienzo = time( NULL );
		entrenadorBloqueado->tiempoBloqueado = comienzo;
		pthread_mutex_lock(&cBloqueados);
		queue_push(colasBloqueados[pos], (void*) entrenadorBloqueado);
		pthread_mutex_unlock(&cBloqueados);
		*bytesRecibidos = 1;
			/*} else if (!hayOtroEntrenador) {
				restarRecurso(items, entrenador->objetivoActual->id);
				// informar al entrenador de objetivo cumplido y recibir otro nuevo objetivo
				int resolucionCaptura = 0;
				enviar(&socketEntrenadorActivo, &resolucionCaptura, sizeof(int));//hasta aca llega bien
				//actualizo objetivo todo
				char idNuevoObjetivo;
				bytesRecibidos = recibir(&socketEntrenadorActivo, &idNuevoObjetivo, sizeof(char));
				entrenador->objetivoActual = _search_item_by_id(items, idNuevoObjetivo);
				objx = entrenador->objetivoActual->posx;
				objy = entrenador->objetivoActual->posy;
			}
		}*/
		break;
	}
	case 4:{
		//getMetadataMedalla();
		//enviar pathMedalla
		break;
	}
	case 5:{ 	//Fin de quantum
		log_info(logMapa, "Se procesa el fin del Quantum");
		atenderFinDeQuantum(socketEntrenadorActivo, mensaje.id);
		break;
	}
	case 6:{
		//finalizaProceso(socketEntrenadorActivo, mensaje.entrenadorID);
		*bytesRecibidos = -1;
		break;
	}
	case 7:{	//ejemplo de como recibir y enviar texto
		//Recibo el tamanio del texto
		int tamanio;
		*bytesRecibidos = recibir(&socketEntrenadorActivo, &tamanio,sizeof(int));
		char* texto = malloc(tamanio);

		//Recibo el texto
		*bytesRecibidos = recibir(&socketEntrenadorActivo, texto, tamanio);

		// Envia el tamanio del texto al Entrenador
		log_info(logMapa, "Tamanio: '%d'", tamanio);
		string_append(&texto,"\0");
		enviar(&socketEntrenadorActivo, &tamanio, sizeof(int));

		// Envia el texto al proceso Entrenador
		log_info(logMapa, "Texto : '%s'", texto);
		enviar(&socketEntrenadorActivo, texto, tamanio);

		free(texto);
		break;
	}
	case 10:{
		atenderFinDeQuantum(socketEntrenadorActivo, mensaje.id);
		break;
	}
	default:{
		log_error(logMapa, "Mensaje recibido invalido. ");
		//printf("Entrenador desconectado.");
	}
	}
	char entrenadorID = mensaje.id;

	//free(mensajeRcv);

	return entrenadorID;
}

void funcionTime() {
	time_t comienzo, final;

	comienzo = time( NULL );
	final = time( NULL );

	printf("Comienzo: %d\n", (int) comienzo);
	printf("Final: %d\n", (int) final);
	printf("Número de segundos transcurridos desde el comienzo del programa: %f s\n",
			difftime(final, comienzo));
}

void procesarRecibir(int socketEntrenador){
	char* mensaje="holaM";
	enviar(&socketEntrenador,mensaje,6);
	char* respuesta=malloc(6);
	recibir(&socketEntrenador,respuesta,6);
	printf("mensaje recibido desde el entrenador: %s\n",respuesta);
	free(respuesta);
}

void procesarEntrenador(char entrenadorID,char* nombreEntrenador) {
	t_procesoEntrenador* procesoEntrenador = malloc(sizeof(t_procesoEntrenador));
	procesoEntrenador->nombre = string_new();
	procesoEntrenador->id = entrenadorID;
	strcpy(procesoEntrenador->nombre, nombreEntrenador);
	procesoEntrenador->programCounter = 0;
	procesoEntrenador->estado = LISTO;
	procesoEntrenador->finalizar = 0;

	pthread_mutex_lock(&listadoProcesos);
	list_add(listaProcesos, (void*) procesoEntrenador);
	pthread_mutex_unlock(&listadoProcesos);

	t_procesoEntrenador* unEntrenador = malloc(sizeof(t_procesoEntrenador));
	memcpy(unEntrenador, procesoEntrenador, sizeof(t_procesoEntrenador));

	//Agrego a la Cola de Listos
	pthread_mutex_lock(&cListos);
	queue_push(colaListos, (void*) unEntrenador);
	pthread_mutex_unlock(&cListos);

	//todo
	//log_info(logMapa, "se agrego a la cola de listos al entrenador %s",nombreEntrenador);

	imprimirColaListos();
}

void atenderFinDeQuantum(int socketEntrenador,char id){
	int pos = buscarEntrenador(socketEntrenador);
	pthread_mutex_lock(&listadoEntrenador);
	t_datosEntrenador* datosEntrenador = (t_datosEntrenador*) list_remove(listaEntrenador, pos);
	pthread_mutex_unlock(&listadoEntrenador);

	free(datosEntrenador);

	//Cambio el PC del Proceso, le sumo el quantum al PC actual.
	t_procesoEntrenador* infoProceso;
	int buscar = buscarProceso(id);
	pthread_mutex_lock(&listadoProcesos);
	infoProceso = (t_procesoEntrenador*)list_get(listaProcesos,buscar);
	pthread_mutex_unlock(&listadoProcesos);

	int quantumUsado= 0;
	recibir(&socketEntrenador, &quantumUsado, sizeof(int));

	int pcnuevo = infoProceso->programCounter + quantumUsado;
	infoProceso->programCounter = pcnuevo;

	//Agrego el proceso a la Cola de Listos
	t_procesoEntrenador* procesoEntrenador = malloc(sizeof(t_procesoEntrenador));
	procesoEntrenador->id = id;
	procesoEntrenador->programCounter = pcnuevo;

	if (infoProceso->finalizar == 0){
		//Cambio el estado del proceso
		int estado = 1;
		cambiarEstadoProceso(id, estado);
		pthread_mutex_lock(&cListos);
		queue_push(colaListos, (void*) procesoEntrenador);
		pthread_mutex_unlock(&cListos);
	} else {
		pthread_mutex_lock(&cFinalizar);
		queue_push(colaFinalizar, (void*) procesoEntrenador);
		pthread_mutex_unlock(&cFinalizar);
	}
}

void planificarProcesoSRDF() {
	while (1){
		//Veo si hay procesos para planificar en la cola de Listos
		if (queue_is_empty(colaListos) && (queue_is_empty(colaFinalizar))) {

		}else if (flagPlanificar != 0){
			imprimirColaListos();

			pthread_mutex_lock(&varGlobal);
			socketEntrenadorActivo = entrenadorMasCercano();
			if(flagPlanificar != -1){
				enviarMensajeTurnoConcedido();
			}
			pthread_mutex_unlock(&varGlobal);

			//Saco el primer elemento de la cola, porque ya lo planifique.
			pthread_mutex_lock(&cListos);
			t_procesoEntrenador* proceso = (t_procesoEntrenador*) queue_pop(colaListos);
			pthread_mutex_unlock(&cListos);

			//log_info(logMapa, "Se libera de la cola de listos el proceso del Entrenador: '%s'. ID: %c", proceso->nombre, abs(proceso->id));
			free(proceso);

			//todo Probar
			imprimirColaListos();
			//log_info(logMapa,"paso por el planificador mas cercano");
			pthread_mutex_lock(&varGlobal);
			flagPlanificar = 0;
			pthread_mutex_unlock(&varGlobal);
		}
	}

}

void planificarProcesoRR() {
	while (1){
		//Veo si hay procesos para planificar en la cola de Listos
		if (queue_is_empty(colaListos) && (queue_is_empty(colaFinalizar))) {
		}else{
			//Saco el primer elemento de la cola, porque ya lo planifique.
			pthread_mutex_lock(&cListos);
			t_procesoEntrenador* proceso = (t_procesoEntrenador*) queue_pop(colaListos);
			pthread_mutex_unlock(&cListos);

			log_info(logMapa, "Se libera de la cola de listos el proceso del Entrenador: '%s'", proceso->nombre);
			free(proceso);

			t_MensajeMapa_Entrenador* contextoProceso = malloc(sizeof(t_MensajeMapa_Entrenador));
			contextoProceso->quantum = configMapa.quantum;
			//Enviar contextoProceso al Entrenador

			///Probar
			imprimirColaListos();
			free(contextoProceso);
		}
	}

}

void detectarDeadLock() {
	while (1){
		usleep(configMapa.tiempoChequeoDeadlock * 1000);
		int p = 0;
		int cantPokenests = list_size(pokeNests);
		while (p <= cantPokenests) {
			switch(queue_size(colasBloqueados[p])){
			case 0:{
				p++;
				break;
			}
			case 1:{//entra cuando hay 1 entrenador en la cola # p de bloqueados
				t_entrenadorBloqueado *proceso;
				pthread_mutex_lock(&cBloqueados);
				proceso = (t_entrenadorBloqueado*) queue_pop(colasBloqueados[p]);
				pthread_mutex_unlock(&cBloqueados);
				time_t final;
				final = time( NULL );
				log_info(logMapa,"Número de segundos transcurridos desde que %s estuvo bloqueado: %f s\n",proceso->nombre,
								difftime(final, proceso->tiempoBloqueado));

				//Cambio el proceso a estado ready y agrego a la cola de listos
				log_info(logMapa,
						"Saco el procesoEntrenador %d de cola Bloqueados %d y lo transfiero a cola de Listos ",
						proceso->nombre, p);
				int estado = 1;
				cambiarEstadoProceso(proceso->entrenadorID, estado);
				int pos = buscarProceso(proceso->entrenadorID);
				pthread_mutex_lock(&listadoProcesos);
				t_procesoEntrenador* infoProceso = (t_procesoEntrenador*) list_get(listaProcesos, pos);
				pthread_mutex_unlock(&listadoProcesos);

				t_datosEntrenador* entrenador = searchEntrenador(infoProceso->id);
				if (entrenador->objetivoActual->quantity > 0) {
					restarRecurso(items, entrenador->objetivoActual->id);
					// informar al entrenador de objetivo cumplido y recibir otro nuevo objetivo
					int resolucionCaptura = 0;
					enviar(&socketEntrenadorActivo, &resolucionCaptura, sizeof(int));//hasta aca llega bien
					//actualizo objetivo todo
					char idNuevoObjetivo;
					recibir(&socketEntrenadorActivo, &idNuevoObjetivo, sizeof(char));
				}else{//no hay pokemones disponibles
					int resolucionCaptura = -1;
					enviar(&socketEntrenadorActivo, &resolucionCaptura, sizeof(int));//hasta aca llega bien
				}

				pthread_mutex_lock(&cListos);
				queue_push(colaListos, (void*) infoProceso);
				pthread_mutex_unlock(&cListos);
				/*if (!queue_is_empty(colasBloqueados[p])) {
					proceso = (t_entrenadorBloqueado*) list_get(colasBloqueados[p]->elements, queue_size(colasBloqueados[p]) - 1);

				}*/
				imprimirColasBloqueados();

				break;
			}
			case 2:{

				imprimirColasBloqueados();

				break;
			}
			default:{//todo evaluar si puede haber mas de 2 entrenadores en 1 pokenest

				break;
			}
			}//Fin del switch
		}//Fin del while
	}//Fin del while (1)


}


void cambiarEstadoProceso(char entrenadorID, int estado) {
	int cambiar = buscarProceso(entrenadorID);
	if (cambiar != -1) {
		t_procesoEntrenador* procesoEntrenador;
		pthread_mutex_lock(&listadoProcesos);
		procesoEntrenador = (t_procesoEntrenador*) list_get(listaProcesos, cambiar);
		pthread_mutex_unlock(&listadoProcesos);
		procesoEntrenador->estado = estado;
	} else {
		log_error(logMapa,"Error al cambiar estado de proceso, proceso no encontrado en la lista.");
	}
}

void actualizarPC(char entrenadorID, int programCounter) {
	int cambiar = buscarProceso(entrenadorID);
	if (cambiar != -1) {
		t_procesoEntrenador* procesoEntrenador;
		pthread_mutex_lock(&listadoProcesos);
		procesoEntrenador = (t_procesoEntrenador*) list_get(listaProcesos, cambiar);
		pthread_mutex_unlock(&listadoProcesos);
		procesoEntrenador->programCounter = programCounter;
	} else {
		log_error(logMapa,"Error al cambiar el PC del proceso, proceso no encontrado en la lista.");
	}
}

void imprimirListaEntrenador() {

	pthread_mutex_lock(&listadoEntrenador);
	t_list* aux = listaEntrenador;
	pthread_mutex_unlock(&listadoEntrenador);

	log_info(logMapa,"lista de entrenadores: ");
	int i = 0;
	int cantEntrenadores = list_size(aux);
	while (i < cantEntrenadores) {
		t_datosEntrenador* datosEntrenador;
		datosEntrenador = (t_datosEntrenador*) list_get(aux, i);

		log_info(logMapa,"Entrenador %d: '%c'\n", i, datosEntrenador->id);

		i++;
	}
	if (i == 0) log_warning(logMapa,"La lista está vacía");

	if (aux == NULL) log_info(logMapa,"Se llego al final de la lista");
}

void imprimirListaPokeNests() {
	pthread_mutex_lock(&listadoPokeNests);
	t_list* aux = pokeNests;
	pthread_mutex_unlock(&listadoPokeNests);

	log_info(logMapa,"lista de PokeNests: \n");
	int i = 0;
	int cantPokeNests = list_size(aux);
	while (i < cantPokeNests) {
		ITEM_NIVEL* datosPokeNest;
		datosPokeNest = (ITEM_NIVEL*) list_get(aux, i);

		log_info(logMapa,"PokeNest %d: '%c'\n", i, datosPokeNest->id);

		i++;
	}
	if (i == 0) log_warning(logMapa, "La lista está vacía");

	if (aux == NULL) log_info(logMapa, "Se llego al final de la lista");
}


void imprimirListaItems() {
	pthread_mutex_lock(&listadoItems);
	t_list* aux = items;
	pthread_mutex_unlock(&listadoItems);

	log_info(logMapa,"lista de Items: \n");
	int i = 0;
	int cantPokeNests = list_size(aux);
	while (i < cantPokeNests) {
		ITEM_NIVEL* datosPokeNest;
		datosPokeNest = (ITEM_NIVEL*) list_get(aux, i);

		log_info(logMapa,"PokeNest %d: '%c'\n", i, datosPokeNest->id);

		i++;
	}
	if (i == 0) log_warning(logMapa,"La lista está vacía");

	if (aux == NULL) log_info(logMapa,"Se llego al final de la lista");
}


void imprimirColaListos() {
	pthread_mutex_lock(&cListos);
	t_queue* aux = colaListos;
	pthread_mutex_unlock(&cListos);

	int i = 0;
	int cantEntrenadores = queue_size(aux);
	if (cantEntrenadores == 0){
		log_info(logMapa, "No hay Entrenadores en Cola de Listos");
		return;
	}
	log_info(logMapa, "Cola de Listos: ");
	while (i < cantEntrenadores) {
		t_procesoEntrenador* datosEntrenador;
		datosEntrenador = (t_procesoEntrenador*) list_get(aux->elements, i);

		log_info(logMapa, "Entrenador %d: '%s'. ID: %c\n", i, datosEntrenador->nombre, datosEntrenador->id);

		i++;
	}

	if (aux == NULL) log_info(logMapa, "-----------------");
}

void imprimirColasBloqueados() {
	pthread_mutex_lock(&cBloqueados);
	t_queue** aux = colasBloqueados;
	pthread_mutex_unlock(&cBloqueados);

	log_info(logMapa, "Cola de Bloqueados: ");
	int i = 0;
	int cantPokeNests = list_size(pokeNests);
	while (i < cantPokeNests) {
		if(queue_is_empty(aux[i])){
			break;
		}
		t_entrenadorBloqueado* procesoEntrenador = (t_entrenadorBloqueado*) list_get(aux[i]->elements,i);

		log_info(logMapa, "Entrenador %d: '%d'\n", i, procesoEntrenador->nombre);

		i++;
	}
	if (i == 0) log_info(logMapa, "No hay Entrenadores en Cola de Bloqueados");

	if (aux == NULL) log_info(logMapa, "-----------------");
}



void crearListas() {
	//Creo la lista de Entrenadores
	listaEntrenador = list_create();
	//Creo la lista de items en interfaz
	items = list_create();
	//Creo Lista Procesos
	listaProcesos = list_create();
	//Creo la Cola de Listos
	colaListos = queue_create();
	//Creo cola de Procesos a Finalizar.
	colaFinalizar = queue_create();
}

void *initialize(int tamanio){
	int i;
	void * retorno = malloc(tamanio);
	for(i=0;i<tamanio;i++){
		((char*)retorno)[i]=0;
	}
	return retorno;
}

void inicializarMutex() {
	pthread_mutex_init(&listadoEntrenador, NULL);
	pthread_mutex_init(&listadoProcesos, NULL);
	pthread_mutex_init(&cListos, NULL);
	pthread_mutex_init(&cBloqueados, NULL);
	pthread_mutex_init(&cFinalizar, NULL);
	pthread_mutex_init(&varGlobal, NULL);
	pthread_mutex_init(&procesoActivo, NULL);
	pthread_mutex_init(&listadoPokeNests, NULL);
	pthread_mutex_init(&listadoItems, NULL);


}

void sighandler1(int signum){
	log_info(logMapa,"Signal capturada %d por pedido de un Entrenador ", signum);
	//Flag activada para enviar vidas al Entrenador
	signalVidas = true;
}

void sighandler2(int signum){
	log_info(logMapa,"Signal capturada %d para leer un archivo de metadata del Pokedex ", signum);
	//Flag activada para leer archivo de metadata
	signalMetadata = true;
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

void getArchivosDeConfiguracion(){
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


/*
	//  Path: 	/Mapas/[nombre]/PokeNests/[PokeNest]/[PokeNest]NNN.dat
	//todo recibir numero de pokemon NNN (como string) y su tamanio
	char* pathPokemon = string_new();
	char* numeroRecibido = string_new();
	char* numero = string_new();
	int lenNumero = 0;
	memcpy(numero, numeroRecibido, lenNumero);
	numero = "001";//solo para probar
	pathPokemon = string_from_format("%s/%s%s.dat\0", pathPokeNest, nombrePokeNest,numero);
	getMetadataPokemon(pathPokemon);
	free(pathPokemon);
	free(numeroRecibido);
	free(pathPokeNest);

	free(pathMapa);*/

	//Crear Lista PokeNests
	pthread_mutex_lock(&listadoPokeNests);
	pokeNests = filtrarPokeNests();
	int cantPokeNests = list_size(pokeNests);
	pthread_mutex_unlock(&listadoPokeNests);

	//Creo colas de Procesos Bloqueados
	colasBloqueados = initialize(cantPokeNests * sizeof(char*));
	int i=0;
	while (i < cantPokeNests) {
		//timers[i] = initialize(sizeof(timer_t));
		//imprimirTimer(timers, i, len);
		colasBloqueados[i] = initialize(sizeof(t_queue*));
		colasBloqueados[i] = queue_create();
		i++;
	}

	sem_post(&configOn);
	sem_post(&mutex);
}

void getMetadataMapa(char* pathMetadataMapa){
	//log_info(logMapa, "Metadata del mapa: %s ", pathMetadataMapa);
	t_config* configuration;

	configuration = config_create(pathMetadataMapa);

	configMapa.tiempoChequeoDeadlock = config_get_int_value(configuration,"TiempoChequeoDeadlock");
	configMapa.batalla = config_get_int_value(configuration,"Batalla");
	configMapa.algoritmo = config_get_string_value(configuration,"algoritmo");
	configMapa.quantum = config_get_int_value(configuration,"quantum");
	configMapa.retardo = config_get_int_value(configuration,"retardo");

	conexion.ip = config_get_string_value(configuration,"IP");
	conexion.puerto = config_get_int_value(configuration,"Puerto");

}

t_pokeNest getMetadataPokeNest(char *pathMetadataPokeNest){
	//log_info(logMapa, "metadata de la Pokenest: %s ", pathMetadataPokeNest);
	t_config* configuration;
	t_pokeNest configPokenest;

	configuration = config_create(pathMetadataPokeNest);

	configPokenest.tipo = config_get_string_value(configuration, "Tipo");
	char* unaPos = config_get_string_value(configuration, "Posicion");

	char** posicionXY;
	posicionXY = string_split(unaPos, ";");
	configPokenest.posx = atoi(posicionXY[0]);
	configPokenest.posy = atoi(posicionXY[1]);

	char* identificador = config_get_string_value(configuration, "Identificador");
	memcpy(&configPokenest.id, identificador, sizeof(char));
	log_info(logMapa,
			"POKENEST - tipo: '%s', posx: '%d', posy: '%d', id: '%c' ",
			configPokenest.tipo, configPokenest.posx, configPokenest.posy,configPokenest.id);
	return configPokenest;

}

void getMetadataPokemon(char* pathPokemon){
	log_info (logMapa, "metadata del mapa: %s ", pathPokemon);
	t_config* configuration;

	configuration = config_create(pathPokemon);

	configPokemon.level = config_get_int_value(configuration,"Nivel");
	/*char* ascii = string_new();
	ascii = config_get_string_value(configuration,""); //todo probar esto [Ascii Art]
	string_split(ascii,"[");
	string_split(ascii,"]");
	memcpy(ascii, &configPokemon.ascii, strlen(ascii));*/
	//log_info(logMapa, "POKEMON - nivel: %s, ascii: %c ", configPokemon.nivel, configPokemon.ascii);

}


/*
 * @NAME: rnd
 * @DESC: Modifica el numero en +1,0,-1, sin pasarse del maximo dado
 */
void rnd(int *x, int max){
	*x += (rand() % 3) - 1;
	*x = (*x<max) ? *x : max-1;
	*x = (*x>0) ? *x : 1;
}
/*
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
				//todo informar al entrenador de objetivo cumplido y recibir otro nuevo objetivo
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
				batallar();
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
void batallar(){
	//Se crea una instancia de la Factory para crear pokemons con solo el nombre y el nivel
	t_pkmn_factory* pokemon_factory = create_pkmn_factory();

	//Factory, Nombre empieza con letra mayúscula (sin errores) y Nivel
	t_pokemon * pikachu = create_pokemon(pokemon_factory, "Pikachu", 30);
	t_pokemon * rhyhorn = create_pokemon(pokemon_factory, "Rhyhorn", 6);

	//Si el nombre del Pokémon no existe en los primeros 151 o está mal escrito
	//Retornará el puntero NULL (0x0)
	char* nombrePokemon = "MissingNo";
	t_pokemon * missigno = create_pokemon(pokemon_factory, nombrePokemon , 128);
	if (missigno == NULL) printf("El Pokémon %s no existe! El puntero de retorno de la factory fue: %p\n", nombrePokemon, missigno);

	//solo para visualizar el resultado -------------
	void destruirItem(ITEM_NIVEL* item) {
		free(item);
	}
	list_destroy_and_destroy_elements(items, (void*) destruirItem);
	nivel_gui_terminar();
	//solo para visualizar el resultado --------------

	printf("========Batalla!========\n");
	printf("Primer Pokemon: %s[%s/%s] Nivel: %d\n",
			pikachu->species, pkmn_type_to_string(pikachu->type),
			pkmn_type_to_string(pikachu->second_type), pikachu->level);
	printf("Segundo Pokemon: %s[%s/%s] Nivel: %d\n",
			rhyhorn->species, pkmn_type_to_string(rhyhorn->type),
			pkmn_type_to_string(rhyhorn->second_type), rhyhorn->level); //Función que sirve para ver el Tipo de Enum como un String

	//La batalla propiamente dicha
	t_pokemon * loser = pkmn_battle(pikachu, rhyhorn);

	printf("El Perdedor es: %s\n", loser->species);//species es el nombre del pokemon

	//Liberemos los recursos
	//Como el puntero loser apunta a alguno de los otros 2, no se lo libera
	free(pikachu);
	free(rhyhorn);
	//Hay que destruir la instancia de la Factory
	destroy_pkmn_factory(pokemon_factory);
}

t_datosEntrenador* searchEntrenador(char id){
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

void agregarEntrenador(char id, char* nombreEntrenador, int socketEntrenador, char objetivoID){
	imprimirListaPokeNests();
	ITEM_NIVEL* item = _search_item_by_id(items, objetivoID);//todo items en vez de pokeNests
	/*imprimirListaPokeNests();
	pthread_mutex_lock(&listadoItems);
	ITEM_NIVEL* item = searchItem(objetivoID);
	pthread_mutex_unlock(&listadoItems);*/

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

void quitGui(){
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

	objetivo = _search_item_by_id(items,entrenador->objetivoActual->id);

	int distanciaX = entrenador->posx - objetivo->posx;
	int distanciaY = entrenador->posy - objetivo->posy;
	int distanciaTotal = abs(distanciaX) + abs(distanciaY);

	return distanciaTotal;
}

bool estaMasCerca(t_datosEntrenador* entrenador1, t_datosEntrenador* entrenador2){
	int distanciaEntrenador1 = distanciaAObjetivo(entrenador1);
	int distanciaEntrenador2 = distanciaAObjetivo(entrenador2);

	if (distanciaEntrenador1 < distanciaEntrenador2){
		return true;
	}
	return false;
}
/*
bool esEntrenador(ITEM_NIVEL* entrenador){
	return entrenador->item_type == PERSONAJE_ITEM_TYPE;
}
*/

t_list* filtrarPokeNests(){
	bool esPokeNest(ITEM_NIVEL* pokeNest){
		return pokeNest->item_type == RECURSO_ITEM_TYPE;
	}
	pthread_mutex_lock(&listadoItems);
	t_list* pokeNests = list_filter(items, (void*) esPokeNest);
	pthread_mutex_unlock(&listadoItems);

	return pokeNests;

}

int entrenadorMasCercano() {
	pthread_mutex_lock(&cListos);
	t_procesoEntrenador* procesoEntrenadorCercano = (t_procesoEntrenador*) queue_peek(colaListos);
	pthread_mutex_unlock(&cListos);
	int i = 0;
	int cantEntrenadores = queue_size(colaListos);
	t_datosEntrenador* entr = searchEntrenador(procesoEntrenadorCercano->id);
	while (i < cantEntrenadores) {
		i++;
		if (i == cantEntrenadores) return entr->numSocket;
		pthread_mutex_lock(&listadoEntrenador);
		t_procesoEntrenador* procesoOtroEntrenador = (t_procesoEntrenador*) list_get(listaEntrenador, i);
		pthread_mutex_unlock(&listadoEntrenador);

		t_datosEntrenador* entrenadorMasCercano = searchEntrenador(procesoEntrenadorCercano->id);
		t_datosEntrenador* otroEntrenador= searchEntrenador(procesoOtroEntrenador->id);
		bool flag = estaMasCerca(entrenadorMasCercano, otroEntrenador);

		if (flag == false) {
			procesoEntrenadorCercano = procesoOtroEntrenador;
			entr = searchEntrenador(procesoEntrenadorCercano->id);
		}
	}
	return entr->numSocket;
}

void avanzarPosicion(int* actualX, int* actualY, int destinoX, int destinoY){
	int posicionX = *actualX;
	int posicionY = *actualY;

	if (!alternateFlag) { //si esta en false entra
		if (posicionX > destinoX) {
			posicionX--;
		} else if (posicionX < destinoX) {
			posicionX++;
		}
		if (posicionY != destinoY) alternateFlag = true;

	} else if (alternateFlag) { //si esta en true entra
		if (posicionY > destinoY) {
			posicionY--;
		} else if (posicionY < destinoY) {
			posicionY++;
		}
		if (posicionX != destinoX) alternateFlag = false;
	}
	*actualX = posicionX;
	*actualY = posicionY;
}

void enviarPosPokeNest(t_datosEntrenador* entrenador, int socketEntrenador){
	ITEM_NIVEL* item = _search_item_by_id(items, entrenador->objetivoActual->id);
	entrenador->objetivoActual = item;

	if (item != NULL) {
		int x = entrenador->objetivoActual->posx;
		int y = entrenador->objetivoActual->posy;
		enviar(&socketEntrenador, &x, sizeof(int));
		enviar(&socketEntrenador, &y, sizeof(int));
	}

}

void enviarMensajeTurnoConcedido(){
	char turnoConcedido[] = "turno concedido";
	int turnoLen = -1;
	turnoLen = strlen(turnoConcedido) + 1 ; // +1(solo en arrays) es porque strlen no cuenta el \0

	// Envia el tamanio del texto Entrenador
	enviar(&socketEntrenadorActivo, &turnoLen, sizeof(turnoLen));
	enviar(&socketEntrenadorActivo, turnoConcedido, turnoLen);

}

void notificarFinDeObjetivos(char* pathMapa, int socketEntrenador){

	//  Path:   /Mapas/[nombre]/medalla-[nombre].jpg
	char* pathMetadataMedalla = string_new();
	pathMetadataMedalla = string_from_format("%s/medalla-%s.jpg\0", pathMapa, configMapa.nombre);
	int pathLen = -1;
	pathLen = strlen(pathMetadataMedalla) + 1 ;
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
		if (datosEntrenador->nombre == nombre) {
			return datosEntrenador->numSocket;
		}
	}
	return -1;
}

void recibirInfoInicialEntrenador(int socketEntrenador){
	t_MensajeEntrenador_Mapa mensaje;

	//Recibo mensaje usando su tamanio
	char* mensajeRcv = malloc(sizeof(t_MensajeEntrenador_Mapa));
	memset(mensajeRcv, '\0', sizeof(t_MensajeEntrenador_Mapa));
	recibir(&socketEntrenador, mensajeRcv, sizeof(t_MensajeEntrenador_Mapa));

	deserializarMapa_Entrenador(&mensaje, mensajeRcv);
	char entrenadorID = mensaje.id;
	agregarEntrenador(entrenadorID, mensaje.nombreEntrenador, socketEntrenador, mensaje.objetivoActual);
	CrearPersonaje(items, entrenadorID, 1, 1);

	procesarEntrenador(entrenadorID,mensaje.nombreEntrenador);
	//log_info(logMapa, "se creo el estado del proceso para el entrenador: %s",mensaje.nombreEntrenador);

	//imprimirListaEntrenador();
	free(mensajeRcv);
}

void procesarDirectorios(char* pathMapa) {
	//  Path:	/Mapas/[nombre]/PokeNests/[nombre-de-PokeNest]/metadata

	char* pathPokeNest = string_new();
	char* pathPokeNests = string_new();
	int bytes = 0;
	int cantPokemones = 0 ;
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
		}
		bytes = bytes + estru.st_size;
	}
	closedir(dir);
}

int cantidadDePokemones(char* pathPokeNest) {
	//  Path:	/Mapas/[nombre]/PokeNests/[nombre-de-PokeNest]
	int bytes = 0, cantPokemones = 0;
	struct stat estru;
	DIR* dir;
	struct dirent* directorio = NULL;
	dir = opendir(pathPokeNest);
	while ((directorio = readdir(dir)) != NULL) {
		char* name = directorio->d_name;
		stat(name, &estru);
		bool esPokemon = false;
		esPokemon = strcmp(name, ".") == 1  && strcmp(name, "..") == 1;
		if (esPokemon) {
			cantPokemones++;
		}
		bytes = bytes + estru.st_size;
	}
	cantPokemones = cantPokemones - 1;//resto 1 por el archivo metadata
	closedir(dir);
	return cantPokemones;
}

