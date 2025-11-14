#ifndef BUFFER_SHARED_H
#define BUFFER_SHARED_H

#include <semaphore.h>

#define BUFFER 5
#define SHM_NAME "/memoria_compartida"
#define SEM_VACIO_NAME "/vacio"
#define SEM_LLENO_NAME "/lleno"

/** 
 * Estructura compartida por el productor y consumidor.
 * Aquí se guardan los datos (bus), junto con los índices de entrada y salida
 * que controlan el acceso concurrente al buffer.
 */
typedef struct {
    int bus[BUFFER];
    int entrada;
    int salida;
} compartir_datos;

#endif