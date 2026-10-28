#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct {
    long long suma;
    int min_val;
    int max_val;
    double promedio;
} Resultados;

int* datos;
long long n = 1000;
int thread_count = 4;
Resultados resultados_paralela;
pthread_mutex_t mutex;

void calcular_secuencial(int* datos, long long cantidad_datos, Resultados* res) {
    res->suma = 0;
    res->min_val = INT_MAX;
    res->max_val = INT_MIN;

    for (long long i = 0; i < cantidad_datos; i++) {
        res->suma += datos[i];
        if (datos[i] < res->min_val) res->min_val = datos[i];
        if (datos[i] > res->max_val) res->max_val = datos[i];
    }

    res->promedio = (double)res->suma / cantidad_datos;
}

void* calcular_paralelo(void* rango){
    long mi_rango = (long)rango;
    long long base = n/thread_count;
    long long resto = n % thread_count;
    long long inicio;
    long long final;

    if(mi_rango<resto){
        inicio = mi_rango*base + mi_rango;
        final = inicio+base+1;
    }else{
        inicio = mi_rango*base + resto;
        final = inicio+base; 
    }

    long long suma_local = 0;
    int min_local = INT_MAX;
    int max_local = INT_MIN;

    for(long long i=inicio;i<final;i++){
        suma_local+=datos[i];
        if(datos[i]<min_local){
            min_local = datos[i];
        }
        if(datos[i]>max_local){
            max_local = datos[i];
        }
    }

    pthread_mutex_lock(&mutex);
    resultados_paralela.suma += suma_local;
    if (min_local < resultados_paralela.min_val){
        resultados_paralela.min_val = min_local;
    }
    if (max_local > resultados_paralela.max_val){
        resultados_paralela.max_val = max_local;
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main(int argc, char* argv[]) {
    struct timespec inicio_tiempo, final_tiempo;
    double tiempo_secuencial, tiempo_paralela;
    if (argc > 1) n = strtoll(argv[1], NULL, 10);
    if (argc > 2) thread_count = strtol(argv[2], NULL, 10);
    srand(42);
    datos = malloc(n * sizeof(int));

    for (long long i = 0; i < n; i++) {
        datos[i] = rand() % 10000; 
    }

    Resultados resultados_sec;
    clock_gettime(CLOCK_MONOTONIC, &inicio_tiempo);
    calcular_secuencial(datos, n, &resultados_sec);
    clock_gettime(CLOCK_MONOTONIC, &final_tiempo);
    tiempo_secuencial = (final_tiempo.tv_sec - inicio_tiempo.tv_sec) + (final_tiempo.tv_nsec - inicio_tiempo.tv_nsec) / 1e9;

    resultados_paralela.suma = 0;
    resultados_paralela.min_val = INT_MAX;
    resultados_paralela.max_val = INT_MIN;
    pthread_mutex_init(&mutex,NULL);

    pthread_t* hilos = malloc(thread_count * sizeof(pthread_t));
    
    clock_gettime(CLOCK_MONOTONIC, &inicio_tiempo);
    for(long i=0;i<thread_count;i++){
        pthread_create(&hilos[i],NULL,calcular_paralelo,(void*)i);
    }

    for(int i=0;i<thread_count;i++){
        pthread_join(hilos[i],NULL);
    }

    resultados_paralela.promedio = (double)resultados_paralela.suma/n;
    clock_gettime(CLOCK_MONOTONIC, &final_tiempo);
    tiempo_paralela = (final_tiempo.tv_sec - inicio_tiempo.tv_sec) + (final_tiempo.tv_nsec - inicio_tiempo.tv_nsec) / 1e9;


    bool ok = (resultados_sec.suma == resultados_paralela.suma &&
          resultados_sec.min_val == resultados_paralela.min_val &&
          resultados_sec.max_val == resultados_paralela.max_val);
    if(ok==true){
        printf("Ambos metodos dieron los mismo resultados\n");
    }else{
        printf("Ambos metodos NO dieron los mismo resultados\n");
    }

    double speedup = tiempo_secuencial/tiempo_paralela;
    double eficiencia = speedup/thread_count;
    printf("CSV,%lld,%d,%.6f,%.6f,%.4f,%.4f,%d\n",
       n, thread_count, tiempo_secuencial, tiempo_paralela,
       speedup, eficiencia, ok);

    printf("N: %lld | Hilos: %d\n", n, thread_count);
    printf("SEC - Suma: %lld, Min: %d, Max: %d, Prom: %.2f\n", 
           resultados_sec.suma, resultados_sec.min_val, resultados_sec.max_val, resultados_sec.promedio);
    printf("PAR - Suma: %lld, Min: %d, Max: %d, Prom: %.2f\n", 
           resultados_paralela.suma, resultados_paralela.min_val, resultados_paralela.max_val, resultados_paralela.promedio);
    
    printf("-------------TIEMPO-------------\n");
    printf("Tiempo Secuencial: %.6f s\n", tiempo_secuencial);
    printf("Tiempo Paralelo: %.6f s\n", tiempo_paralela);
    printf("Speedup (Aceleración): %.4f\n", speedup);
    printf("Eficiencia: %.4f\n", eficiencia);
    pthread_mutex_destroy(&mutex);
    free(hilos);
    free(datos);
    return 0;
}