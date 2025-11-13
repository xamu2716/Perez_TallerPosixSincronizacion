GCC   = gcc
FLAGS = -pthread

PROGRAMAS = posixSincro producer consumer

all: $(PROGRAMAS)

posixSincro:
	$(GCC) $@.c -o $@ $(FLAGS)

producer:
	$(GCC) $@.c -o $@ $(FLAGS)

consumer:
	$(GCC) $@.c -o $@ $(FLAGS)

clean:
	$(RM) $(PROGRAMAS)
