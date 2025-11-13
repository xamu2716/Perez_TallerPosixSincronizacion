#ifndef PRODUCER_H
#define PRODUCER_H

#define BUFFER 5

// Estructura del buffer compartido
typedef struct {
    int bus[BUFFER];
    int entrada;
    int salida;
} compartir_datos;

// Prototipo de main 
int main(void);

#endif
