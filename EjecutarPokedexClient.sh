#!/bin/sh
DEPLOY_FOLDER="/home/utnso/git/tp-2016-2c-SegmentationFault"

#CREO DIRECTORIO DONDE VA A ESTAR EL FILE SYSTEM
cd
rm -rf FUSE
mkdir FUSE
PATH_POKEDEX="/home/utnso/FUSE"

#EJECUTAR POKEDEX CLIENT
cd /home/utnso/git/tp-2016-2c-SegmentationFault/PokedexClient/Debug
./PokedexClient $PATH_POKEDEX -f -s
