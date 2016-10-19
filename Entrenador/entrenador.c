/*
 * entrenador.c
 *
 */

#include "entrenador.h"

int main(int argc, char **argv) {
	system("clear");
	time_t comienzo;
	comienzo = time(NULL);
	seniales();//todo ponerlo adentro del while(esMiTurno)

	//assert(("ERROR - No se pasaron argumentos", argc > 1)); // Verifica que se haya pasado al menos 1 parametro, sino falla

	//Parametros
	int i;
	for (i = 0; i < argc; i++) {
		if (i == 0) {
			entrenador.nombre = argv[i + 1];
			printf("Nombre Entrenador: '%s'\n", entrenador.nombre);
		}
		if (i == 1) {
			entrenador.rutaPokedex = argv[i + 1];
			printf("Ruta Pokedex: '%s'\n", entrenador.rutaPokedex);
		}
	}

	//assert(("ERROR - No se paso el nombre del entrenador como argumento", entrenador.nombre != NULL));
	//assert(("ERROR - No se paso la ruta del pokedex como argumento", entrenador.rutaPokedex != NULL));

	entrenador.rutaPokedex = "/home/utnso/Pokedex/";
	//Creo el archivo de Log
	logEntrenador = log_create("logEntrenador", "ENTRENADOR", 0, LOG_LEVEL_TRACE);
	//pthread_t rcvThread;
	//pthread_create(&rcvThread, NULL, (void*) recibirTurnoConcedido,NULL);

	entrenador.hojaDeViaje = list_create();

	//Levanto los datos del metadata de Entrenador
	getMetadataEntrenador();

	crearListaPokemones();
	interactuarConMapas();

	//Se crea el thread para interactuar con señales
	/*
	pthread_t seniales ;
	pthread_create(&seniales, NULL, (void*)manejoDeSeniales, NULL );
	*/
	time_t final;
	final = time( NULL);
	double timer = difftime(final, comienzo);
	printf("Tiempo Total que tomó toda la aventura: %.2f s\n", timer);
	printf("Cantidad de DeadLocks involucrado: %d\n", cantDeadLocks);
	//printf("Tiempo que estuvo bloqueado en las PokeNests: %d\n", tiempoBloqueadoEnPokeNests);//todo tiempoBloqueado cantMuertes
	//printf("Cantidad de muertes: %d\n", cantMuertes);

	//getchar();

	//Se crea el thread para interactuar con el mapa

	//pthread_t interaccionConMapa;
	/*
	pthread_join(seniales, NULL);
	//pthread_join(interaccionConMapa, NULL);

	*/
/*
	//Se crea el thread para recibir el pedido del mapa de tu pokemon mas fuerte
	pthread_t *recibirPedidoMapa = malloc(sizeof(pthread_t));
	pthread_create(recibirPedidoDeMapa, NULL, (void*)enviarPokemonMasFuerte, )


	//chequearObjetivos(entrenador,'M');

	//CONFIGURACION DEL ENTRENADOR


	//faltan los objetivos
*/
	return EXIT_SUCCESS;
}

void crearListaPokemones(){
	int cantMapas = list_size(entrenador.hojaDeViaje);
	pokemonesCapturados = inicializar(cantMapas * sizeof(char*));
	int m;
	for (m = 0; m < cantMapas; m++) {
		pokemonesCapturados[m] = inicializar(sizeof(t_list*));
		pokemonesCapturados[m] = list_create();
	}
}

void *inicializar(int tamanio) {
	int i;
	void * retorno = malloc(tamanio);
	for (i = 0; i < tamanio; i++) {
		((char*) retorno)[i] = 0;
	}
	return retorno;
}

void procesarRecibir(){
	char* mensaje="holaE";
	char* respuesta=malloc(6);
	fflush(stdin);
	recibir(&socketMapa,respuesta,6);

	printf("Mensaje recibido del mapa: %s \n", respuesta);
	enviar(&socketMapa, mensaje, 6);
	free(respuesta);
	//close(socketDeMapa);
}


//Funcion que levanta los datos del entrenador

