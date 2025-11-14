#ifndef POSIXSINCRO_H
#define POSIXSINCRO_H

#define MAX_BUFFERS 10  // Tamaño máximo del arreglo de buffers

// Prototipos de las funciones que se definen en posixSincro.c
void *producer(void *arg);
void *spooler(void *arg);

#endif
