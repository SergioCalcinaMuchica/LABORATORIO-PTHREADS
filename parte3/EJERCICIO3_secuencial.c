#include <stdio.h>
#include <stdlib.h>

// Versión secuencial base de la Parte III.
// Un solo thread (main) hace de productor y de consumidor por turnos:
// llena el buffer hasta que esté lleno y luego lo vacía, repitiendo
// hasta completar el total de elementos.

// Buffer circular
int* buffer;
int capacidad = 8;
int indice_insercion = 0;   // donde se inserta el próximo elemento
int indice_extraccion = 0;  // de donde se extrae el próximo elemento
int items_por_productor = 1000;
int cantd_productores = 2;  // solo se usa para calcular el total

// Contadores globales
int actual_cantidad = 0;    // elementos que hay en el buffer en este momento
long long total_producidos = 0;
long long total_consumidos = 0;

void producir(int dato){
    buffer[indice_insercion] = dato;
    if(indice_insercion==capacidad-1){
        indice_insercion=0;
    }else{
        indice_insercion++;
    }
    actual_cantidad++;
    total_producidos++;
}

int consumir(){
    int dato = buffer[indice_extraccion];
    if(indice_extraccion==capacidad-1){
        indice_extraccion=0;
    }else{
        indice_extraccion++;
    }
    actual_cantidad--;
    total_consumidos++;
    return dato;
}

int main(int argc, char* argv[]) {
    long total = (long)cantd_productores * items_por_productor;

    buffer = malloc(capacidad * sizeof(int));
    if (buffer == NULL) {
        fprintf(stderr, "malloc fallo\n");
        return 1;
    }

    long siguiente = 0;  // próximo valor a producir
    while(total_consumidos < total){
        // Fase de producción: llenar el buffer mientras haya espacio y queden elementos
        while(actual_cantidad < capacidad && siguiente < total){
            producir(siguiente);
            siguiente++;
        }
        // Fase de consumo: vaciar el buffer
        while(actual_cantidad > 0){
            consumir();
        }
    }

    printf("Producidos: %lld, consumidos: %lld\n", total_producidos, total_consumidos);

    free(buffer);
    return 0;
}