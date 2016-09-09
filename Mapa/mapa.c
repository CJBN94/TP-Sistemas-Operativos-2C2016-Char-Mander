/*
 * mapa.c
 *
 */

#include "mapa.h"

int main(int argc, char **argv) {
	char* logFile = "/home/utnso/git/tp-2016-2c-SegmentationFault/Mapa/logMapa";

	//pthread_t planificadorRRThread;

	assert(("ERROR - No se pasaron argumentos", argc > 1)); // Verifica que se haya pasado al menos 1 parametro, sino falla

	//Parametros
	int i;
	for (i = 0; i < argc; i++) {
		if (i == 0) {
			configMapa.nombre = argv[i + 1];
			printf("Nombre Mapa: '%s'\n", configMapa.nombre);
		}
		if (i == 1) {
			configMapa.pathPokedex = argv[i + 1];
			printf("Path Pokedex: '%s'\n", configMapa.pathPokedex);
		}
	}

	assert(("ERROR - No se paso el nombre del mapa como argumento", configMapa.nombre != NULL));
	assert(("ERROR - No se paso el path del Pokedex como argumento", configMapa.pathPokedex != NULL));

	//Creo el archivo de Log
	logMapa = log_create(logFile, "MAPA", 0, LOG_LEVEL_TRACE);

	//Inicializacion de listas y mutex
	crearListas();
	inicializarMutex();

	//Obtengo informacion de archivos e inicializo mapa
	getArchivosDeConfiguracion();

/*	if (strcmp(configMapa.algoritmo, "RR") == 0){
		pthread_create(&planificadorRRThread, NULL, (void*) planificarProcesoRR, NULL);
	} else {
		//pthread_create(&planificadorRRThread, NULL, (void*) planificarProcesoSRDF, NULL);
	}
*/
	ejemploProgramaGui();

/*	if (strcmp(configMapa.algoritmo, "RR") == 0){
		pthread_join(planificadorRRThread, NULL);
	} else {
		//pthread_join(planificarProcesoSRDF, NULL);
	}
*/

	return 1;

}


void procesarEntrenador(char* nombreEntrenador, int socketEntrenador) {
	t_proceso* procesoEntrenador = malloc(sizeof(t_proceso));
	log_info(logMapa, "creo el estadoEntrenador del proceso");

	procesoEntrenador->nombre = nombreEntrenador;
	procesoEntrenador->programCounter = 0;
	procesoEntrenador->estado = LISTO;
	procesoEntrenador->finalizar = 0;

	pthread_mutex_lock(&listadoProcesos);
	list_add(listaProcesos, (void*) procesoEntrenador);
	pthread_mutex_unlock(&listadoProcesos);

	//Agrego a la Cola de Listos
	pthread_mutex_lock(&cListos);
	queue_push(colaListos, (void*) procesoEntrenador);
	pthread_mutex_unlock(&cListos);
	log_info(logMapa, "size colaListos: %d", queue_size(colaListos));

}

