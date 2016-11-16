#!/bin/sh
#EJECUTAR ENTRENADOR

DEPLOY_FOLDER="/home/utnso/SegmentationFault"
PATHPOKEDEX="/home/utnso/SegmentationFault/Recursos/PokedexBase"

if [ "$#" -ne 1 ] || [ ! $1 ] ; then
	echo "Uso: $0 [Nombre-Entrenador]"
	echo 
	exit 33
fi

NOMBRE_ENTRENADOR=$1

cd $DEPLOY_FOLDER/Entrenador/Debug
./Entrenador $NOMBRE_ENTRENADOR $PATHPOKEDEX