void getMetadataEntrenador() {

	t_config* configEntrenador = malloc(sizeof(t_config));
	configEntrenador->path = string_from_format("%sEntrenadores/%s/metadata", entrenador.rutaPokedex, entrenador.nombre);
	//configEntrenador->path = "/home/utnso/git/tp-2016-2c-SegmentationFault/PokedexClient/Debug/test6/metaDataEntrenador";
	configEntrenador = config_create(configEntrenador->path);

	entrenador.nombre = config_get_string_value(configEntrenador, "nombre");
	char* simbolo = config_get_string_value(configEntrenador, "simbolo");
	memcpy(&entrenador.simbolo, simbolo, sizeof(entrenador.simbolo));
	entrenador.cantVidas = config_get_int_value(configEntrenador, "vidas");
	char** hojaDeViaje = config_get_array_value(configEntrenador, "hojaDeViaje");
	//inicializo la posicion en 1;1
	entrenador.posicion[0] = 1;
	entrenador.posicion[1] = 1;
	entrenador.mapaActual = 0;

	printf("ENTRENADOR - nombre: '%s'. simbolo: '%c'. cantVidas: '%d'\n", entrenador.nombre, entrenador.simbolo, entrenador.cantVidas);
	pokemonMasFuerte.level = -1;

	int i = 0;
	while (hojaDeViaje[i] != NULL) {
		t_mapa* mapa = malloc(sizeof(t_mapa));
		mapa->nombreMapa = hojaDeViaje[i];

		printf("Mapa a recorrer: '%s' con los sig. objetivos: ",mapa->nombreMapa);

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
		printf("MAPA %s - IP: %s. PUERTO: %d \n", mapa->nombreMapa, mapa->ip, mapa->puerto);

		list_add(entrenador.hojaDeViaje, (void*) mapa);

		i++;
	}

	printf("La cantidad de mapas a recorrer es: %d \n", entrenador.hojaDeViaje->elements_count);

	recorrerEPrintearLista(entrenador.hojaDeViaje);

}

void recorrerEPrintearLista(t_list* unaLista){
	int i;
	t_mapa* unMapa;
	for (i = 0; i < unaLista->elements_count; i++) {
		unMapa = (t_mapa*) list_get(unaLista, i);
		printf("MAPA %s - IP: %s. PUERTO: %i \n",unMapa->nombreMapa,unMapa->ip,unMapa->puerto);
	}
}

void interactuarConMapas(){

	while(entrenador.hojaDeViaje != NULL){ //todo recorrer mapas
		t_mapa* mapa;
		mapa = (t_mapa*) list_get(entrenador.hojaDeViaje,entrenador.mapaActual);

		//socketMapa=conectarseA("10.0.2.15",1982);
		socketMapa = conectarseA(mapa->ip, mapa->puerto);
		if (socketMapa >0) printf("me conecte al mapa %s\n",mapa->nombreMapa);

		enviarInfoAlMapa();	//Envio al mapa los datos de entrenador y el objetivo (todo no enviar el objetivo)
		recibir(&socketMapa, &esMiTurno, sizeof(bool));

		//Solicito la posicion de la pokenest de mi proximo objetivo (se le notifica cual es el objetivo)
		//Se recibe la pos de la pokenest pasando por referencia y ya trae la pos
		int i = 0;

		solicitarUbicacionPokenest(&posObjX, &posObjY, i);
		recibir(&socketMapa, &esMiTurno, sizeof(bool));

		while(esMiTurno){

			//Confirmo no haber llegado a la pokenest
			if(entrenador.posicion[0]==posObjX && entrenador.posicion[1]==posObjY){
				//Solicito atrapar al pokemon ¡Llegue a la pokenest!
				char pokemon;
				memcpy(&pokemon, mapa->objetivos[i], sizeof(char));
				atraparUnPokemon(pokemon);//tener en cuenta que ya se habia enviado el primer objetivo
				i++; //se debe ir actualizando el objetivo
			}else{
				//No llegue pido para seguir avanzando
				avanzarHastaPokenest(posObjX, posObjY);
			}
			if (cumpliObjetivos){
				break;
			}
			recibir(&socketMapa, &esMiTurno, sizeof(bool));

		}
		liberarRecursosCapturados();
		close(socketMapa);
		entrenador.mapaActual++;
	}
}

