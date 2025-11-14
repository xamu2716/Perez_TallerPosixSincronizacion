##############################################
#		Pontificia Universidad Javeriana
# Autores: David Beltran Gomez, Xamuel Pérez Hernández, Juan Gomez Lopez
# Materia: Sistemas Operativos
# Taller 03 - Actividad 2: Sincro POSIX (Threads)
# Fichero: Automaticación Compilación
####################################

GCC   = gcc
FLAGS = -pthread
PROGRAMAS = posixSincro

all: $(PROGRAMAS)

posixSincro:
	$(GCC) $@.c -o $@ $(FLAGS)

clean:
	$(RM) $(PROGRAMAS)
