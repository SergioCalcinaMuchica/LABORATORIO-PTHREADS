#!/bin/bash
FUENTE=EJERCICIO3.c
EXE=parte3
RES=resultados_PARTE3.csv
TABLA=tabla_PARTE3.csv
ITEMS=100000
REPS=3

gcc -O2 -pthread $FUENTE -o $EXE || exit 1
echo "experimento,productores,consumidores,capacidad,items,tiempo_ms,producidos,consumidos,duplicados,faltantes" > $RES

# Experimento A: variar la capacidad del buffer con 2 productores y 2 consumidores
for cap in 1 2 4 8 16 64 256 1024; do
  for rep in $(seq $REPS); do
    ./$EXE 2 2 $cap $ITEMS | grep '^CSV' | cut -d, -f2- | sed 's/^/A,/' >> $RES
  done
done

# Experimento B: variar productores y consumidores con capacidad 8
for pc in "1 1" "2 2" "4 4" "8 8" "1 4" "4 1" "2 5" "5 2" "1 8" "8 1"; do
  read p c <<< "$pc"
  for rep in $(seq $REPS); do
    ./$EXE $p $c 8 $ITEMS | grep '^CSV' | cut -d, -f2- | sed 's/^/B,/' >> $RES
  done
done

# Tabla resumen: tiempo promedio y suma de duplicados y faltantes de todas las repeticiones
echo "experimento,productores,consumidores,capacidad,items,tiempo_ms_prom,producidos,consumidos,duplicados,faltantes,reps" > $TABLA
LC_ALL=C awk -F, 'NR>1{k=$1","$2","$3","$4","$5; t[k]+=$6; p[k]=$7; c[k]=$8; d[k]+=$9; f[k]+=$10; n[k]++}
END{for(k in t){
printf "%s,%.3f,%s,%s,%d,%d,%d\n",k,t[k]/n[k],p[k],c[k],d[k],f[k],n[k]}}' $RES \
| LC_ALL=C sort -t, -k1,1 -k2,2n -k3,3n -k4,4n >> $TABLA

echo "Listo. Resultados en $RES y resumen en $TABLA"