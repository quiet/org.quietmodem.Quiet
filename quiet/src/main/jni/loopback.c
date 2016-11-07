#include "quiet-jni.h"

// TODO fix include mess so we can include unistd.h
extern int usleep(useconds_t);

const int loopback_sample_rate = 44100;
const int loopback_sleep = 23220;  // in microseconds
const int loopback_buffer_length = 1024;

quiet_loopback_system *loopback_create() {
    quiet_loopback_system *loopback = calloc(1, sizeof(quiet_loopback_system));

    pthread_mutex_init(&loopback->lock, NULL);

    loopback->producers_cap = 8;
    loopback->consumers_cap = 8;
    loopback->num_producers = 0;
    loopback->num_consumers = 0;

    loopback->producers = calloc(loopback->producers_cap, sizeof(quiet_opensl_producer *));
    loopback->consumers = calloc(loopback->consumers_cap, sizeof(quiet_opensl_consumer *));

    loopback->is_closed = false;

    return loopback;
}

void loopback_close(quiet_loopback_system *loopback) {
    pthread_mutex_lock(&loopback->lock);
    loopback->is_closed = true;
    pthread_mutex_unlock(&loopback->lock);
}

void loopback_destroy(quiet_loopback_system *loopback) {
    pthread_mutex_destroy(&loopback->lock);
    free(loopback->producers);
    free(loopback->consumers);
    free(loopback);
}

static void loopback_add_producer_locked(quiet_loopback_system *loopback, quiet_opensl_producer *p) {
    if (loopback->num_producers == loopback->producers_cap) {
        loopback->producers_cap *= 2;
        loopback->producers = realloc(loopback->producers, loopback->producers_cap * sizeof(quiet_opensl_producer *));
    }
    loopback->producers[loopback->num_producers] = p;
    loopback->num_producers++;
}

static void loopback_remove_producer_locked(quiet_loopback_system *loopback, quiet_opensl_producer *p) {
    for (size_t i = 0; i < loopback->num_producers; i++) {
        if (loopback->producers[i] == p) {
            loopback->producers[i] = NULL;
            for (size_t j = i + 1; j < loopback->num_producers; j++) {
                loopback->producers[j - 1] = loopback->producers[j];
            }
            loopback->num_producers--;
            break;
        }
    }
}

static void loopback_add_consumer_locked(quiet_loopback_system *loopback, quiet_opensl_consumer *c) {
    if (loopback->num_consumers == loopback->consumers_cap) {
        loopback->consumers_cap *= 2;
        loopback->consumers = realloc(loopback->consumers, loopback->consumers_cap * sizeof(quiet_opensl_consumer *));
    }
    loopback->consumers[loopback->num_consumers] = c;
    loopback->num_consumers++;
}

static void loopback_remove_consumer_locked(quiet_loopback_system *loopback, quiet_opensl_consumer *c) {
    for (size_t i = 0; i < loopback->num_consumers; i++) {
        if (loopback->consumers[i] == c) {
            loopback->consumers[i] = NULL;
            for (size_t j = i + 1; j < loopback->num_consumers; j++) {
                loopback->consumers[j - 1] = loopback->consumers[j];
            }
            loopback->num_consumers--;
            break;
        }
    }
}

void loopback_add_producer(quiet_loopback_system *loopback, quiet_opensl_producer *p) {
    pthread_mutex_lock(&loopback->lock);
    loopback_add_producer_locked(loopback, p);
    pthread_mutex_unlock(&loopback->lock);
}

void loopback_remove_producer(quiet_loopback_system *loopback, quiet_opensl_producer *p) {
    pthread_mutex_lock(&loopback->lock);
    loopback_remove_producer_locked(loopback, p);
    pthread_mutex_unlock(&loopback->lock);
}

void loopback_add_consumer(quiet_loopback_system *loopback, quiet_opensl_consumer *c) {
    pthread_mutex_lock(&loopback->lock);
    loopback_add_consumer_locked(loopback, c);
    pthread_mutex_unlock(&loopback->lock);
}

