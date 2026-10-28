# Laboratorio A: Programación con Pthreads

Laboratorio del curso **Computación Paralela y Distribuida** (Escuela Profesional de Ciencia de la Computación, UNSA).

**Estudiante.** Calcina Muchica, Sergio Eliseo
**Docente.** Mamani Aliaga, Alvaro Henry

El laboratorio aplica programación multithreading con Pthreads en C para resolver tres problemas que requieren repartir el trabajo entre varios threads, compartir datos y sincronizar los accesos.

1. **Parte I.** Estadísticas de un conjunto de datos (suma, mínimo, máximo y promedio) con resultados parciales por thread.
2. **Parte II.** Construcción paralela de un histograma de diez categorías, comparando un mutex compartido con histogramas privados por thread.
3. **Parte III.** Productores y consumidores con un buffer circular acotado, sincronizado con un mutex y dos semáforos.

## Estructura del repositorio

```
paralela/
├── README.md
├── parte1/
│   ├── EJERCICIO1.c              # Versión secuencial y paralela de las estadísticas
│   └── correr_PARTE1.sh          # Ejecuta los experimentos de la Parte I
├── parte2/
│   ├── EJERCICIO2.c              # Histograma secuencial, con mutex y privado
│   └── correr_PARTE2.sh          # Ejecuta los experimentos de la Parte II
├── parte3/
│   ├── EJERCICIO3_secuencial.c   # Versión secuencial de referencia
│   ├── EJERCICIO3.c              # Productores y consumidores con mutex y semáforos
│   └── correr_PARTE3.sh          # Ejecuta los experimentos de la Parte III
└── resultados/
    ├── resultados_PARTE*.csv     # Resultados de cada ejecución
    ├── tabla_PARTE*.csv          # Tablas resumen (promedios)
    └── graficas.ipynb            # Notebook que genera las gráficas
```

## Entorno de ejecución

| Característica | Valor |
|---|---|
| Procesador | AMD Ryzen 5 4500U with Radeon Graphics |
| Núcleos | 6 físicos, 1 hilo por núcleo |
| Caché | L1 de 32 KiB (datos) y 32 KiB (instrucciones) por núcleo, L2 de 512 KiB por núcleo, L3 de 8 MiB |
| Memoria RAM | 7,2 GiB |
| Sistema operativo | Ubuntu 24.04.1 LTS |
| Compilador | GCC 13.3.0 |

## Requisitos

* GCC con soporte para Pthreads (incluido por defecto en Linux).
* Bash para ejecutar los scripts de experimentos.
* Python 3 con Jupyter, pandas y matplotlib para las gráficas (opcional).

```bash
pip install jupyter pandas matplotlib
```

## Compilación y ejecución

Cada programa se compila con `-O2 -pthread`. Los comandos deben ejecutarse **dentro de la carpeta de cada parte**.

### Parte I. Estadísticas del conjunto de datos

```bash
cd parte1
gcc -O2 -pthread EJERCICIO1.c -o parte1
./parte1 <N> <threads>
```

Ejemplo con 10 000 000 de datos y 4 threads.

```bash
./parte1 10000000 4
```

El programa calcula las estadísticas de forma secuencial y paralela, verifica que ambos resultados coincidan y muestra los tiempos (en segundos), el speedup y la eficiencia.

### Parte II. Histograma paralelo

```bash
cd parte2
gcc -O2 -pthread EJERCICIO2.c -o parte2
./parte2 <N> <threads>
```

Ejemplo con 1 000 000 de datos y 8 threads.

```bash
./parte2 1000000 8
```

El programa construye el histograma de forma secuencial, con mutex compartido y con histogramas privados por thread. Verifica que la suma de las diez categorías sea igual a N y que los tres histogramas coincidan.

### Parte III. Productores y consumidores

Versión paralela (con mutex y semáforos).

```bash
cd parte3
gcc -O2 -pthread EJERCICIO3.c -o parte3
./parte3 <productores> <consumidores> <capacidad> <elementos_por_productor>
```

Ejemplo con 2 productores, 5 consumidores, buffer de capacidad 8 y 1000 elementos por productor.

```bash
./parte3 2 5 8 1000
```

Si no se pasan argumentos, se usan los valores por defecto definidos en el código. El programa muestra el tiempo de ejecución y verifica que todos los elementos producidos se consuman exactamente una vez, reportando la cantidad de elementos duplicados y faltantes.

Versión secuencial de referencia. No recibe argumentos; usa los valores definidos en el código (2 productores equivalentes, capacidad 8 y 1000 elementos por productor) y muestra los totales producidos y consumidos.

```bash
gcc -O2 EJERCICIO3_secuencial.c -o secuencial
./secuencial
```

## Experimentos

Cada parte incluye un script que compila el programa, ejecuta varias configuraciones con repeticiones (5 en la Parte I y 3 en las Partes II y III) y genera dos archivos CSV, uno con todas las ejecuciones (`resultados_PARTE*.csv`) y otro con los promedios (`tabla_PARTE*.csv`). Los CSV se crean en la carpeta de cada parte y luego se movieron a `resultados/`.

```bash
cd parte1 && bash correr_PARTE1.sh && cd ..
cd parte2 && bash correr_PARTE2.sh && cd ..
cd parte3 && bash correr_PARTE3.sh && cd ..
```

Los experimentos realizados fueron los siguientes.

* **Parte I y Parte II.** N igual a 100 000, 1 000 000, 10 000 000 y 100 000 000, con 1, 2, 4 y 8 threads.
* **Parte III, experimento A.** 2 productores y 2 consumidores, variando la capacidad del buffer entre 1 y 1024.
* **Parte III, experimento B.** Buffer de capacidad 8, con combinaciones balanceadas y desbalanceadas de productores y consumidores.

Los CSV generados se encuentran en la carpeta `resultados/`. Las gráficas del informe se generan abriendo `resultados/graficas.ipynb` con Jupyter.

```bash
cd resultados
jupyter notebook graficas.ipynb
```

> **Nota.** Los tiempos dependen del equipo, por lo que al ejecutar los experimentos en otra máquina los valores serán distintos, aunque las tendencias deberían mantenerse.
