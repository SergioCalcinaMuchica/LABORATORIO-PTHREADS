#!/bin/bash
gcc -O2 -pthread EJERCICIO1.c -o parte1 || exit 1
echo "n,hilos,t_seq,t_par,speedup,eficiencia,ok" > resultados_PARTE1.csv
for n in 100000 1000000 10000000 100000000; do
  for t in 1 2 4 8; do
    for rep in 1 2 3 4 5; do
      ./parte1 $n $t | grep '^CSV' | cut -d, -f2- >> resultados_PARTE1.csv
    done
  done
done

echo "n,hilos,t_seq_prom,t_par_prom,speedup,eficiencia" > tabla_PARTE1.csv
LC_ALL=C awk -F, 'NR>1{k=$1","$2; s[k]+=$3; p[k]+=$4; c[k]++}
END{for(k in s){ts=s[k]/c[k]; tp=p[k]/c[k]; split(k,a,",");
printf "%s,%s,%.6f,%.6f,%.3f,%.3f\n",a[1],a[2],ts,tp,ts/tp,ts/tp/a[2]}}' resultados_PARTE1.csv \
| LC_ALL=C sort -t, -k1,1n -k2,2n >> tabla_PARTE1.csv