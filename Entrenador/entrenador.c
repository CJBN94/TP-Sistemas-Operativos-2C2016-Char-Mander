/*
 * entrenador.c
 *
 */

#include "entrenador.h"

int main(int argc, char **argv) {


	t_entrenador* datosEntrenador = malloc(sizeof(t_entrenador));



	//datosEntrenador->nombre = NULL;
	//datosEntrenador->rutaPokedex = NULL;
	char *logFile = NULL;

	//assert(("ERROR - No se pasaron argumentos", argc > 1)); // Verifica que se haya pasado al menos 1 parametro, sino falla

	//Parametros
	int i;
	for (i = 0; i < argc; i++) {
		if (i == 0) {
			datosEntrenador->nombre = argv[i + 1];
			printf("Nombre Entrenador: '%s'\n", datosEntrenador->nombre);
		}
		if (i == 1) {
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

	datosEntrenador->hojaDeViaje = list_create();

	//Levanto los datos del metadata de Entrenador
	//conectarseA("127.0.0.1",6000);
	getMetadataEntrenador(datosEntrenador);
	//CONFIGURACION DEL ENTRENADOR


	//faltan los objetivos



	free(datosEntrenador);
	return EXIT_SUCCESS;

}


//Funcion que levanta los datos del entrenador

void getMetadataEntrenador(t_entrenador* datosEntrenador) {


	//t_entrenador* datosEntrenador = malloc(sizeof(t_entrenador));
	t_config* configEntrenador = malloc(sizeof(t_config));
		configEntrenador->path = string_from_format("/home/utnso/Pokedex/Entrenadores/%s/metadata","Red");
		configEntrenador = config_create(configEntrenador->path);

		datosEntrenador->nombre = config_get_string_value(configEntrenador, "nombre");
		datosEntrenador->simbolo = config_get_string_value(configEntrenador, "simbolo");
		datosEntrenador->cantVidas = config_get_int_value(configEntrenador, "vidas");
		char** hojaDeViaje = config_get_array_value(configEntrenador,
				"hojaDeViaje");

		printf("El nombre del Entrenador es: %s \n", datosEntrenador->nombre);
		printf("El simbolo que representa al Entrenador es: %s \n",datosEntrenador->simbolo);
		printf("La cantidad de vidas del Entrenador es: %d \n", datosEntrenador->cantVidas);

		int i = 0;
		while (hojaDeViaje[i] != NULL) {
			t_mapa* mapa=malloc(sizeof(t_mapa));
			mapa->nombreMapa = hojaDeViaje[i];

			printf("El mapa que debe recorrer el datosEntrenador: %s \n",
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

			list_add(datosEntrenador->hojaDeViaje, (void*)mapa);

			i++;
		}

		printf("La cantidad de mapas a recorrer es: %d \n", datosEntrenador->hojaDeViaje->elements_count);

		recorrerEPrintearLista(datosEntrenador->hojaDeViaje);



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


void chequearVidas(t_entrenador* unEntrenador){
 unEntrenador->cantVidas--;
 if(unEntrenador->cantVidas==0){
  printf("Te quedaste sin vidas \n");
  //borrarDirectorioDeBill();
  //borra sus medallas
  shutdown(socketEntrenador,2);
 }else{
  printf("Perdiste una vida, te queda:%i \n",unEntrenador->cantVidas);
 }
}

void chequearObjetivos(t_entrenador* unEntrenador,char pokemon){
	t_mapa* mapaEnElQueEstoy=(t_mapa*)list_get(unEntrenador->hojaDeViaje,unEntrenador->mapaActual);

		int j=0;
		int mapa = (int)mapaEnElQueEstoy->objetivos[j];
		while(!(mapa==pokemon)){
		j++;
		}
		mapaEnElQueEstoy->objetivos[j]=0;

		if(mapaEnElQueEstoy->objetivos[j+1]==NULL){
			if(list_size(unEntrenador->hojaDeViaje)==unEntrenador->mapaActual){
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

int solicitarUbicacionPokenest(){
	//Solicito la posicion de mi proximo objetivo
	int posicionXoY;
	if(recibir(&posicionXoY,socketEntrenador,sizeof(int)*2) > 0){
		printf("Se recibio el tamanio correctamente \n");
	}else{
		printf("Se recibio un tamanio distinto al esperado \n");
	}
	return posicionXoY;
}


void avanzarHastaPokenest(t_entrenador* unEntrenador, int posicionXPokenest, int posicionYPokenest){
	int flagMovimiento = 0;
	enviar(socketDeMapa, &flagMovimiento, sizeof(int));
	recibir(socketEntrenador, &flagMovimiento, sizeof(int));
	avanzarPosicion(&unEntrenador->posicion[0], &unEntrenador->posicion[1],
			&posicionXPokenest, &posicionYPokenest);
}

void pedirAvanzarUnaPosicion(){
	//Enviar lo que sea que maneje guido para las posiciones
}

void atraparUnPokemon(char pokemon,t_entrenador* unEntrenador){
	int resolucionCaptura;
	//Envio el pokemon que necesito el mapa me confirma la resolucion
	enviar(socketDeMapa,&pokemon,sizeof(char));
	//Recibo lo que me responde
	recibir(socketEntrenador,&resolucionCaptura,sizeof(int));
	if(resolucionCaptura==0){
		chequearObjetivos(unEntrenador,pokemon);
	}else{
		chequearVidas(unEntrenador);
	}
}

void interactuarConMapa(t_entrenador* unEntrenador){

	t_mapa* mapa = malloc(sizeof(t_mapa));
	mapa = list_get(unEntrenador->hojaDeViaje,unEntrenador->mapaActual);
	conectarseAlMapa(mapa);
	int i = 0;
	while(mapa->objetivos[i] != NULL){
		//Solicito la posicion de la pokenest de mi proximo objetivo
		//Envio el pokemon a atrapar al mapa
		int posicionX;
		int posicionY;
		if(enviar(socketDeMapa, &mapa->objetivos[i], sizeof(char))!= -1){
				printf("Datos enviados satisfactoriamente \n");
			}
			else{
				printf("No se han podido enviar todos los datos \n");
			}
		//Recibo la posicion del pokemon en dos partes
		posicionX=solicitarUbicacionPokenest();
		posicionY=solicitarUbicacionPokenest();
		//Confirmo no haber llegado a la pokenest
		if(unEntrenador->posicion[0]==posicionX && unEntrenador->posicion[1]==posicionY){
			//Solicito atrapar al pokemon ¡Llegue a la pokenest!
			atraparUnPokemon(mapa->objetivos[i],unEntrenador);
		}else{
			//No llegue pido para seguir avanzando
			//Pido autorizacion para avanzar una posicion;
			avanzarHastaPokenest(unEntrenador, posicionX, posicionY);

		}



	}
}



