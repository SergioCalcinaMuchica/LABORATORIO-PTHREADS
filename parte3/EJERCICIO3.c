#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

// Buffer circular
int* buffer;
int capacidad = 8;
int indice_insercion = 0;   // donde el productor inserta el proximo elemento
int indice_extraccion = 0;  // de donde el consumidor extrae el proximo elemento
int items_por_productor = 1000;

// Contadores globales
int actual_cantidad = 0;
long long total_producidos = 0;
long long total_consumidos = 0;

int cantd_productores = 2;
int cantd_consumidores = 5;
pthread_mutex_t mutex_buffer;

sem_t elemento_vacio;
sem_t elemento_disponible;
int* verificador_datos;

void* productor(void* rango){
    long mi_rango = (long)rango;
    for(int i=mi_rango*items_por_productor;i<items_por_productor*(mi_rango+1);i++){
        sem_wait(&elemento_vacio);
        pthread_mutex_lock(&mutex_buffer);
        total_producidos++;
        actual_cantidad++;
        buffer[indice_insercion] = i;
        if(indice_insercion==capacidad-1){
            indice_insercion=0;
        }else{
            indice_insercion++;
        }
        pthread_mutex_unlock(&mutex_buffer);
        sem_post(&elemento_disponible);
    }
    return NULL;
}

void* consumidor(void* rango){
    long mi_rango = (long) rango;
    long total = items_por_productor * cantd_productores;
    long residuo = total % cantd_consumidores;
    long base = total/cantd_consumidores;

    long iterador;
    if(mi_rango<residuo){
        iterador = base+1;
    }else{
        iterador = base;
    }

    int dato = 0;
    for(int i=0;i<iterador;i++){
        sem_wait(&elemento_disponible);
        pthread_mutex_lock(&mutex_buffer);
        total_consumidos++;
        actual_cantidad--;
        dato = buffer[indice_extraccion];
        verificador_datos[dato]+=1;
        if(indice_extraccion==capacidad-1){
            indice_extraccion=0;
        }else{
            indice_extraccion++;
        }
        pthread_mutex_unlock(&mutex_buffer);
        sem_post(&elemento_vacio);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc > 1) cantd_productores = strtol(argv[1], NULL, 10);
    if (argc > 2) cantd_consumidores = strtol(argv[2], NULL, 10);
    if (argc > 3) capacidad = strtol(argv[3], NULL, 10);
    if (argc > 4) items_por_productor = strtol(argv[4], NULL, 10);

    struct timespec inicio_tiempo, final_tiempo;
    double tiempo_ejecucion;

    buffer = malloc(capacidad * sizeof(int));
    if (buffer == NULL) {
        fprintf(stderr, "malloc fallo\n");
        return 1;
    }

    verificador_datos = calloc(cantd_productores*items_por_productor,sizeof(int));
    if(verificador_datos==NULL){
        fprintf(stderr, "calloc fallo\n");
        return 1;
    }

    pthread_t* productores = malloc(cantd_productores * sizeof(pthread_t));
    if (productores == NULL) {
        fprintf(stderr, "malloc fallo\n");
        return 1;
    }

    pthread_t* consumidores = malloc(cantd_consumidores * sizeof(pthread_t));
    if (consumidores == NULL) {
        fprintf(stderr, "malloc fallo\n");
        return 1;
    }
    sem_init(&elemento_vacio,0,capacidad);
    sem_init(&elemento_disponible,0,0);

    pthread_mutex_init(&mutex_buffer,NULL);
    clock_gettime(CLOCK_MONOTONIC, &inicio_tiempo);
    for(long i=0;i<cantd_productores;i++){
        pthread_create(&productores[i], NULL, productor, (void*)i);
    }

    for(long i=0;i<cantd_consumidores;i++){
        pthread_create(&consumidores[i], NULL, consumidor, (void*)i);
    }

    for(long i=0;i<cantd_productores;i++){
        pthread_join(productores[i], NULL);
    }

    for(long i=0;i<cantd_consumidores;i++){
        pthread_join(consumidores[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &final_tiempo);
    pthread_mutex_destroy(&mutex_buffer);

    tiempo_ejecucion = (final_tiempo.tv_sec - inicio_tiempo.tv_sec) * 1000.0
                 + (final_tiempo.tv_nsec - inicio_tiempo.tv_nsec) / 1e6;

    if(total_consumidos==total_producidos){
        printf("CORRECTO, el total de consumidos es igual al de producidos\n");
    }else{
        printf("ERROR, el total de consumidos NO es igual al de producidos\n");
    }

    //VERIFIFCAR DUPLICADOS O FALTANTES
    int duplicados=0;
    int faltantes=0;
    for(int i=0;i<cantd_productores*items_por_productor;i++){
        if(verificador_datos[i]==0){
            faltantes++;
        }
        if(verificador_datos[i]>1){
            duplicados++;
        }
    }
    printf("VERIFICACION\nElementos duplicados: %d\nElementos faltantes: %d\n",duplicados,faltantes);

    printf("CSV,%d,%d,%d,%d,%.3f,%lld,%lld,%d,%d\n",
       cantd_productores, cantd_consumidores, capacidad, items_por_productor,
       tiempo_ejecucion, total_producidos, total_consumidos, duplicados, faltantes);

    sem_destroy(&elemento_vacio);
    sem_destroy(&elemento_disponible);
    free(verificador_datos);
    free(productores);
    free(consumidores);
    free(buffer);
    return 0;
}