void enviarInfoAlMapa(){
	t_MensajeEntrenador_Mapa mensaje;
	mensaje.nombreEntrenador = entrenador.nombre;
	mensaje.id = entrenador.simbolo;

	t_mapa* mapa;
	mapa = list_get(entrenador.hojaDeViaje,entrenador.mapaActual);
	memcpy(&mensaje.objetivoActual, mapa->objetivos[0], sizeof(mensaje.objetivoActual));//todo no le deberia enviar su objetivo
	entrenador.objetivoActual = mensaje.objetivoActual;

	mensaje.operacion = -1;//no es necesario pero se inicializa
	char* bufferAEnviar = malloc(sizeof(t_MensajeEntrenador_Mapa));
	serializarEntrenador_Mapa(&mensaje, bufferAEnviar);
	enviar(&socketMapa, bufferAEnviar, sizeof(t_MensajeEntrenador_Mapa));

	free(bufferAEnviar);
	printf("envie mis datos iniciales al Mapa\n");
}

void solicitarUbicacionPokenest(int* posx, int* posy, int index){
	t_MensajeEntrenador_Mapa mensaje;
	mensaje.operacion = 1;
	mensaje.nombreEntrenador = entrenador.nombre;
	mensaje.id = entrenador.simbolo;

	//t_mapa* mapa;
	//mapa = list_get(entrenador.hojaDeViaje,entrenador.mapaActual);
	//memcpy(&mensaje.objetivoActual, mapa->objetivos[index], sizeof(mensaje.objetivoActual));

	memcpy(&mensaje.objetivoActual, &entrenador.objetivoActual, sizeof(mensaje.objetivoActual));

	char* bufferAEnviar = malloc(sizeof(t_MensajeEntrenador_Mapa));
	serializarEntrenador_Mapa(&mensaje, bufferAEnviar);
	enviar(&socketMapa, bufferAEnviar, sizeof(t_MensajeEntrenador_Mapa));

	free(bufferAEnviar);

	int posicionX;
	int posicionY;
	int bytesRecibidosX = recibir(&socketMapa, &posicionX,sizeof(int));
	int bytesRecibidosY = recibir(&socketMapa, &posicionY,sizeof(int));

	if(bytesRecibidosX > 0 && bytesRecibidosY > 0){
		printf("Se recibio el tamanio correctamente \n");
		*posx = posicionX;
		*posy = posicionY;
		printf("posicionX: %d. posicionY: %d.\n",posicionX,posicionY);
	}else{
		printf("Se recibio un tamanio distinto al esperado \n");
	}
}

