#include <pthread.h>
#include <stdlib.h>

typedef struct {
    int size;
    int numEmpty;
    int numFull;
    int head;
    int tail;
    pthread_cond_t isEmpty;
    pthread_cond_t isFull;
    pthread_mutex_t mutex;
    void **buckets;
} BoundedBuffer;

void *createBuffer(unsigned int size) {
    // invalid size
    if (size == 0) {
        return NULL;
    }

    BoundedBuffer *b = malloc(sizeof(BoundedBuffer));
    if (b == NULL) {
        return NULL;
    }

    b->buckets = calloc(size, sizeof(void *));
    if (b->buckets == NULL) {
        free(b);
        return NULL;
    }

    b->size = size;
    b->numEmpty = size;
    b->numFull = 0;
    b->head = 0;
    b->tail = 0;
    pthread_cond_init(&b->isEmpty, NULL);
    pthread_cond_init(&b->isFull, NULL);
    pthread_mutex_init(&b->mutex, NULL);

    return b;
}

// producer
void putBuffer(void *handle, void *p) {
    BoundedBuffer *b = handle;
    // wait for there to be space
    // lock buffer and enqueue
    pthread_mutex_lock(&b->mutex);

    while (b->numFull == b->size) {
        pthread_cond_wait(&b->isFull, &b->mutex);
    }

    // enqueue
    b->buckets[b->tail] = p;
    b->tail = (b->tail + 1) % b->size;

    b->numEmpty--;
    b->numFull++;

    pthread_cond_signal(&b->isEmpty);
    pthread_mutex_unlock(&b->mutex);
}

// consumer
void *getBuffer(void *handle) {
    BoundedBuffer *b = handle;
    // wait for a value
    // lock buffer and dequeue
    pthread_mutex_lock(&b->mutex);

    while (b->numEmpty == b->size) {
        pthread_cond_wait(&b->isEmpty, &b->mutex);
    }

    // dequeue
    void *ret = b->buckets[b->head];
    b->head = (b->head + 1) % b->size;

    b->numEmpty++;
    b->numFull--;

    pthread_cond_signal(&b->isFull);
    pthread_mutex_unlock(&b->mutex);
    return ret;
}

void *tryGetBuffer(void *handle) {
    BoundedBuffer *b = handle;
    pthread_mutex_lock(&b->mutex);

    // nothing available
    if (b->numEmpty == b->size) {
        pthread_mutex_unlock(&b->mutex);
        return NULL;
    }

    // dequeue
    void *ret = b->buckets[b->head];
    b->head = (b->head + 1) % b->size;

    b->numEmpty++;
    b->numFull--;
    
    pthread_mutex_unlock(&b->mutex);
    return ret;
}

void deleteBuffer(void *handle) {
    BoundedBuffer *b = handle;
    pthread_cond_destroy(&b->isFull);
    pthread_cond_destroy(&b->isEmpty);
    pthread_mutex_destroy(&b->mutex);
    free(b->buckets);
    free(b);
}
