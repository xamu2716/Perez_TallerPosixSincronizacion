##############################################
#		Pontificia Universidad Javeriana
# Autores: David Beltran Gomez, Xamuel Pérez Hernández, Juan Gomez Lopez
# Materia: Sistemas Operativos
# Taller 03 - Actividad 1: Sincro POSIX (Semáforos)
# Fichero: Automaticación Compilación
####################################

GCC   = gcc
FLAGS = -pthread -lrt
PROGRAMAS = producer consumer

all: $(PROGRAMAS)

producer:
	$(GCC) $@.c -o $@ $(FLAGS)

consumer:
	$(GCC) $@.c -o $@ $(FLAGS)

clean:
	$(RM) $(PROGRAMAS)