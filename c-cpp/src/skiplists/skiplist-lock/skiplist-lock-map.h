/*
 * File:
 *   skiplist-lock-map.h
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 *   Wenbin Xiao <github.com/xwb1989>
 * Description:
 *   Skip list implementation of an long-void* map
 */

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

//change it if necessary
#include <../../atomic_ops/atomic_ops.h>

#define DEFAULT_DURATION                10000
#define DEFAULT_INITIAL                 256
#define DEFAULT_NB_THREADS              1
#define DEFAULT_RANGE                   0x7FFFFFFF
#define DEFAULT_SEED                    0
#define DEFAULT_UPDATE                  20
#define DEFAULT_ELASTICITY		4
#define DEFAULT_ALTERNATE               0
#define DEFAULT_EFFECTIVE 		1

#define XSTR(s)                         STR(s)
#define STR(s)                          #s

extern volatile AO_t stop;
extern unsigned int global_seed;
/* Skip list level */
#ifdef TLS
extern __thread unsigned int *rng_seed;
#else /* ! TLS */
extern pthread_key_t rng_seed_key;
#endif /* ! TLS */
extern unsigned int levelmax;

#define TRANSACTIONAL                   d->unit_tx

typedef long sl_key_t;
typedef void*   sl_val_t;

#define VAL_MIN                         LONG_MAX 
#define VAL_MAX                         LONG_MAX 

#ifdef MUTEX
typedef pthread_mutex_t ptlock_t;
#  define INIT_LOCK(lock)		pthread_mutex_init(lock, NULL);
#  define DESTROY_LOCK(lock)		pthread_mutex_destroy(lock)
#  define LOCK(lock)			pthread_mutex_lock(lock)
#  define UNLOCK(lock)			pthread_mutex_unlock(lock)
#else
typedef pthread_spinlock_t ptlock_t;
#  define INIT_LOCK(lock)		pthread_spin_init(lock, PTHREAD_PROCESS_PRIVATE);
#  define DESTROY_LOCK(lock)		pthread_spin_destroy(lock)
#  define LOCK(lock)			pthread_spin_lock(lock)
#  define UNLOCK(lock)			pthread_spin_unlock(lock)
#endif

typedef struct sl_node {
    sl_key_t key;
	sl_val_t val; 
	int toplevel;
	struct sl_node** next;
	volatile int marked;
	volatile int fullylinked;
	ptlock_t lock;	
} sl_node_t;

typedef struct sl_map {
	sl_node_t *head;
} sl_map_t;

inline void *xmalloc(size_t size);
inline int rand_100();

int get_rand_level();
int floor_log_2(unsigned int n);

/* 
 * Create a new node without setting its next fields. 
 */
sl_node_t *sl_new_simple_node(sl_key_t key, sl_val_t val, int toplevel, int transactional);
/* 
 * Create a new node with its next field. 
 * If next=NULL, then this create a tail node. 
 */
sl_node_t *sl_new_node(sl_key_t key, sl_val_t val, sl_node_t *next, int toplevel, int 
transactional);

void sl_delete_node(sl_node_t *n);
sl_map_t *sl_set_new();

void sl_map_delete(sl_map_t *map);
int sl_map_size(sl_map_t *map);

/* 
 * Returns a pseudo-random value in [1;range).
 * Depending on the symbolic constant RAND_MAX>=32767 defined in stdlib.h,
 * the granularity of rand() could be lower-bounded by the 32767^th which might 
 * be too high for given values of range and initial.
 */
inline long rand_range(long r);
