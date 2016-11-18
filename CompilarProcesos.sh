#COMPILAR TODOS LOS PROCESOS
dir=`pwd`
for x in Conexiones PokedexServer PokedexClient Mapa Entrenador
do
	cd ${x}/Debug
	make clean
	make all
	cd ${dir}
done 

echo Procesos compilados exitosamente
echo

