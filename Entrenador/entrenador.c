/*
 * entrenador.c
 *
 */

#include "entrenador.h"

int main(int argc, char **argv) {
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
	 char* logFile = "/home/utnso/git/tp-2016-2c-SegmentationFault/Entrenador/logEntrenador";

	//Creo el archivo de Log
	logEntrenador = log_create(logFile, "ENTRENADOR", 0, LOG_LEVEL_TRACE);
	//pthread_t rcvThread;
	//pthread_create(&rcvThread, NULL, (void*) recibirTurnoConcedido,NULL);

	entrenador.hojaDeViaje = list_create();

	//Levanto los datos del metadata de Entrenador
	getMetadataEntrenador();
	//socketMapa=conectarseA("10.0.2.15",1982);
	interactuarConMapa();
	//procesarRecibir();


	//chequearObjetivos(entrenador,'M');

	//CONFIGURACION DEL ENTRENADOR

	//pthread_join(rcvThread, NULL);

	//faltan los objetivos

	return EXIT_SUCCESS;
}

void procesarRecibir(){
	int bytesRecibidos;
	char* mensaje="holaE";
	char* respuesta=malloc(6);
	fflush(stdin);
	bytesRecibidos=recibir(&socketMapa,respuesta,6);

	printf("Mensaje recibido del mapa: %s \n", respuesta);
	enviar(&socketMapa, mensaje, 6);
	free(respuesta);
	//close(socketDeMapa);
}


//Funcion que levanta los datos del entrenador

void getMetadataEntrenador() {

	t_config* configEntrenador = malloc(sizeof(t_config));
	configEntrenador->path = string_from_format("/home/utnso/Pokedex/Entrenadores/%s/metadata","Red");
	configEntrenador = config_create(configEntrenador->path);

	entrenador.nombre = config_get_string_value(configEntrenador, "nombre");
	char* simbolo = config_get_string_value(configEntrenador, "simbolo");
	memcpy(&entrenador.simbolo, simbolo, sizeof(entrenador.simbolo));
	entrenador.cantVidas = config_get_int_value(configEntrenador, "vidas");
	char** hojaDeViaje = config_get_array_value(configEntrenador, "hojaDeViaje");
	//inicializo la posicion en 1;1
	entrenador.posicion[0] = 1;
	entrenador.posicion[1] = 1;

	printf("El nombre del Entrenador es: %s \n", entrenador.nombre);
	printf("El simbolo que representa al Entrenador es: %c \n",entrenador.simbolo);
	printf("La cantidad de vidas del Entrenador es: %d \n", entrenador.cantVidas);

	int i = 0;
	while (hojaDeViaje[i] != NULL) {
		t_mapa* mapa=malloc(sizeof(t_mapa));
		mapa->nombreMapa = hojaDeViaje[i];

		printf("El mapa que debe recorrer el entrenador: %s \n",
				mapa->nombreMapa);

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
		printf("La IP del mapa %s es: %s \n", mapa->nombreMapa,mapa->ip);
		printf("El puerto del mapa %s es: %d \n", mapa->nombreMapa,mapa->puerto);

		list_add(entrenador.hojaDeViaje, (void*)mapa);

		i++;
	}

	printf("La cantidad de mapas a recorrer es: %d \n", entrenador.hojaDeViaje->elements_count);

	recorrerEPrintearLista(entrenador.hojaDeViaje);

}

void recorrerEPrintearLista(t_list* unaLista){
	int i;
	t_mapa* unMapa=malloc(sizeof(t_mapa));
	for(i=0;i<unaLista->elements_count;i++){
		unMapa=(t_mapa*)list_get(unaLista,i);
		printf("%s \n",unMapa->nombreMapa);
		printf("%s \n",unMapa->ip);
		printf("%i \n",unMapa->puerto);
	}
}
//Cambia la posicion del entrenador segun determine el mapa.


void chequearVidas(){
	entrenador.cantVidas--;
	if(entrenador.cantVidas==0){
		printf("Te quedaste sin vidas \n");
		//borrarDirectorioDeBill();
		//borra sus medallas
		shutdown(socketEntrenador,2);
	}else{
		printf("Perdiste una vida, te queda:%i \n",entrenador.cantVidas);
	}
}

