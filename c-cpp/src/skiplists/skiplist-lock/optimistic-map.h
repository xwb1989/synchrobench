/*
 * File:
 *   optimistic-map.h
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 *   Wenbin Xiao <github.com/xwb1989>
 * Description:
 *   Fine-grained locking skip list implementation of Herlihy et al. algorithm
 *   "A Simple Optimistic Skiplist Algorithm" 
 *   M. Herlihy, Y. Lev, V. Luchangco, N. Shavit 
 *   p.124-138, SIROCCO 2007
 */

#include "skiplist-lock-map.h"

/*
 * interface for map
 */
sl_val_t optimistic_get(sl_map_t *map, sl_key_t key);
int optimistic_find(sl_map_t *map, sl_key_t key);
int optimistic_insert(sl_map_t *map, sl_key_t key, sl_val_t val);
int optimistic_delete(sl_map_t *map, sl_key_t key);
