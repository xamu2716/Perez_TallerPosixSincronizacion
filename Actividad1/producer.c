/********************************************************
*           Pontificia Universidad Javeriana            *
*                                                       *
*   Autor: David Esteban Beltrán Gómez                  *
*   Fecha: 13 Noviembre 2025                            *
*   Materia: Sistemas Operativos                        *
*   Tema: OpenMP                                        *
*   Descripción:                                        *
*	    - Se crea el fichero que realizara las          *
*        funciones de productor en el problema          *
*        productor-consumidor utilizando memoria        *
*        compartida y semáforos para la sincronización. *
*********************************************************/
#define _POSIX_C_SOURCE 200809L
#include "buffer_shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>      // O_CREAT, O_EXCL, O_RDWR
#include <sys/mman.h>   // shm_open, mmap
#include <sys/stat.h>   // Constantes de permisos
#include <errno.h>
#include <string.h>
#include <signal.h>

// Variables globales para controlar los recursos
static sem_t *vacio = SEM_FAILED;
static sem_t *lleno = SEM_FAILED;
static int shm_fd = -1;
static compartir_datos *compartir = MAP_FAILED;
static int created_shm = 0;     // Indica si este proceso creó la memoria
static int created_vacio = 0;   // Indica si este proceso creó el semáforo "vacio"
static int created_lleno = 0;   // Indica si este proceso creó el semáforo "lleno"

/**
 * Manejador de señal SIGINT
 * Si el proceso productor se interrumpe, este manejador asegura
 * que los recursos abiertos se cierren correctamente, sin eliminarlos
 * del sistema.
 */
void handle_sigint(int sig) {
    (void)sig;
    if (compartir != MAP_FAILED) munmap(compartir, sizeof(compartir_datos));
    if (shm_fd != -1) close(shm_fd);
    if (vacio != SEM_FAILED) sem_close(vacio);
    if (lleno != SEM_FAILED) sem_close(lleno);
    fprintf(stderr, "\nProductor: interrumpido. Recursos cerrados localmente.\n");
    exit(EXIT_FAILURE);
}

int main(void) {
    // Captura Ctrl + C para limpiar correctamente
    signal(SIGINT, handle_sigint);

    /* Intenta crear los semáforos con O_CREAT | O_EXCL.
     * Si ya existen, simplemente se abren sin crearlos de nuevo.
     */
    vacio = sem_open(SEM_VACIO_NAME, O_CREAT | O_EXCL, 0666, BUFFER);
    // Si falla, verifica si es porque ya existe
    if (vacio == SEM_FAILED) {
        // Verifica si el error es EEXIST
        if (errno == EEXIST) {
            vacio = sem_open(SEM_VACIO_NAME, 0);
            // Verifica si la apertura fue exitosa
            if (vacio == SEM_FAILED) {
                perror("sem_open (vacio) apertura existente");
                exit(EXIT_FAILURE);
            }
        } else {
            perror("sem_open (vacio)");
            exit(EXIT_FAILURE);
        }
    } else {
        created_vacio = 1;
    }

    // Crear o abrir el semáforo "lleno"
    lleno = sem_open(SEM_LLENO_NAME, O_CREAT | O_EXCL, 0666, 0);
    // Si falla, verifica si es porque ya existe
    if (lleno == SEM_FAILED) {
        // Verifica si el error es EEXIST
        if (errno == EEXIST) {
            lleno = sem_open(SEM_LLENO_NAME, 0);
            // Verifica si la apertura fue exitosa
            if (lleno == SEM_FAILED) {
                perror("sem_open (lleno) apertura existente");
                sem_close(vacio);
                exit(EXIT_FAILURE);
            }
        } else {
            perror("sem_open (lleno)");
            sem_close(vacio);
            exit(EXIT_FAILURE);
        }
    } else {
        created_lleno = 1;
    }

    /* Intentar crear la memoria compartida. 
     * Si ya existe (EEXIST), se abre la existente para no sobrescribir.
     */
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0666);
    // Verificar si la creación fue exitosa
    if (shm_fd >= 0) {
        created_shm = 1;
        // Se define el tamaño necesario para el buffer compartido
        if (ftruncate(shm_fd, sizeof(compartir_datos)) == -1) {
            perror("ftruncate");
            sem_close(vacio); sem_close(lleno);
            // Verificar si se crearon los recursos para eliminarlos
            if (created_vacio)
                sem_unlink(SEM_VACIO_NAME);

            // Verificar si se creó el semáforo "lleno"
            if (created_lleno)
                sem_unlink(SEM_LLENO_NAME);
            shm_unlink(SHM_NAME);
            exit(EXIT_FAILURE);
        }
    } else if (errno == EEXIST) {
        // Ya existe, se abre para escritura
        shm_fd = shm_open(SHM_NAME, O_RDWR, 0);
        if (shm_fd < 0) {
            perror("shm_open abrir existente");
            sem_close(vacio); sem_close(lleno);
            exit(EXIT_FAILURE);
        }
    } else {
        perror("shm_open");
        sem_close(vacio); sem_close(lleno);
        exit(EXIT_FAILURE);
    }

    // Mapear la memoria compartida
    compartir = mmap(NULL, sizeof(compartir_datos), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    // Verificar si el mapeo fue exitoso
    if (compartir == MAP_FAILED) {
        perror("mmap");
        sem_close(vacio);
        sem_close(lleno);
        // Verificar si se crearon los recursos para eliminarlos
        if (created_shm)
            shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    // Inicializar el buffer si el productor fue quien creó la memoria
    if (created_shm) {
        compartir->entrada = 0;
        compartir->salida = 0;
        // Inicializar el buffer a cero
        for (int i = 0; i < BUFFER; ++i)
            compartir->bus[i] = 0;
    }

    // Bucle principal del productor
    for (int i = 1; i <= 10; i++) {
        // Espera hasta que haya un espacio vacío disponible
        if (sem_wait(vacio) == -1) {
            perror("sem_wait vacio");
            break;
        }

        // Escribe un nuevo valor en el buffer
        compartir->bus[compartir->entrada] = i;
        printf("Productor: Produce %d (índice %d)\n", i, compartir->entrada);

        // Avanza circularmente el índice de escritura
        compartir->entrada = (compartir->entrada + 1) % BUFFER;

        // Señala que hay un nuevo elemento lleno
        if (sem_post(lleno) == -1) {
            perror("sem_post lleno");
            break;
        }

        sleep(1); // Simula tiempo de producción
    }

    // Liberación de recursos locales
    if (munmap(compartir, sizeof(compartir_datos)) == -1)
        perror("munmap");

    // Cerrar el descriptor de memoria compartida
    if (close(shm_fd) == -1)
        perror("close shm_fd");
    sem_close(vacio);
    sem_close(lleno);

    /* No se hace unlink aquí porque otros procesos pueden seguir usando los recursos compartidos. 
     * El unlink se deja para el último proceso que termine.
     */
    printf("Productor: terminado correctamente.\n");
    return 0;
}