void atenderFinDeQuantum(int socketEntrenador,char* nombre){
	int pos = buscarEntrenador(socketEntrenador);
	pthread_mutex_lock(&listadoEntrenador);
	t_datosEntrenador* datosEntrenador = (t_datosEntrenador*) list_remove(listaEntrenador, pos);
	pthread_mutex_unlock(&listadoEntrenador);

	free(datosEntrenador);

	//Cambio el PC del Proceso, le sumo el quantum al PC actual.
	t_proceso* infoProceso;
	int buscar = buscarProceso(nombre);
	pthread_mutex_lock(&listadoProcesos);
	infoProceso = (t_proceso*)list_get(listaProcesos,buscar);
	pthread_mutex_unlock(&listadoProcesos);

	int quantumUsado= 0;
	recibir(socketEntrenador, &quantumUsado, sizeof(int));

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

void planificarProcesoSRDF() {

}

void planificarProcesoRR() {
	while (1){
		//Veo si hay procesos para planificar en la cola de Listos
		if (queue_is_empty(colaListos) && (queue_is_empty(colaFinalizar))) {
			break;
		}else{

			//Saco el primer elemento de la cola, porque ya lo planifique.
			pthread_mutex_lock(&cListos);
			t_proceso* proceso = (t_proceso*) queue_pop(colaListos);
			pthread_mutex_unlock(&cListos);

			log_info(logMapa, "Se libera de la cola de listos el proceso de nombre: '%s'", proceso->nombre);
			free(proceso);

			t_MensajeMapa_Entrenador* contextoProceso = malloc(sizeof(t_MensajeMapa_Entrenador));
			contextoProceso->quantum = configMapa.quantum;
			//Enviar contextoProceso al Entrenador

			imprimirListaEntrenador();//Probar
			free(contextoProceso);
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

void imprimirListaEntrenador() {

	pthread_mutex_lock(&listadoEntrenador);
	t_list* aux = listaEntrenador;
	pthread_mutex_unlock(&listadoEntrenador);

	log_info(logMapa, "lista de entrenadores: ");
	int i = 0;
	while (aux != NULL) {
		t_datosEntrenador* datosEntrenador;
		datosEntrenador = (t_datosEntrenador*) list_get(aux, i);

		log_info(logMapa, "Entrenador %d: '%c'\n", i, datosEntrenador->id);

		i++;
	}
	if (i == 0) log_info(logMapa, "La lista está vacía");

	if (aux == NULL) log_info(logMapa, "Se llego al final de la lista");
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
	//Creo cola de Procesos Bloqueados.
	colaBloqueados = queue_create();
	//Creo cola de Procesos a Finalizar.
	colaFinalizar = queue_create();
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
		procesoEntrenador = (t_proceso*) list_get(listaProcesos, i);
		pthread_mutex_unlock(&listadoProcesos);
		if (procesoEntrenador->nombre == nombreEntrenador) {
			return i;
		}
	}
	return -1;
}

void getArchivosDeConfiguracion(){
	char* pathMapa = string_new();
	string_append(&pathMapa, configMapa.pathPokedex);
	string_append(&pathMapa, "/Mapas/");
	string_append(&pathMapa, configMapa.nombre);

	//  Path:   /Mapas/[nombre]/metadata
	char* pathMetadataMapa = string_new();
	pathMetadataMapa = string_from_format("%s/metadata\0", pathMapa);
	getMetadataMapa(pathMetadataMapa);
	free(pathMetadataMapa);

//TODO recorrer todos los directorios e ir creando las cajas

	//  Path:	/Mapas/[nombre]/PokeNests/[nombre-de-PokeNest]/metadata
	char* pathPokeNest = string_new();
	char* pathPokeNests = string_new();
	char* nombrePokeNest = string_new();
	int bytes = 0;
	struct stat estru;
	DIR* dir;
	log_info(logMapa, "id del nombre PokeNest: %c", nombrePokeNest[0]);

	pathPokeNests = string_from_format("%s/PokeNests\0", pathMapa);

	dir = opendir(pathPokeNests);
	struct dirent* directorio = NULL;
	while ((directorio = readdir(dir)) != NULL) {
		char* nombrePokeNest = directorio->d_name;
		pathPokeNest = string_from_format("%s/%s\0",pathPokeNests, nombrePokeNest);
		stat(directorio->d_name, &estru);
		if (strcmp(nombrePokeNest, ".") == 1 && strcmp(nombrePokeNest, "..") == 1) {
			char* pathMetadataPokeNest = string_new();
			string_append(&pathMetadataPokeNest, pathPokeNest);
			string_append(&pathMetadataPokeNest, "/metadata");
			getMetadataPokeNest(pathMetadataPokeNest);
			//printf("%s \n", name);
		}
		bytes = bytes + estru.st_size;
	}

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

}

void getMetadataMapa(char* pathMetadataMapa){
	log_info(logMapa, "Creando el archivo de configuracion de metadata del mapa: %s ", pathMetadataMapa);
	t_config* configuration;

	configuration = config_create(pathMetadataMapa);

	configMapa.tiempoChequeoDeadlock = config_get_int_value(configuration,"TiempoChequeoDeadlock");
	configMapa.batalla = config_get_int_value(configuration,"Batalla");
	configMapa.algoritmo = config_get_string_value(configuration,"algoritmo");
	configMapa.quantum = config_get_int_value(configuration,"quantum");
	int retardo = config_get_int_value(configuration,"retardo");
	configMapa.retardo =  retardo * 1000;//paso a microsegundos (1 miliseg = 1000 microseg)

	conexion.ip = config_get_string_value(configuration,"IP");
	conexion.puerto = config_get_int_value(configuration,"Puerto");

}

void getMetadataPokeNest(char *pathMetadataPokeNest){
	log_info(logMapa, "Creando el archivo de configuracion de metadata de la Pokenest: %s ", pathMetadataPokeNest);
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

	CrearCaja(items, configPokenest.id, configPokenest.posx, configPokenest.posy, 1);
}

void getMetadataPokemon(char* pathPokemon){
	log_info (logMapa, "Creando el archivo de metadata del mapa: %s ", pathPokemon);
	t_config* configuration;

	configuration = config_create(pathPokemon);

	configPokemon.nivel = config_get_int_value(configuration,"Nivel");
	/*char* ascii = string_new();
	ascii = config_get_string_value(configuration,""); //todo probar esto [Ascii Art]
	string_split(ascii,"[");
	string_split(ascii,"]");
	memcpy(ascii, &configPokemon.ascii, strlen(ascii));*/
	//log_info(logMapa, "POKEMON - nivel: %s, ascii: %c ", configPokemon.nivel, configPokemon.ascii);

}

void getPosicion(t_config* configuration) {


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

void ejemploProgramaGui() {
	pthread_t finalizarMapaThread;
	pthread_create(&finalizarMapaThread, NULL, (void*) quitGui, NULL);

	int rows = 19, cols = 78;
	int q, p;

	int x = 43, y = 15;

	int a = 20, b = 7;

	int ex1 = 10, ey1 = 14;
	int ex2 = 20, ey2 = 3;

	int cx1 = 50, cy1 = 15;
	int cx2 = 8, cy2 = 15;
	int cx3 = 45, cy3 = 2;

	nivel_gui_inicializar();
	nivel_gui_get_area_nivel(&rows, &cols);
	//Dimensiones normales = 78 x 19
	log_info(logMapa,"En X(cantCol):'%d'. En Y(cantFilas):'%d'",cols,rows);

	p = cols;
	q = rows;

	t_pokeNest* objetivo = malloc(sizeof(t_pokeNest));
	objetivo->id = 'H';
	objetivo->posx = cx1;
	objetivo->posy = cy1;
	agregarEntrenador('@', p, q, objetivo);
	//free(objetivo);
	CrearPersonaje(items, '@', p, q);

	t_pokeNest* objetivo2 = malloc(sizeof(t_pokeNest));
	objetivo2->id = 'B';
	ITEM_NIVEL* item = _search_item_by_id(items, objetivo2->id);
	objetivo2->posx = item->posx;
	objetivo2->posy = item->posx;
	agregarEntrenador('#', x, y, objetivo2);
	//free(objetivo2);
	CrearPersonaje(items, '#', x, y);

	t_pokeNest* objetivo3 = malloc(sizeof(t_pokeNest));
	objetivo3->id = 'F';
	objetivo3->posx = cx3;
	objetivo3->posy = cy3;
	agregarEntrenador('=', a, b, objetivo3);
	//free(objetivo3);
	CrearPersonaje(items, '=', a, b);
	//CrearPersonaje(items, '!', 20, 13);
	//CrearPersonaje(items, '?', 28, 1);

	CrearEnemigo(items, '1', ex1, ey1);
	CrearEnemigo(items, '2', ex2, ey2);

	CrearCaja(items, 'H', cx1, cy1, 5);
	//CrearCaja(items, 'M', cx2, cy2, 3);
	CrearCaja(items, 'F', cx3, cy3, 2);

	nivel_gui_dibujar(items, "Test Mapa SegmentationFault");

	t_datosEntrenador* entrenador = entrenadorMasCercano();
	log_info(logMapa,"entrenador mas cercano:'%c'", entrenador->id);

	t_pokeNest* pokeNest = buscarObjetivo(entrenador->id);

	while (1){
		avanzarPosicion(&entrenador->posx, &entrenador->posy, pokeNest->posx, pokeNest->posy);
		//t_datosEntrenador* datosEntrenador = searchEntrenador(entrenador->id);
		usleep(configMapa.retardo);

		rnd(&ex1, cols);
		rnd(&ey1, rows);
		rnd(&ex2, cols);
		rnd(&ey2, rows);
		MoverPersonaje(items, '1', ex1, ey1);
		MoverPersonaje(items, '2', ex2, ey2);

		MoverPersonaje(items, entrenador->id, entrenador->posx, entrenador->posy);
		//todo enviar el informe del uso de una unidad de tiempo

		int objx = entrenador->objetivoActual->posx;
		int objy = entrenador->objetivoActual->posy;

		ITEM_NIVEL* unObjetivo = _search_item_by_id(items,entrenador->objetivoActual->id);

		if ((entrenador->posx == objx) && (entrenador->posy == objy) && unObjetivo->quantity > 0) {//esto ya lo valida el entrenador
			pthread_mutex_lock(&cBloqueados);
			queue_push(colaBloqueados, (void*) entrenador);
			pthread_mutex_unlock(&cBloqueados);

			//todo verificar que no haya otros entrenadores en la pokenest
			restarRecurso(items, entrenador->objetivoActual->id);
			entrenador = entrenadorMasCercano();
			log_info(logMapa,"el nuevo entrenador mas cercano es:'%c'", entrenador->id);
			pokeNest = buscarObjetivo(entrenador->id);

			//informar al entrenador de objetivo cumplido y recibir otro nuevo objetivo
			//todo agregar algo para que no siga restando una vez que llega
		}
		if ((p == x) && (q == y)) {//todo
			BorrarItem(items, '#'); //si chocan, borramos uno (!)
		}

		nivel_gui_dibujar(items, "Test Mapa SegmentationFault");
	}

	BorrarItem(items, '#');
	BorrarItem(items, '@');

	BorrarItem(items, '1');
	BorrarItem(items, '2');

	BorrarItem(items, 'H');
	BorrarItem(items, 'M');
	BorrarItem(items, 'F');

	nivel_gui_terminar();

	pthread_join(finalizarMapaThread, NULL);

}

t_datosEntrenador* searchEntrenador(char id){
	bool search_by_id(t_datosEntrenador* entrenador) {
		return entrenador->id == id;
	}

	return list_find(listaEntrenador, (void*) search_by_id);
}

void agregarEntrenador(char id, int x, int y, t_pokeNest* objetivo){
	t_datosEntrenador* datosEntrenador = malloc(sizeof(t_datosEntrenador));
	datosEntrenador->objetivoActual = malloc(sizeof(t_pokeNest));
	datosEntrenador->id = id;
	memcpy(datosEntrenador->objetivoActual, objetivo, sizeof(t_pokeNest));
	datosEntrenador->posx = x;
	datosEntrenador->posy = y;

	pthread_mutex_lock(&listadoEntrenador);
	list_add(listaEntrenador, (void*) datosEntrenador);
	pthread_mutex_unlock(&listadoEntrenador);

}

void quitGui(){
	while (1) {

		int key;
		key = getch();

		if (key == 'Q' || key == 'q') {
			nivel_gui_terminar();
			exit(0);
		}
	}
}

t_pokeNest* buscarObjetivo(char id){
	t_pokeNest* objetivo = malloc(sizeof(t_pokeNest)) ;
	int cantEntrenadores, i;
	t_datosEntrenador* datosEntrenador;
	cantEntrenadores = list_size(listaEntrenador);
	for (i = 0; i < cantEntrenadores; i++) {
		pthread_mutex_lock(&listadoEntrenador);
		datosEntrenador = (t_datosEntrenador*) list_get(listaEntrenador, i);
		pthread_mutex_unlock(&listadoEntrenador);

		if (datosEntrenador->id == id) {
			objetivo->posx = datosEntrenador->objetivoActual->posx;
			objetivo->posy = datosEntrenador->objetivoActual->posy;

			return objetivo;
		}
	}
	return objetivo;
}

int distanciaAObjetivo(t_datosEntrenador* entrenador) {
	t_pokeNest* objetivo = buscarObjetivo(entrenador->id);
	int distanciaX = entrenador->posx - objetivo->posx;
	int distanciaY = entrenador->posy - objetivo->posy;
	int distanciaTotal = abs(distanciaX) + abs(distanciaY);

	free(objetivo);
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

t_list* filtrarPokeNests(){
	bool esPokeNest(ITEM_NIVEL* pokeNest){
		return pokeNest->item_type == RECURSO_ITEM_TYPE;
	}

	t_list* pokeNests = list_filter(items, (void*) esPokeNest);

	return pokeNests;

}*/

t_datosEntrenador* entrenadorMasCercano() {
	t_datosEntrenador* entrenadorMasCercano = (t_datosEntrenador*) list_get(listaEntrenador, 0);
	int i = 0;
	int cantEntrenadores= list_size(listaEntrenador);

	while (i < cantEntrenadores) {
		i++;
		if (i == cantEntrenadores) return entrenadorMasCercano;
		t_datosEntrenador* otroEntrenador = (t_datosEntrenador*) list_get(listaEntrenador, i);


		bool flag = estaMasCerca(entrenadorMasCercano, otroEntrenador);

		if (flag == false) {
			entrenadorMasCercano = otroEntrenador;
		}
	}
	return entrenadorMasCercano;
}

void avanzarPosicion(int* actualX, int* actualY, int destinoX, int destinoY){
	int posicionX = *actualX;
	int posicionY = *actualY;

	if (posicionY == destinoY || (!alternateFlag)){
		if (posicionX > destinoX) {
			posicionX--;
			alternateFlag = true;
		} else if (posicionX < destinoX) {
			posicionX++;
			alternateFlag = true;
		}
	} else if(posicionX == destinoX || alternateFlag) {
		if (posicionY > destinoY) {
			posicionY--;
			alternateFlag = false;
		} else if (posicionY < destinoY) {
			posicionY++;
			alternateFlag = false;
		}
	}
	*actualX = posicionX;
	*actualY = posicionY;
}

void enviarPosPokeNest(t_datosEntrenador* entrenador, int socketEntrenador){
	int nombreLen = -1;
	recibir(socketEntrenador, &nombreLen, sizeof(int));
	char* nombrePokeNest = malloc(nombreLen);
	recibir(socketEntrenador, nombrePokeNest, nombreLen);

	char id = nombrePokeNest[0];
	ITEM_NIVEL* item = _search_item_by_id(items, id);
	entrenador->objetivoActual = item;

	if (item != NULL) {
		enviar(socketEntrenador, &item->posx, sizeof(int));
		enviar(socketEntrenador, &item->posy, sizeof(int));
	}

}

void enviarMensajeTurnoConcedido(){
	char turnoConcedido[] = "turnoConedido";
	int turnoLen = -1;
	turnoLen = strlen(turnoConcedido) + 1 ;
	enviar(socketEntrenador, turnoConcedido, turnoLen);

}

void notificarFinDeObjetivos(char* pathMapa){

	//  Path:   /Mapas/[nombre]/medalla-[nombre].jpg
	char* pathMetadataMedalla = string_new();
	pathMetadataMedalla = string_from_format("%s/medalla-%s.jpg\0", pathMapa, configMapa.nombre);
	int pathLen = -1;
	pathLen = strlen(pathMetadataMedalla) + 1 ;
	enviar(socketEntrenador, pathMetadataMedalla, pathLen);
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


int procesarMensajeEntrenador(){
	log_info(logMapa, "Procesar mensaje Entrenador");

	int bytesRecibidos =-1;
	t_MensajeEntrenador_Mapa* mensaje = malloc(sizeof(t_MensajeMapa_Entrenador));

	int socketEntrenador = buscarSocketEntrenador(mensaje->nombreEntrenador);
	if (socketEntrenador==-1){
		log_error(logMapa,"No se encontro Entrenador %d ",mensaje->nombreEntrenador);
		return -1;
	}

	//Recibo mensaje usando su tamanio
	int mensajeSize = 0;
	bytesRecibidos = recibir(socketEntrenador, &mensajeSize, sizeof(int));

	char* mensajeRcv = malloc(sizeof(mensajeSize));
	bytesRecibidos = recibir(socketEntrenador, mensajeRcv, mensajeSize);

	//Deserializo mensajeRcv
	deserializarMapa_Entrenador(mensaje, mensajeRcv);

	int pos = buscarEntrenador(socketEntrenador);
	pthread_mutex_lock(&listadoEntrenador);
	t_datosEntrenador* entrenador = (t_datosEntrenador*) list_get(listaEntrenador, pos);
	pthread_mutex_unlock(&listadoEntrenador);


	switch (mensaje->operacion) {
	case 1:{
		enviarPosPokeNest(entrenador,socketEntrenador);
		break;
	}
	case 2:{
		MoverPersonaje(items, entrenador->id, entrenador->posx, entrenador->posy);
		//todo enviar el informe del uso de una unidad de tiempo

		break;
	}
	case 3:{
		if (entrenador->objetivoActual->quantity > 0) {
			int objetivoX = entrenador->objetivoActual->posx;
			int objetivoY = entrenador->objetivoActual->posy;

			bool estaEnPosObjetivo(ITEM_NIVEL* unEntrenador) {
				return (unEntrenador->posx == objetivoX
						&& unEntrenador->posy == objetivoY);
			}
			bool hayOtroEntrenador = list_any_satisfy(listaEntrenador, (void*) estaEnPosObjetivo);
			if (hayOtroEntrenador && configMapa.batalla == 0) {
				pthread_mutex_lock(&cBloqueados);
				queue_push(colaBloqueados, (void*) entrenador);
				pthread_mutex_unlock(&cBloqueados);
				//contar tiempo bloqueado

			} else if (!hayOtroEntrenador) {
				restarRecurso(items, entrenador->objetivoActual->id);
				//avisar al entrenador que cumplio el objetivo
			}
			//batallar();

		}

		break;
	}
	case 4:{
		//getMetadataMedalla();
		//enviar pathMedalla
		break;
	}
	case 5:{ 	//Fin de quantum
		log_info(logMapa, "Se procesa el fin del Quantum");
		atenderFinDeQuantum(socketEntrenador, mensaje->nombreEntrenador);
		break;
	}
	case 6:{
		//finalizaProceso(socketEntrenador, mensaje->nombreEntrenador);
		bytesRecibidos = -1;
		break;
	}
	case 7:{	//ejemplo de como recibir y enviar texto
		//Recibo el tamanio del texto
		int tamanio;
		recibir(socketEntrenador, &tamanio,sizeof(int));
		char* texto = malloc(tamanio);

		//Recibo el texto
		recibir(socketEntrenador, texto, tamanio);

		// Envia el tamanio del texto al Entrenador
		log_info(logMapa, "Tamanio: '%d'", tamanio);
		string_append(&texto,"\0");
		enviar(socketEntrenador, &tamanio, sizeof(int));

		// Envia el texto al proceso Entrenador
		log_info(logMapa, "Texto : '%s'", texto);
		enviar(socketEntrenador, texto, tamanio);

		free(texto);
		break;
	}
	case 10:{
		atenderFinDeQuantum(socketEntrenador, mensaje->nombreEntrenador);
		break;
	}
	default:
		log_error(logMapa, "Mensaje recibido invalido. ");
		//printf("Entrenador desconectado.");
	}

	free(mensajeRcv);
	free(mensaje);
	return bytesRecibidos;
}

void procesarDirectorios(char* pathMapa) {
	char* pathPokeNest = string_new();
	int bytes = 0;
	struct stat estru;
	DIR* dir;
	char* pathMetadataPokeNest = string_new();
	string_append(&pathMetadataPokeNest, pathPokeNest);
	string_append(&pathMetadataPokeNest, "/metadata");

	dir = opendir(pathPokeNest);
	struct dirent* directorio = NULL;
	while ((directorio = readdir(dir)) != NULL) {
		char* nombrePokeNest = directorio->d_name;
		pathPokeNest = string_from_format("%s/PokeNests/%s\0", pathMapa, nombrePokeNest);
		stat(directorio->d_name, &estru);
		if (strcmp(nombrePokeNest, ".") == 1 && strcmp(nombrePokeNest, "..") == 1) {
			getMetadataPokeNest(pathMetadataPokeNest);
			log_info(logMapa, "Nombre PokeNest: %c", nombrePokeNest);
			//printf("%s \n", name);
		}
		bytes = bytes + estru.st_size;
	}
}
