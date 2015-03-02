/*
 * File:
 *   sl-map.h
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 *   Wenbin Xiao <github.com/xwb1989>
 * Description:
 *   Lock-based skip list integer set operations
 *
 */

#include "optimistic-map.h"

int sl_contains(sl_map_t *map, sl_key_t key, int transactional);
int sl_insert(sl_map_t *map, sl_key_t key, sl_val_t val, int transactional);
int sl_delete(sl_map_t *map, sl_key_t key, int transactional);
sl_val_t sl_get(sl_map_t *map, sl_key_t key, int transactional);
