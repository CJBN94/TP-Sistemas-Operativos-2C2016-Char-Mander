/*
 * mapa.c
 *
 */

#include "mapa.h"

int main(int argc, char **argv) {
	t_mapa* datosMapa = malloc(sizeof(t_mapa));
	char *logFile = NULL;
	pthread_t planificadorRRThread;

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
	//Creo cola de Procesos a Finalizar.
	colaFinalizar = queue_create();

	//Inicializacion de los mutex
	inicializarMutex();

	pthread_create(&planificadorRRThread, NULL, (void*) planificarProcesoRR, NULL);


	ejemploProgramaGui();


	pthread_join(planificadorRRThread, NULL);

	return 0;

}

void atenderFinDeQuantum(int socketEntrenador,char* nombre){
	//Libero Entrenador
	if (alertFlag == false){
		liberarEntrenador(socketEntrenador);
	}else{
		int pos = buscarEntrenador(socketEntrenador);
		pthread_mutex_lock(&listadoEntrenador);
		t_datosEntrenador* datosEntrenador= list_remove(listaEntrenador,pos);
		pthread_mutex_unlock(&listadoEntrenador);

		free(datosEntrenador);
	}
	//Cambio el PC del Proceso, le sumo el quantum al PC actual.
	t_proceso* infoProceso;
	int buscar = buscarProceso(nombre);
	pthread_mutex_lock(&listadoProcesos);
	infoProceso = (t_proceso*)list_get(listaProcesos,buscar);
	pthread_mutex_unlock(&listadoProcesos);

	int quantumUsado= 0;
	//recibir(&socketEntrenador, &quantumUsado, sizeof(int));

	int pcnuevo = infoProceso->programCounter + quantumUsado;
	infoProceso->programCounter = pcnuevo;

	//Agrego el proceso a la Cola de Listos
	t_proceso* procesoEntrenador = malloc(sizeof(t_proceso));
	procesoEntrenador->nombre = nombre;
	procesoEntrenador->programCounter = pcnuevo;

	if (infoProceso->finalizar == 0){
		//Cambio el estado del proceso
		int estado = 1;
		cambiarEstadoProceso(nombre, estado);
		pthread_mutex_lock(&cListos);
		queue_push(colaListos, (void*) procesoEntrenador);
		pthread_mutex_unlock(&cListos);
	} else {
		pthread_mutex_lock(&cFinalizar);
		queue_push(colaFinalizar, (void*) procesoEntrenador);
		pthread_mutex_unlock(&cFinalizar);
	}
}

void planificarProcesoRR() {
	while (1){
		//Veo si hay procesos para planificar en la cola de Listos
		if (queue_is_empty(colaListos)) {
			break;
		}else{
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

		log_info(logMapa, "Se libera de la cola de listos el proceso de nombre: '%s'", proceso->nombre);
		free(proceso);

		imprimirListaEntrenador();
		}
	}

}

void cambiarEstadoProceso(char* nombreEntrenador, int estado) {
	int cambiar = buscarProceso(nombreEntrenador);
	if (cambiar != -1) {
		t_proceso* procesoEntrenador;
		pthread_mutex_lock(&listadoProcesos);
		procesoEntrenador = (t_proceso*) list_get(listaProcesos, cambiar);
		pthread_mutex_unlock(&listadoProcesos);
		procesoEntrenador->estado = estado;
	} else {
		log_error(logMapa,"Error al cambiar estado de proceso, proceso no encontrado en la lista.");
	}
}

void actualizarPC(char* nombreEntrenador, int programCounter) {
	int cambiar = buscarProceso(nombreEntrenador);
	if (cambiar != -1) {
		t_proceso* procesoEntrenador;
		pthread_mutex_lock(&listadoProcesos);
		procesoEntrenador = (t_proceso*) list_get(listaProcesos, cambiar);
		pthread_mutex_unlock(&listadoProcesos);
		procesoEntrenador->programCounter = programCounter;
	} else {
		log_error(logMapa,"Error al cambiar el PC del proceso, proceso no encontrado en la lista.");
	}
}

int buscarEntrenadorLibre() {
	int cantEntrenadores, i = 0;
	t_datosEntrenador* datosEntrenador;
	cantEntrenadores = list_size(listaEntrenador);
	log_info(logMapa, "size de listaEntrenador: %d",list_size(listaEntrenador));
	for (i = 0; i < cantEntrenadores; i++) {
		pthread_mutex_lock(&listadoEntrenador);
		datosEntrenador = (t_datosEntrenador*) list_get(listaEntrenador, i);
		pthread_mutex_unlock(&listadoEntrenador);

		if (datosEntrenador->estadoEntrenador== 0) {
			datosEntrenador->estadoEntrenador = 1;
			return datosEntrenador->numSocket;
		}
	}
	return -1;
}

void imprimirListaEntrenador() {

	pthread_mutex_lock(&listadoEntrenador);
	t_list* aux = listaEntrenador;
	pthread_mutex_unlock(&listadoEntrenador);

	log_info(logMapa, "lista de entrenadores: ");
	int i = 0;
	while (aux != NULL) {
		t_datosEntrenador* datosEntrenador;
		datosEntrenador = (t_datosEntrenador*) list_get(aux, i);

		log_info(logMapa, "Entrenador %d: '%s'\n", i, datosEntrenador->nombre);

		i++;
	}
	if (i == 0) log_info(logMapa, "La lista está vacía");

	if (aux == NULL) log_info(logMapa, "Se llego al final de la lista");
}

