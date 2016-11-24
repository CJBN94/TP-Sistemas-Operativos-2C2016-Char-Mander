#!/bin/sh
#CREAR DIRECTORIOS (de Bill y medallas)
dir=`pwd`
cd ${dir}/Recursos/PokedexBase/Entrenadores
for x in  Ash Blue Gary Red
do
	cd ${x}
        mkdir "Dir de Bill"
        mkdir "medallas"
	cd ..
done 

cd
cd ${dir}/Recursos/PokedexCompleto/Entrenadores
for x in  Ash Blue Brook Gary Misty Red
do
	cd ${x}
        mkdir "Dir de Bill"
        mkdir "medallas"
	cd ..
done 

cd
cd ${dir}/Recursos/Pokedex/Entrenadores
for x in  Ash AshDead Blue LadyDead Red RedDead TamerDead
do
	cd ${x}
        mkdir "Dir de Bill"
        mkdir "medallas"
	cd ..
done 

echo Directorios creados exitosamente
echo