void avanzarHastaPokenest(int posicionXPokenest, int posicionYPokenest){
	t_MensajeEntrenador_Mapa mensaje;
	mensaje.operacion = 2;
	mensaje.nombreEntrenador = entrenador.nombre;
	mensaje.id = entrenador.simbolo;
	mensaje.objetivoActual = entrenador.objetivoActual;

	char* bufferAEnviar = malloc(sizeof(t_MensajeEntrenador_Mapa));
	serializarEntrenador_Mapa(&mensaje, bufferAEnviar);
	enviar(&socketMapa, bufferAEnviar, sizeof(t_MensajeEntrenador_Mapa));

	free(bufferAEnviar);

	avanzarPosicion(&entrenador.posicion[0], &entrenador.posicion[1],
			posicionXPokenest, posicionYPokenest);
	enviar(&socketMapa, &entrenador.posicion[0], sizeof(int));
	enviar(&socketMapa, &entrenador.posicion[1], sizeof(int));

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

void atraparUnPokemon(char pokemon){
	t_MensajeEntrenador_Mapa mensaje;
	mensaje.operacion = 3;
	mensaje.nombreEntrenador = entrenador.nombre;
	mensaje.id = entrenador.simbolo;
	mensaje.objetivoActual = pokemon;//Envio el pokemon que necesito el mapa me confirma la resolucion

	char* bufferAEnviar = malloc(sizeof(t_MensajeEntrenador_Mapa));
	serializarEntrenador_Mapa(&mensaje, bufferAEnviar);
	enviar(&socketMapa, bufferAEnviar, sizeof(t_MensajeEntrenador_Mapa));

	free(bufferAEnviar);

	int resolucionCaptura;
	recibir(&socketMapa, &resolucionCaptura, sizeof(int));
	if (resolucionCaptura == 0) {  // 0 es para capturar
		printf("se captura el pokemon: %c \n", pokemon);
		capturarPokemon();
		chequearObjetivos(pokemon);
	} else if (resolucionCaptura == 1) {  // 1 es por batalla y se envia el pokemon mas fuerte
		printf("se envia el pokemon mas fuerte: %s \n", pokemonMasFuerte.species);
		enviarPokemon(socketMapa, &pokemonMasFuerte);
		bool continuar = true;
		while(continuar){
			int resolucionDeBatalla = 0;
			recibir(&socketMapa, &resolucionDeBatalla, sizeof(int));
			if (resolucionDeBatalla == 1){
				printf("Mi pokemon mas fuerte gano la batalla. \n");
				continuar = false;
			}else if (resolucionDeBatalla == 0){
				printf("Mi pokemon mas fuerte perdio la batalla y fui seleccionado como victima. \n");
				muerteDelEntrenador();
				continuar = false;
			}else{
				printf("Mi pokemon mas fuerte perdio la batalla y debe batallar nuevamente. \n");

			}
		}
		cantDeadLocks ++;
	} else {
		//chequearVidas(entrenador);
	}
}

void capturarPokemon(){
	t_pokemon* pokemon = recibirPokemon(socketMapa);
	list_add(pokemonesCapturados[entrenador.mapaActual], (void*) pokemon);

	if (pokemonMasFuerte.level == -1){
		memcpy(&pokemonMasFuerte, pokemon, sizeof(t_pokemon));
	}else if (pokemonMasFuerte.level != -1 && pokemonMasFuerte.level < pokemon->level) {
			printf("Pokemon mas fuerte anterior: %s, de nivel %d \n",
					pokemonMasFuerte.species,
					pokemonMasFuerte.level);
			memcpy(&pokemonMasFuerte, pokemon, sizeof(t_pokemon));
	}
	printf("Pokemon mas fuerte actual: %s, de nivel %d \n",
			pokemonMasFuerte.species,
			pokemonMasFuerte.level);

}

void chequearObjetivos(char pokemon){
	t_mapa* mapaEnElQueEstoy = (t_mapa*) list_get(entrenador.hojaDeViaje, entrenador.mapaActual);

	int i=0;
	char objetivo;
	while(mapaEnElQueEstoy->objetivos[i]!=NULL){
		memcpy(&objetivo, mapaEnElQueEstoy->objetivos[i], sizeof(char));
		if (objetivo == pokemon ){
			break;
		}
		i++;
	}
	int cantObjetivos = (strlen((char*) mapaEnElQueEstoy->objetivos) /sizeof(char*));
	//printf("cantObjetivos: %d. \n", cantObjetivos);
	mapaEnElQueEstoy->objetivos[i] = "NO";//marco que ya no es un objetivo
	entrenador.objetivoActual = 0;

	if(mapaEnElQueEstoy->objetivos[i+1]==NULL){
		if (list_size(entrenador.hojaDeViaje) - 1 == entrenador.mapaActual) {
			//copiarMedallaDelMapa();
			printf("Eres un maestro pokemon completaste la aventura");
			//GenerarReporteDeAventura();
		}else{
			printf("Complete los objetivos del mapa actual");

			cumpliObjetivos = true;
			//copiarMedallaDelMapa();
			//conectarseConElSiguienteMapa();//esto no es necesario hacerlo aca (va a seguir con la logica en interacturaConMapas)
		}
	}else{
		i++;
		if (i < cantObjetivos) {
			recibir(&socketMapa, &esMiTurno, sizeof(bool));
			memcpy(&entrenador.objetivoActual, mapaEnElQueEstoy->objetivos[i], sizeof(char));
			solicitarUbicacionPokenest(&posObjX, &posObjY, i);
			//seguirAtrapando();
		}
	}
}

void verificarTurno(){
	int mensajeLen = 0;
	recibir(&socketMapa, &mensajeLen, sizeof(int));
	char* mensajeTurno = malloc(mensajeLen);
	recibir(&socketMapa, mensajeTurno, mensajeLen);
	esMiTurno = strcmp(mensajeTurno, "turno concedido") == 0;
	if (esMiTurno) {
		free(mensajeTurno);
		return;
	}
}

void agregarVida(){
		entrenador.cantVidas ++;
		printf("+1 Vida \n");
}

void quitarVida(){
	entrenador.cantVidas --;
	printf("-1 Vida \n");

}

void controladorDeSeniales(int signo)
{
    if (signo == SIGUSR1){
        agregarVida();
        printf("La cantidad de vidas del jugador %s es: %i \n", entrenador.nombre, entrenador.cantVidas);
    }
    else if (signo == SIGKILL){
    	printf("Abandono el juego \n");
    	exit(1);
    }
    else if (signo == SIGINT){
        printf("Abandono el juego \n");
        exit(1);
    }
    else if (signo == SIGTERM){
    	quitarVida();
    	perdiElJuego();
    	printf("La cantidad de vidas del jugador %s es: %i \n", entrenador.nombre, entrenador.cantVidas);
    }
}

void manejoDeSeniales(){

	while(1){//todo no es necesario while (1). Se llama a la funcion antes de comenzar una operacion

		if (signal(SIGUSR1, controladorDeSeniales) == SIG_ERR){}
		if (signal(SIGKILL, controladorDeSeniales) == SIG_ERR){}
		if (signal(SIGINT, controladorDeSeniales) == SIG_ERR){}
		if (signal(SIGTERM, controladorDeSeniales) == SIG_ERR){}
	}
}

void perdiElJuego(){
	if(entrenador.cantVidas == 0){
		printf("El entrenador %s perdio el juego por falta de vidas \n", entrenador.nombre );
		liberarRecursosCapturados();
		exit(1);
	}
}

void liberarRecursosCapturados(){
/*	t_MensajeEntrenador_Mapa mensaje;
	mensaje.operacion = 5;
	mensaje.nombreEntrenador = entrenador.nombre;
	mensaje.id = entrenador.simbolo;

	char* bufferAEnviar = malloc(sizeof(t_MensajeEntrenador_Mapa));
	serializarEntrenador_Mapa(&mensaje, bufferAEnviar);
	enviar(&socketMapa, bufferAEnviar, sizeof(t_MensajeEntrenador_Mapa));

	free(bufferAEnviar);
*/
	int i = 0;
	int m = entrenador.mapaActual;
	int cantPokemones = list_size(pokemonesCapturados[m]);
	enviar(&socketMapa, &cantPokemones, sizeof(int));
	while (i < cantPokemones){
		t_pokemon* pokemonCapturado = (t_pokemon*) list_remove(pokemonesCapturados[m], i);
		enviarPokemon(socketMapa, pokemonCapturado);
		i++;
	}
	//list_clean_and_destroy_elements(pokemonesCapturados[m], (void*) destruirPokemon);
}

void destruirPokemon(t_pokemon* unPokemon){
	free(unPokemon->species);
	free(unPokemon);
}

void seniales(){

	signal(SIGUSR1, controladorDeSeniales);
	signal(SIGKILL, controladorDeSeniales);
	signal(SIGINT, controladorDeSeniales);
	signal(SIGTERM, controladorDeSeniales);
}

void muerteDelEntrenador(){
	entrenador.cantVidas--;
	if(entrenador.cantVidas==0){
		printf("Te quedaste sin vidas \n");
		//borrarDirectorioDeBill();
		//borra sus medallas
		//perdiElJuego();
		shutdown(socketMapa,2);
	}else{
		printf("Perdiste una vida, te queda:%i \n",entrenador.cantVidas);
		liberarRecursosCapturados();
		close(socketMapa);
	}
}

/*
void conectarseAlMapa(t_mapa* unMapa){

	if(conectarseA(unMapa->ip,unMapa->puerto) == 0){
		printf("Conexion realizada con exito \n");
	}
	else{
		printf("Conexion fracaso \n");
		exit(-1);
	}
}
*/
