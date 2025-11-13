#include "posixSincro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

char buf[MAX_BUFFERS][100];
int buffer_index;
int buffer_print_index;

pthread_mutex_t buf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  buf_cond  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  spool_cond = PTHREAD_COND_INITIALIZER;
int buffers_available = MAX_BUFFERS;
int lines_to_print = 0;

int main(int argc, char **argv) {
    pthread_t tid_producer[10], tid_spooler;
    int i, r;
    int thread_no[10];

    buffer_index = 0;
    buffer_print_index = 0;

    if ((r = pthread_create(&tid_spooler, NULL, spooler, NULL)) != 0) {
        fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
        exit(1);
    }

    for (i = 0; i < 10; i++) {
        thread_no[i] = i;
        if ((r = pthread_create(&tid_producer[i], NULL, producer, (void *)&thread_no[i])) != 0) {
            fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
            exit(1);
        }
    }

    for (i = 0; i < 10; i++) {
        if ((r = pthread_join(tid_producer[i], NULL)) != 0) {
            fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
            exit(1);
        }
    }

    // Dejo que el spooler termine de imprimir lo que queda
    while (lines_to_print)
        sleep(1);

    if ((r = pthread_cancel(tid_spooler)) != 0) {
        fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
        exit(1);
    }

    return 0;
}

void *producer(void *arg) {
    int i, r;
    int my_id = *((int *)arg);
    int count = 0;

    for (i = 0; i < 10; i++) {

        if ((r = pthread_mutex_lock(&buf_mutex)) != 0) {
            fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
            exit(1);
        }

        while (!buffers_available)
            pthread_cond_wait(&buf_cond, &buf_mutex);

        int j = buffer_index;
        buffer_index++;
        if (buffer_index == MAX_BUFFERS)
            buffer_index = 0;
        buffers_available--;

        sprintf(buf[j], "Thread %d: %d\n", my_id, ++count);
        lines_to_print++;

        pthread_cond_signal(&spool_cond);

        if ((r = pthread_mutex_unlock(&buf_mutex)) != 0) {
            fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
            exit(1);
        }

        sleep(1);
    }

    return NULL;
}

void *spooler(void *arg) {
    int r;
    (void)arg;

    while (1) {
        if ((r = pthread_mutex_lock(&buf_mutex)) != 0) {
            fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
            exit(1);
        }

        while (!lines_to_print)
            pthread_cond_wait(&spool_cond, &buf_mutex);

        printf("%s", buf[buffer_print_index]);
        lines_to_print--;

        buffer_print_index++;
        if (buffer_print_index == MAX_BUFFERS)
            buffer_print_index = 0;

        buffers_available++;
        pthread_cond_signal(&buf_cond);

        if ((r = pthread_mutex_unlock(&buf_mutex)) != 0) {
            fprintf(stderr, "Error = %d (%s)\n", r, strerror(r));
            exit(1);
        }
    }

    return NULL;
}