void chequearObjetivos(char pokemon){
	t_mapa* mapaEnElQueEstoy=(t_mapa*)list_get(entrenador.hojaDeViaje,entrenador.mapaActual);

	int j=0;
	char objetivo;
	while(!(objetivo==pokemon) && mapaEnElQueEstoy->objetivos[j]!=NULL){
		memcpy(&objetivo, mapaEnElQueEstoy->objetivos[j], sizeof(char));
		j++;
	}
	mapaEnElQueEstoy->objetivos[j]=0;

	if(mapaEnElQueEstoy->objetivos[j+1]==NULL){
		if(list_size(entrenador.hojaDeViaje)==entrenador.mapaActual){
			//copiarMedallaDelMapa();
			printf("Eres un maestro pokemon completaste la aventura");
			//GenerarReporteDeAventura();
		}else{
			//copiarMedallaDelMapa();
			//conectarseConElSiguienteMapa();
		}
	}else{
		//AvisarAlMapaQueDeboSeguirAtrapandoPokemon();
	}
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


void conectarseAlMapa(t_mapa* unMapa){

	if(conectarseA(unMapa->ip,unMapa->puerto) == 0){
		printf("Conexion realizada con exito \n");
	}
	else{
		printf("Conexion fracaso \n");
		exit(-1);
	}
}

void enviarInfoAlMapa(){
	enum_procesos proceso = ENTRENADOR;//enviar siempre antes del struct
	enviar(&socketMapa, &proceso, sizeof(int));

	t_mapa* mapa;
	mapa = list_get(entrenador.hojaDeViaje,entrenador.mapaActual);
	t_MensajeEntrenador_Mapa mensaje;
	mensaje.nombreEntrenador = entrenador.nombre;
	mensaje.id = entrenador.simbolo;
	memcpy(&mensaje.objetivoActual, mapa->objetivos[0], sizeof(mensaje.objetivoActual));
	char* bufferAEnviar = malloc(sizeof(t_MensajeEntrenador_Mapa));
	serializarEntrenador_Mapa(&mensaje, bufferAEnviar);
	enviar(&socketMapa, bufferAEnviar, sizeof(t_MensajeEntrenador_Mapa));

	free(bufferAEnviar);
}

void recibirTurnoConcedido(){
	while(1){
		int mensajeLen = 0;
		recibir(&socketMapa, &mensajeLen, sizeof(int));
		char* mensajeTurno = malloc(mensajeLen);
		recibir(&socketMapa, mensajeTurno, mensajeLen);
		if(strcmp(mensajeTurno, "turno concedido")==0){
			//turnoConcedido = true;
		}
	}
}

void solicitarUbicacionPokenest(int* posx, int* posy){
	enum_procesos proceso = ENTRENADOR;//enviar siempre antes del struct
	enviar(&socketMapa, &proceso, sizeof(int));

	t_MensajeEntrenador_Mapa mensaje;
	mensaje.operacion = 1;
	mensaje.nombreEntrenador = entrenador.nombre;
	mensaje.id = entrenador.simbolo;

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
	enum_procesos proceso = ENTRENADOR;//enviar siempre antes del struct
	enviar(&socketMapa, &proceso, sizeof(int));

	t_MensajeEntrenador_Mapa mensaje;
	mensaje.operacion = 2;
	mensaje.nombreEntrenador = entrenador.nombre;
	mensaje.id = entrenador.simbolo;

	char* bufferAEnviar = malloc(sizeof(t_MensajeEntrenador_Mapa));
	serializarEntrenador_Mapa(&mensaje, bufferAEnviar);
	enviar(&socketMapa, bufferAEnviar, sizeof(t_MensajeEntrenador_Mapa));

	free(bufferAEnviar);

	avanzarPosicion(&entrenador.posicion[0], &entrenador.posicion[1],
			posicionXPokenest, posicionYPokenest);
	enviar(&socketMapa, &entrenador.posicion[0], sizeof(int));
	enviar(&socketMapa, &entrenador.posicion[1], sizeof(int));

}

void atraparUnPokemon(char pokemon){
	enum_procesos proceso = ENTRENADOR;//enviar siempre antes del struct
	enviar(&socketMapa, &proceso, sizeof(int));

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
	recibir(&socketMapa,&resolucionCaptura,sizeof(int));
	if(resolucionCaptura==0){
		chequearObjetivos(pokemon);
	}else{
		chequearVidas(entrenador);
	}
}

void interactuarConMapa(){

	t_mapa* mapa;
	mapa = list_get(entrenador.hojaDeViaje,entrenador.mapaActual);
	//conectarseAlMapa(mapa);

	//socketMapa=conectarseA("10.0.2.15",1982);
	socketMapa = conectarseA(mapa->ip, mapa->puerto);
	if (socketMapa >0) printf("me conecte al mapa %s\n",mapa->nombreMapa);

	enviarInfoAlMapa();	//Envio al mapa los datos de entrenador y el primer pokemon a atrapar
	verificarTurno();

	//Solicito la posicion de la pokenest de mi proximo objetivo
	//Se recibe la pos de la pokenest pasando por referencia y ya trae la pos
	int posicionX = 0;
	int posicionY = 0;
	solicitarUbicacionPokenest(&posicionX, &posicionY);

	int i = 0;
	while(esMiTurno){

		//Confirmo no haber llegado a la pokenest
		if(entrenador.posicion[0]==posicionX && entrenador.posicion[1]==posicionY){
			//Solicito atrapar al pokemon ¡Llegue a la pokenest!
			char pokemon;
			memcpy(&pokemon, mapa->objetivos[i], sizeof(char));
			atraparUnPokemon(pokemon);//tener en cuenta que ya se habia enviado el primer objetivo
			i++; //se debe ir actualizando el objetivo
		}else{
			//No llegue pido para seguir avanzando
			avanzarHastaPokenest(posicionX, posicionY);
		}
		verificarTurno();
	}
}

void verificarTurno(){
	int mensajeLen = 0;
	recibir(&socketMapa, &mensajeLen, sizeof(int));
	char* mensajeTurno = malloc(mensajeLen);
	recibir(&socketMapa, mensajeTurno, mensajeLen);
	esMiTurno = strcmp(mensajeTurno, "turno concedido") == 0;
	if (!esMiTurno) {
		printf("no es mi turno de realizar una operacion\n");
	}
}
