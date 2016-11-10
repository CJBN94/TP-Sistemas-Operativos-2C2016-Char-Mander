#!/bin/sh
DEPLOY_FOLDER="/home/utnso/SegmentationFault"

#CREO DIRECTORIO DONDE VA A ESTAR EL FILE SYSTEM
rm -rf $DEPLOY_FOLDER/POKEDEX
mkdir -p $DEPLOY_FOLDER/POKEDEX

#EJECUTAR FUSE
cd /home/utnso/SegmentationFault/PokedexClient/Debug
./PokedexClient $DEPLOY_FOLDER/POKEDEX -f
