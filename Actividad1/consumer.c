/********************************************************
*           Pontificia Universidad Javeriana            *
*                                                       *
*   Autor: Xamuel Perez Madrigal                        *
*   Fecha: 13 Noviembre 2025                            *
*   Materia: Sistemas Operativos                        *
*   Tema: OpenMP                                        *
*   Descripción:                                        *
*	    - Se crea el fichero que realizara las          *
*        funciones de consumidor en el problema         *
*        productor-consumidor utilizando memoria        *
*        compartida y semáforos para la sincronización. *
*********************************************************/
#define _POSIX_C_SOURCE 200809L
#include "buffer_shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>      // Para usar O_RDWR
#include <sys/mman.h>   // Para shm_open y mmap
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>

// Variables globales estáticas para manejar los recursos compartidos
static sem_t *vacio = SEM_FAILED;
static sem_t *lleno = SEM_FAILED;
static int shm_fd = -1;
static compartir_datos *compartir = MAP_FAILED;

/**
 * Manejador de señal SIGINT 
 * Cierra correctamente los recursos en caso de que el proceso consumidor
 * sea interrumpido por el usuario. Evita fugas de memoria o semáforos abiertos.
 */
void handle_sigint(int sig) {
    (void)sig;
    if (compartir != MAP_FAILED) munmap(compartir, sizeof(compartir_datos));
    if (shm_fd != -1) close(shm_fd);
    if (vacio != SEM_FAILED) sem_close(vacio);
    if (lleno != SEM_FAILED) sem_close(lleno);
    fprintf(stderr, "\nConsumidor: interrumpido. Recursos cerrados localmente.\n");
    exit(EXIT_FAILURE);
}

int main(void) {
    // Se asocia el manejador de señal para cerrar recursos si se interrumpe el programa
    signal(SIGINT, handle_sigint);

    // Abrir semáforos existentes (Conectados por el productor)
    vacio = sem_open(SEM_VACIO_NAME, 0);
    lleno = sem_open(SEM_LLENO_NAME, 0);
    if (vacio == SEM_FAILED || lleno == SEM_FAILED) {
        perror("sem_open (consumer)");
        exit(EXIT_FAILURE);
    }

    // Abrir la memoria compartida ya creada por el productor
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
    if (shm_fd < 0) {
        perror("shm_open (consumer)");
        sem_close(vacio); sem_close(lleno);
        exit(EXIT_FAILURE);
    }

    // Se mapea la memoria compartida en el espacio de direcciones del proceso
    compartir = mmap(NULL, sizeof(compartir_datos), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (compartir == MAP_FAILED) {
        perror("mmap (consumer)");
        close(shm_fd);
        sem_close(vacio); sem_close(lleno);
        exit(EXIT_FAILURE);
    }

     /* No se reinicia el índice de salida aquí porque podría sobrescribir
      * datos ya generados por el productor.
      */

    // Bucle principal de consumo (consume 10 elementos)
    for (int i = 1; i <= 10; i++) {
        // Espera hasta que haya un elemento lleno disponible
        if (sem_wait(lleno) == -1) {
            perror("sem_wait lleno (consumer)");
            break;
        }

        // Leer el elemento del buffer circular
        int item = compartir->bus[compartir->salida];
        printf("Consumidor: Consume %d (índice %d)\n", item, compartir->salida);

        // Avanza el puntero de salida circularmente
        compartir->salida = (compartir->salida + 1) % BUFFER;

        // Libera un espacio vacío en el buffer
        if (sem_post(vacio) == -1) {
            perror("sem_post vacio (consumer)");
            break;
        }

        sleep(2); // Simula el tiempo de procesamiento
    }

    // Liberar la memoria compartida y cerrar el descriptor
    if (munmap(compartir, sizeof(compartir_datos)) == -1)
        perror("munmap");

    // Cerrar el descriptor de memoria compartida
    if (close(shm_fd) == -1)
        perror("close shm_fd");

    // Cerrar los semáforos locales
    sem_close(vacio);
    sem_close(lleno);

    /* El consumidor elimina los nombres del sistema ya que normalmente
     * es el último en ejecutarse.
     */

    // Eliminar los semáforos y la memoria compartida del sistema
    if (sem_unlink(SEM_VACIO_NAME) == -1 && errno != ENOENT)
        perror("sem_unlink vacio");
    
    // Eliminar el semáforo lleno
    if (sem_unlink(SEM_LLENO_NAME) == -1 && errno != ENOENT)
        perror("sem_unlink lleno");

    // Eliminar la memoria compartida
    if (shm_unlink(SHM_NAME) == -1 && errno != ENOENT)
        perror("shm_unlink");

    printf("Consumidor: terminado y recursos liberados.\n");
    return 0;
}
