#include "producer.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>      // O_CREAT, O_RDWR
#include <sys/mman.h>   // shm_open, mmap
#include <sys/stat.h>   // mode constants

int main(void) {
    // Semáforos nombrados: vacio (espacios libres) y lleno (espacios ocupados)
    sem_t *vacio = sem_open("/vacio", O_CREAT, 0644, BUFFER);
    sem_t *lleno = sem_open("/lleno", O_CREAT, 0644, 0);
    if (vacio == SEM_FAILED || lleno == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Memoria compartida para el buffer circular
    int shm_fd = shm_open("/memoria_compartida", O_CREAT | O_RDWR, 0644);
    if (shm_fd < 0) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, sizeof(compartir_datos)) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    compartir_datos *compartir = mmap(NULL, sizeof(compartir_datos),
                                      PROT_READ | PROT_WRITE,
                                      MAP_SHARED,
                                      shm_fd,
                                      0);
    if (compartir == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    compartir->entrada = 0;

    for (int i = 1; i <= 10; i++) {
        // Esperar a que haya al menos un espacio vacío
        if (sem_wait(vacio) == -1) {
            perror("sem_wait vacio");
            exit(EXIT_FAILURE);
        }

        // Escribir en el buffer circular
        compartir->bus[compartir->entrada] = i;
        printf("Productor: Produce %d\n", i);

        compartir->entrada = (compartir->entrada + 1) % BUFFER;

        // Señalar que hay un elemento lleno
        if (sem_post(lleno) == -1) {
            perror("sem_post lleno");
            exit(EXIT_FAILURE);
        }

        sleep(1);
    }

    // Liberar recursos locales (pero NO borrar aún los nombres globales)
    if (munmap(compartir, sizeof(compartir_datos)) == -1) {
        perror("munmap");
    }

    close(shm_fd);
    sem_close(vacio);
    sem_close(lleno);

    return 0;
}
