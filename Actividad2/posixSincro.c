/********************************************************
*           Pontificia Universidad Javeriana            *
*                                                       *
*   Autor: David Esteban Beltrán Gómez                  *
*   Fecha: 13 Noviembre 2025                            *
*   Materia: Sistemas Operativos                        *
*   Tema: OpenMP                                        *
*   Descripción:                                        *
*	    - Se crea el fichero que realizara las          *
*        funciones de productor-consumidor utilizando   *
*        hilos POSIX, mutex y variables de condición    *
*********************************************************/
#include "posixSincro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

char buf[MAX_BUFFERS][100]; // Arreglo de buffers para almacenar las líneas
int buffer_index;
int buffer_print_index;

pthread_mutex_t buf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  buf_cond  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  spool_cond = PTHREAD_COND_INITIALIZER;
int buffers_available = MAX_BUFFERS; // Cantidad de buffers libres
int lines_to_print = 0;              // Cantidad de líneas pendientes

int main(int argc, char **argv) {
    pthread_t tid_producer[10], tid_spooler;
    int i, r;
    int thread_no[10]; // Guarda el ID de cada hilo productor

    buffer_index = 0;
    buffer_print_index = 0;

    // Crear el hilo spooler
    if ((r = pthread_create(&tid_spooler, NULL, spooler, NULL)) != 0) {
        fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
        exit(1);
    }

    // Crear 10 hilos productores
    for (i = 0; i < 10; i++) {
        thread_no[i] = i;
        // Crear el hilo productor
        if ((r = pthread_create(&tid_producer[i], NULL, producer, (void *)&thread_no[i])) != 0) {
            fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
            exit(1);
        }
    }

    // Esperar a que todos los productores terminen
    for (i = 0; i < 10; i++) {
        // Esperar al hilo productor
        if ((r = pthread_join(tid_producer[i], NULL)) != 0) {
            fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
            exit(1);
        }
    }

    // Esperar a que el spooler imprima todo lo que queda
    while (lines_to_print)
        sleep(1);

    // Cancelar el hilo spooler después de imprimir todo
    if ((r = pthread_cancel(tid_spooler)) != 0) {
        fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
        exit(1);
    }

    return 0;
}

// Hilo productor
void *producer(void *arg) {
    int i, r;
    int my_id = *((int *)arg); // ID del hilo productor
    int count = 0;

    // Producir 10 líneas
    for (i = 0; i < 10; i++) {

        // Bloquear el mutex para proteger el acceso al buffer
        if ((r = pthread_mutex_lock(&buf_mutex)) != 0) {
            fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
            exit(1);
        }

        // Esperar si no hay buffers disponibles
        while (!buffers_available)
            pthread_cond_wait(&buf_cond, &buf_mutex);

        // Escribir una línea en el buffer
        int j = buffer_index;
        buffer_index++;

        // Mover el índice circularmente
        if (buffer_index == MAX_BUFFERS)
            buffer_index = 0;
        buffers_available--;

        sprintf(buf[j], "Thread %d: %d\n", my_id, ++count);
        lines_to_print++;

        // Avisar al spooler que hay algo para imprimir
        pthread_cond_signal(&spool_cond);

        // Liberar el mutex
        if ((r = pthread_mutex_unlock(&buf_mutex)) != 0) {
            fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
            exit(1);
        }

        sleep(1); // Simula trabajo
    }

    return NULL;
}

// Hilo Consumidor (spooler)
void *spooler(void *arg) {
    int r;
    (void)arg;

    while (1) {
        // Bloquear el mutex para acceder al buffer compartido
        if ((r = pthread_mutex_lock(&buf_mutex)) != 0) {
            fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
            exit(1);
        }

        // Esperar hasta que haya líneas para imprimir
        while (!lines_to_print)
            pthread_cond_wait(&spool_cond, &buf_mutex);

        // Imprimir la línea
        printf("%s", buf[buffer_print_index]);
        lines_to_print--;

        // Mover el índice de impresión circularmente
        buffer_print_index++;
        // Volver al inicio si se llega al final
        if (buffer_print_index == MAX_BUFFERS)
            buffer_print_index = 0;

        // Liberar un buffer y avisar a los productores
        buffers_available++;
        pthread_cond_signal(&buf_cond);

        // Desbloquear el mutex
        if ((r = pthread_mutex_unlock(&buf_mutex)) != 0) {
            fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
            exit(1);
        }
    }

    return NULL;
}