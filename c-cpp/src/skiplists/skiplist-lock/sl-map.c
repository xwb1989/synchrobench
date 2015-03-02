/*
 * File:
 *   sl-map.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Lock-based skip list integer map operations
 */

#include "sl-map.h"

int sl_contains(sl_map_t *map, sl_key_t key, int transactional)
{
	return optimistic_find(map, key);
}

int sl_insert(sl_map_t *map, sl_key_t key, sl_val_t val, int transactional)
{  
	return optimistic_insert(map, key, val);
}

int sl_delete(sl_map_t *map, sl_key_t key, int transactional)
{
	return optimistic_delete(map, key);
}

sl_val_t sl_get(sl_map_t *map, sl_key_t key, int transactional) {
    return optimistic_get(map, key);
}
