#ifndef BG_THREADS_
#define BG_THREADS_

#ifdef BB_WIN

#include <glib.h>

typedef GMutex BG_MUTEX_T;
#define BG_MUTEX_INIT(pmutex) g_mutex_init(pmutex)
#define BG_MUTEX_DEINIT(pmutex) g_mutex_clear(pmutex)
#define BG_MUTEX_LOCK(pmutex) g_mutex_lock(pmutex)
#define BG_MUTEX_UNLOCK(pmutex) g_mutex_unlock(pmutex)

typedef GThread * BG_THREAD_T;
// #define BG_THREAD_NEW(f, d) g_thread_new(NULL, f, d);
#define BG_THREAD_JOIN(t) g_thread_join(t)
#else

#include <pthread.h>

typedef pthread_mutex_t BG_MUTEX_T;
#define BG_MUTEX_INIT(pmutex) pthread_mutex_init(pmutex, NULL)
#define BG_MUTEX_DEINIT(pmutex) pthread_mutex_destroy(pmutex)
#define BG_MUTEX_LOCK(pmutex) pthread_mutex_lock(pmutex)
#define BG_MUTEX_UNLOCK(pmutex) pthread_mutex_unlock(pmutex)

typedef pthread_t BG_THREAD_T;
// #define BG_THREAD_NEW(f, d) pthread_create(NULL, f, d);
#define BG_THREAD_JOIN(t) pthread_join(t, NULL)
#endif                                      // BB_WIN

#endif                                      // BG_THREADS_