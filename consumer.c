#include "consumer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>      // O_CREAT, O_RDWR
#include <sys/mman.h>   // shm_open, mmap
#include <sys/stat.h>   // mode constants

int main(void) {
    // Abrir los mismos semáforos nombrados creados por el productor
    sem_t *vacio = sem_open("/vacio", 0);
    sem_t *lleno = sem_open("/lleno", 0);
    if (vacio == SEM_FAILED || lleno == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Abrir la misma memoria compartida
    int fd_compartido = shm_open("/memoria_compartida", O_RDWR, 0644);
    if (fd_compartido < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    compartir_datos *compartir = mmap(NULL, sizeof(compartir_datos),
                                      PROT_READ | PROT_WRITE,
                                      MAP_SHARED,
                                      fd_compartido,
                                      0);
    if (compartir == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    compartir->salida = 0;

    for (int i = 1; i <= 10; i++) {
        // Esperar a que haya al menos un elemento lleno
        if (sem_wait(lleno) == -1) {
            perror("sem_wait lleno");
            exit(EXIT_FAILURE);
        }

        int item = compartir->bus[compartir->salida];
        printf("Consumidor: Consume %d\n", item);

        compartir->salida = (compartir->salida + 1) % BUFFER;

        // Indicar que se liberó un espacio vacío
        if (sem_post(vacio) == -1) {
            perror("sem_post vacio");
            exit(EXIT_FAILURE);
        }

        sleep(2);
    }

    // Liberar recursos locales
    if (munmap(compartir, sizeof(compartir_datos)) == -1) {
        perror("munmap");
    }

    close(fd_compartido);
    sem_close(vacio);
    sem_close(lleno);

    // El consumidor actúa como "último" y borra los nombres del sistema
    sem_unlink("/vacio");
    sem_unlink("/lleno");
    shm_unlink("/memoria_compartida");

    return 0;
}