void inicializarMutex() {
	pthread_mutex_init(&listadoEntrenador, NULL);
	pthread_mutex_init(&listadoProcesos, NULL);
	pthread_mutex_init(&cListos, NULL);
	pthread_mutex_init(&cBloqueados, NULL);
	pthread_mutex_init(&cFinalizar, NULL);
	pthread_mutex_init(&varGlobal, NULL);
	pthread_mutex_init(&procesoActivo, NULL);
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

void liberarEntrenador(int socket) {
	int liberar = buscarEntrenador(socket);
	if (liberar != -1) {
		t_datosEntrenador* datosEntrenador;
		pthread_mutex_lock(&listadoEntrenador);
		datosEntrenador = (t_datosEntrenador*) list_get(listaEntrenador, liberar);
		pthread_mutex_unlock(&listadoEntrenador);
		datosEntrenador->estadoEntrenador= 0;
	} else {
		log_error(logMapa, "Error al liberar ENTRENADOR de socket: %d no encontrado en la lista.", socket);
	}
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

int buscarProceso(char* nombreEntrenador) {
	t_proceso* procesoEntrenador;
	int i = 0;
	int cantProcesos = list_size(listaProcesos);
	for (i = 0; i < cantProcesos; i++) {
		pthread_mutex_lock(&listadoProcesos);
		procesoEntrenador = list_get(listaProcesos, i);
		pthread_mutex_unlock(&listadoProcesos);
		if (procesoEntrenador->nombre == nombreEntrenador) {
			return i;
		}
	}
	return -1;
}

void rnd(int *x, int max){
	*x += (rand() % 3) - 1;
	*x = (*x<max) ? *x : max-1;
	*x = (*x>0) ? *x : 1;
}

void ejemploProgramaGui() {

	t_list* items = list_create();

	int rows, cols;
	int q, p;

	int x = 1;
	int y = 1;

	int ex1 = 10, ey1 = 14;
	int ex2 = 20, ey2 = 3;

	nivel_gui_inicializar();

	nivel_gui_get_area_nivel(&rows, &cols);

	p = cols;
	q = rows;

	CrearPersonaje(items, '@', p, q);
	CrearPersonaje(items, '#', x, y);

	CrearEnemigo(items, '1', ex1, ey1);
	CrearEnemigo(items, '2', ex2, ey2);

	CrearCaja(items, 'H', 26, 10, 5);
	CrearCaja(items, 'M', 8, 15, 3);
	CrearCaja(items, 'F', 19, 9, 2);

	nivel_gui_dibujar(items, "Test Mapa SegmentationFault");

	while ( 1 ) {
		int key ;
		//key= getch();//TODO probar si compila con getch() para usar las flechas, sino probar con las teclas: a s d w

		switch( key ) {

		case KEY_UP:
			if (y > 1) {
				y--;
			}
			break;

		case KEY_DOWN:
			if (y < rows) {
				y++;
			}
			break;

		case KEY_LEFT:
			if (x > 1) {
				x--;
			}
			break;
		case KEY_RIGHT:
			if (x < cols) {
				x++;
			}
			break;
		case 'w':
		case 'W':
		if (q > 1) {
			q--;
		}
		break;

		case 's':
		case 'S':
			if (q < rows) {
				q++;
			}
			break;

		case 'a':
		case 'A':
			if (p > 1) {
				p--;
			}
			break;
		case 'D':
		case 'd':
			if (p < cols) {
				p++;
			}
			break;
		case 'Q':
		case 'q':
			nivel_gui_terminar();
			exit(0);
			break;
		}


		rnd(&ex1, cols);
		rnd(&ey1, rows);
		rnd(&ex2, cols);
		rnd(&ey2, rows);
		MoverPersonaje(items, '1', ex1, ey1 );
		MoverPersonaje(items, '2', ex2, ey2 );

		MoverPersonaje(items, '@', p, q);
		MoverPersonaje(items, '#', x, y);

		if (   ((p == 26) && (q == 10)) || ((x == 26) && (y == 10)) ) {
			restarRecurso(items, 'H');
		}

		if (   ((p == 19) && (q == 9)) || ((x == 19) && (y == 9)) ) {
			restarRecurso(items, 'F');
		}

		if (   ((p == 8) && (q == 15)) || ((x == 8) && (y == 15)) ) {
			restarRecurso(items, 'M');
		}

		if((p == x) && (q == y)) {
			BorrarItem(items, '#'); //si chocan, borramos uno (!)
		}

		nivel_gui_dibujar(items, "Test Chamber 04");
	}

	BorrarItem(items, '#');
	BorrarItem(items, '@');

	BorrarItem(items, '1');
	BorrarItem(items, '2');

	BorrarItem(items, 'H');
	BorrarItem(items, 'M');
	BorrarItem(items, 'F');

	nivel_gui_terminar();

}

