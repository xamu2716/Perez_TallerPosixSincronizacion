#ifndef POSIXSINCRO_H
#define POSIXSINCRO_H

#define MAX_BUFFERS 10

// Interfaces (prototipos) de las funciones del archivo posixSincro.c
void *producer(void *arg);
void *spooler(void *arg);

#endif