void loopback_remove_consumer(quiet_loopback_system *loopback, quiet_opensl_consumer *c) {
    pthread_mutex_lock(&loopback->lock);
    loopback_remove_consumer_locked(loopback, c);
    pthread_mutex_unlock(&loopback->lock);
}

static void loopback_sum_producer(quiet_loopback_system *l,
                                  quiet_opensl_producer *p,
                                  opensl_sample_t *dest) {
    ssize_t written = p->produce(p->produce_arg, p->scratch, p->num_frames);

    if (written == 0) {
        // EOF - this signals that the producer is done and should be removed
        // we use the locked version of this call since this sum is run
        // with the lock held
        loopback_remove_producer_locked(l, p);
    }

    if (written <= 0) {
        // 0: eof (handled above)
        // <0: no frames ready, but more could be later
        // in either case there's nothing left to do
        return;
    }

    for (size_t i = written; i < p->num_frames; i++) {
        p->scratch[i] = 0;
    }
    size_t num_bytes = p->num_frames * num_playback_channels * sizeof(opensl_sample_t);
    memset(p->buf[0], 0, num_bytes);
    convert_monofloat2stereoopensl(p->scratch, p->buf[0],
                                   p->num_frames);
    for (size_t i = 0; i < p->num_frames * num_playback_channels; i++) {
        dest[i] += p->buf[0][i];
    }
}

static void loopback_write_consumer(quiet_loopback_system *l,
                                    quiet_opensl_consumer *c,
                                    opensl_sample_t *src) {
    convert_stereoopensl2monofloat(src, c->scratch, c->num_frames, num_playback_channels);
    c->consume(c->consume_arg, c->scratch, c->num_frames);
}

static void *loopback_thread(void *args_v) {
    quiet_loopback_system *l = (quiet_loopback_system *)args_v;
    struct timeval now, last_now;
    gettimeofday(&last_now, NULL);
    opensl_sample_t *sample_stereo_buffer = malloc(loopback_buffer_length * num_playback_channels * sizeof(opensl_sample_t));
    while (true) {
        memset(sample_stereo_buffer, 0, loopback_buffer_length * num_playback_channels * sizeof(opensl_sample_t));
        pthread_mutex_lock(&l->lock);

        if (l->is_closed) {
            pthread_mutex_unlock(&l->lock);
            break;
        }
        for (size_t i = 0; i < l->num_producers; i++) {
            loopback_sum_producer(l, l->producers[i], sample_stereo_buffer);
        }

        for (size_t i = 0; i < l->num_consumers; i++) {
            loopback_write_consumer(l, l->consumers[i], sample_stereo_buffer);
        }

        pthread_mutex_unlock(&l->lock);
        gettimeofday(&now, NULL);
        time_t elapsed = (now.tv_sec - last_now.tv_sec) * 1000000L;
        elapsed += (now.tv_usec - last_now.tv_usec);
        int sleep_length = loopback_sleep - elapsed;
        if (sleep_length > 0) {
            usleep(sleep_length);
        }
        // TODO consider else case here - maybe sleep less next time?
        // assuming we are staying caught up at all
        last_now = now;
    }
    free(sample_stereo_buffer);
    pthread_exit(NULL);
}

static void start_loopback_thread(quiet_loopback_system *loopback) {
    pthread_attr_t thread_attr;

    pthread_attr_init(&thread_attr);
    pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_JOINABLE);

    pthread_create(&loopback->thread, &thread_attr, loopback_thread, loopback);

    pthread_attr_destroy(&thread_attr);
}

void stop_loopback_thread(quiet_loopback_system *loopback) {
    loopback_close(loopback);
    pthread_join(loopback->thread, NULL);
}

void quiet_loopback_system_create(quiet_loopback_system **sys_dest) {
    *sys_dest = loopback_create();
    start_loopback_thread(*sys_dest);
}

void quiet_loopback_system_destroy(quiet_loopback_system *sys) {
    stop_loopback_thread(sys);
    loopback_destroy(sys);
}
