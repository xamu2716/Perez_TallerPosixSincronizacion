#ifndef CONSUMER_H
#define CONSUMER_H

#define BUFFER 5

// Misma estructura del buffer compartido
typedef struct {
    int bus[BUFFER];
    int entrada;
    int salida;
} compartir_datos;

int main(void);

#endif
