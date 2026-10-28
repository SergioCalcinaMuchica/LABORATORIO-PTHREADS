#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define CATEGORIAS 10

int* datos;
long long n = 1000;
int thread_count = 4;
long long histograma_sec[CATEGORIAS];

void calcular_histograma_secuencial(int* datos, long long cantidad_datos, long long* hist) {
    for (int c = 0; c < CATEGORIAS; c++) {
        hist[c] = 0;
    }

    for (long long i = 0; i < cantidad_datos; i++) {
        int categoria = datos[i] / 10;
        hist[categoria]++;
    }
}

long long histograma_par[CATEGORIAS];
pthread_mutex_t mutex_histograma;
void* calcular_histograma_paralelo(void* rango) {
    long mi_rango = (long)rango;
    long long base = n / thread_count;
    long long resto = n % thread_count;
    long long inicio;
    long long final;

    if (mi_rango < resto) {
        inicio = mi_rango * base + mi_rango;
        final = inicio + base + 1;
    } else {
        inicio = mi_rango * base + resto;
        final = inicio + base;
    }

    for (long long i = inicio; i < final; i++) {
        int categoria = datos[i] / 10;

        pthread_mutex_lock(&mutex_histograma);
        histograma_par[categoria]++;
        pthread_mutex_unlock(&mutex_histograma);
    }

    return NULL;
}

long long histograma_priv[CATEGORIAS];
void* calcular_histograma_privado(void* rango) {
    long mi_rango = (long)rango;
    long long base = n / thread_count;
    long long resto = n % thread_count;
    long long inicio;
    long long final;

    if (mi_rango < resto) {
        inicio = mi_rango * base + mi_rango;
        final = inicio + base + 1;
    } else {
        inicio = mi_rango * base + resto;
        final = inicio + base;
    }

    long long histograma_local[CATEGORIAS];
    for (int c = 0; c < CATEGORIAS; c++) {
        histograma_local[c] = 0;
    }

    for (long long i = inicio; i < final; i++) {
        int categoria = datos[i] / 10;
        histograma_local[categoria]++;
    }

    pthread_mutex_lock(&mutex_histograma);
    for (int c = 0; c < CATEGORIAS; c++) {
        histograma_priv[c] += histograma_local[c];
    }
    pthread_mutex_unlock(&mutex_histograma);

    return NULL;
}

long long sumar_categorias(long long* hist) {
    long long total = 0;
    for (int c = 0; c < CATEGORIAS; c++) {
        total += hist[c];
    }
    return total;
}

int histogramas_iguales(long long* a, long long* b) {
    for (int c = 0; c < CATEGORIAS; c++) {
        if (a[c] != b[c]) return 0;
    }
    return 1;
}

void imprimir_histograma(const char* titulo, long long* hist) {
    printf("%s\n", titulo);
    for (int c = 0; c < CATEGORIAS; c++) {
        printf("  %2d-%2d: %lld\n", c * 10, c * 10 + 9, hist[c]);
    }
}

int main(int argc, char* argv[]) {
    struct timespec inicio_tiempo, final_tiempo;
    double tiempo_secuencial;
    double tiempo_paralela;
    double tiempo_privado;

    if (argc > 1) n = strtoll(argv[1], NULL, 10);
    if (argc > 2) thread_count = strtol(argv[2], NULL, 10);

    datos = malloc(n * sizeof(int));
    if (datos == NULL) {
        fprintf(stderr, "malloc fallo\n");
        return 1;
    }

    srand(42);
    for (long long i = 0; i < n; i++) {
        datos[i] = rand() % 100;
    }

    // HISTOGRAMA SECUENCIAL
    clock_gettime(CLOCK_MONOTONIC, &inicio_tiempo);
    calcular_histograma_secuencial(datos, n, histograma_sec);
    clock_gettime(CLOCK_MONOTONIC, &final_tiempo);
    tiempo_secuencial = (final_tiempo.tv_sec - inicio_tiempo.tv_sec)
                      + (final_tiempo.tv_nsec - inicio_tiempo.tv_nsec) / 1e9;

    printf("N: %lld | Hilos: %d\n", n, thread_count);
    imprimir_histograma("SEC - Histograma:", histograma_sec);

    long long total = sumar_categorias(histograma_sec);
    printf("Suma de categorias: %lld (N = %lld) -> %s\n",
           total, n, total == n ? "OK" : "ERROR");
    printf("Tiempo Secuencial: %.6f s\n", tiempo_secuencial);

    // HISTOGRAMA PARALELO
    for (int c = 0; c < CATEGORIAS; c++) {
        histograma_par[c] = 0;
    }

    pthread_t* hilos = malloc(thread_count * sizeof(pthread_t)); //espacio para identificadores de hilos

    pthread_mutex_init(&mutex_histograma, NULL);
    clock_gettime(CLOCK_MONOTONIC, &inicio_tiempo);
    for (long i = 0; i < thread_count; i++) {
        pthread_create(&hilos[i], NULL, calcular_histograma_paralelo, (void*)i);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(hilos[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &final_tiempo);
    tiempo_paralela = (final_tiempo.tv_sec - inicio_tiempo.tv_sec)
                    + (final_tiempo.tv_nsec - inicio_tiempo.tv_nsec) / 1e9;

    imprimir_histograma("PAR - Histograma:", histograma_par);
    long long total_par = sumar_categorias(histograma_par);
    printf("Suma de categorias (PAR): %lld (N = %lld) -> %s\n",
           total_par, n, total_par == n ? "OK" : "ERROR");
    printf("Histograma PAR igual a SEC: %s\n",
           histogramas_iguales(histograma_sec, histograma_par) ? "SI" : "NO");
    printf("Tiempo Paralelo: %.6f s\n", tiempo_paralela);
    
    // HISTOGRAMA PRIVADO
    for (int c = 0; c < CATEGORIAS; c++) {
        histograma_priv[c] = 0;
    }

    clock_gettime(CLOCK_MONOTONIC, &inicio_tiempo);
    for (long i = 0; i < thread_count; i++) {
        pthread_create(&hilos[i], NULL, calcular_histograma_privado, (void*)i);
    }
    for (int i = 0; i < thread_count; i++) {
        pthread_join(hilos[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &final_tiempo);
    tiempo_privado = (final_tiempo.tv_sec - inicio_tiempo.tv_sec)
                   + (final_tiempo.tv_nsec - inicio_tiempo.tv_nsec) / 1e9;

    imprimir_histograma("PRIV - Histograma:", histograma_priv);
    long long total_priv = sumar_categorias(histograma_priv);
    printf("Suma de categorias (PRIV): %lld (N = %lld) -> %s\n",
           total_priv, n, total_priv == n ? "OK" : "ERROR");
    printf("Histograma PRIV igual a SEC: %s\n",
           histogramas_iguales(histograma_sec, histograma_priv) ? "SI" : "NO");
    printf("Tiempo Privado: %.6f s\n", tiempo_privado);

    pthread_mutex_destroy(&mutex_histograma);

    int ok_par  = (total_par == n)  && histogramas_iguales(histograma_sec, histograma_par);
    int ok_priv = (total_priv == n) && histogramas_iguales(histograma_sec, histograma_priv);

    printf("CSV,%lld,%d,%.6f,%.6f,%.6f,%d,%d\n",
           n, thread_count, tiempo_secuencial, tiempo_paralela, tiempo_privado,
           ok_par, ok_priv);

    free(hilos);
    free(datos);
    return 0;
}