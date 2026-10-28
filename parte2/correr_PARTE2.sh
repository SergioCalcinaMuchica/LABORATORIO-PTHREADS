#!/bin/bash
RES=resultados_PARTE2.csv
TABLA=tabla_PARTE2.csv

gcc -O2 -pthread EJERCICIO2.c -o parte2 || exit 1
echo "n,hilos,t_seq,t_mutex,t_priv,ok_mutex,ok_priv" > $RES

for n in 100000 1000000 10000000 100000000; do
  for t in 1 2 4 8; do
    for rep in 1 2 3; do
      ./parte2 $n $t | grep '^CSV' | cut -d, -f2- >> $RES
    done
  done
done

echo "n,hilos,t_seq,t_mutex,t_priv,ok_mutex,ok_priv,reps" > $TABLA
LC_ALL=C awk -F, 'NR>1{k=$1","$2; s[k]+=$3; m[k]+=$4; p[k]+=$5; om[k]+=$6; op[k]+=$7; c[k]++}
END{for(k in s){
printf "%s,%.6f,%.6f,%.6f,%d,%d,%d\n",k,s[k]/c[k],m[k]/c[k],p[k]/c[k],om[k],op[k],c[k]}}' $RES \
| LC_ALL=C sort -t, -k1,1n -k2,2n >> $TABLA