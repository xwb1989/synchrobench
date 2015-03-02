/*
 * File:
 *   optimistic-map.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 *   Wenbin Xiao <github.com/xwb1989>
 *
 * Description:
 *   Fine-grained locking skip list.
 *   C implementation of the Herlihy et al. algorithm originally 
 *   designed for managed programming language.
 *   "A Simple Optimistic Skiplist Algorithm" 
 *   M. Herlihy, Y. Lev, V. Luchangco, N. Shavit 
 *   p.124-138, SIROCCO 2007
 */

#include "optimistic-map.h"

unsigned int levelmax;

inline int ok_to_delete(sl_node_t *node, int found) {
  return (node->fullylinked && ((node->toplevel-1) == found) && !node->marked);
}

/*
 * Function optimistic_search corresponds to the findNode method of the 
 * original paper. A fast parameter has been added to speed-up the search 
 * so that the function quits as soon as the searched element is found.
 */
inline int optimistic_search(sl_map_t *map, sl_key_t key, sl_node_t **preds, sl_node_t **succs, int fast) {
  int found, i;
  sl_node_t *pred, *curr;
	
  found = -1;
  pred = map->head;
	
  for (i = (pred->toplevel - 1); i >= 0; i--) {
    curr = pred->next[i];
    while (key > curr->key) {
      pred = curr;
      curr = pred->next[i];
    }
    if (preds != NULL) 
      preds[i] = pred;
    succs[i] = curr;
    if (found == -1 && key == curr->key) {
      found = i;
    }
  }
  return found;
}

/*
 * Function optimistic_get behaves similarly as optimistic_find; unlike 
 * optimistic_find returning a bool indicating whether the key is in the 
 * map or not, this one returns the value associated with the key.
 */
sl_val_t optimistic_get(sl_map_t *map, sl_key_t key) {
  sl_node_t **succs, **preds;
  int result, found;
	
  preds = (sl_node_t **)xmalloc(levelmax * sizeof(sl_node_t *));
  succs = (sl_node_t **)xmalloc(levelmax * sizeof(sl_node_t *));
  found = optimistic_search(map, key, preds, succs, 1);
  result = (found != -1 && succs[found]->fullylinked && !succs[found]->marked);
  void* ret = NULL;
  if (result) {
      ret = succs[found]->val;
  }
  free(preds);
  free(succs);
  return ret;
}



/*
 * Function optimistic_find corresponds to the contains method of the original 
 * paper. In contrast with the original version, it allocates and frees the 
 * memory at right places to avoid the use of a stop-the-world garbage 
 * collector. 
 */
int optimistic_find(sl_map_t *map, sl_key_t key) { 
  sl_node_t **succs, **preds;
  int result, found;
	
  preds = (sl_node_t **)xmalloc(levelmax * sizeof(sl_node_t *));
  succs = (sl_node_t **)xmalloc(levelmax * sizeof(sl_node_t *));
  found = optimistic_search(map, key, preds, succs, 1);
  result = (found != -1 && succs[found]->fullylinked && !succs[found]->marked);
  free(preds);
  free(succs);
  return result;
}

/*
 * Function unlock_levels is an helper function for the insert and delete 
 * functions.
 */ 
inline void unlock_levels(sl_node_t **nodes, int highestlevel, int j) {
  int i, r;
  sl_node_t *old = NULL;

  for (i = 0; i <= highestlevel; i++) {
    if (old != nodes[i])
    	if ((r=UNLOCK(&nodes[i]->lock)) != 0)
		fprintf(stderr, "Error %d from %d cannot unlock node[%d]:%ld\n", 
	        	r, j, i, (long)nodes[i]->key);
    old = nodes[i];
  }
}

/*
 * Function optimistic_insert stands for the add method of the original paper.
 * Instead of just adding the value as the set version does, it inserts a 
 * key-value pair.
 * Unlocking and freeing the memory are done at the right places.
 */
int optimistic_insert(sl_map_t *map, sl_key_t key, sl_val_t val) {
  sl_node_t **preds, **succs;
  sl_node_t  *node_found, *prev_pred, *new_node;
  sl_node_t *pred, *succ;
  int toplevel, highest_locked, i, valid, found;
  unsigned int backoff;
  struct timespec timeout;

  preds = (sl_node_t **)xmalloc(levelmax * sizeof(sl_node_t *));
  succs = (sl_node_t **)xmalloc(levelmax * sizeof(sl_node_t *));
  toplevel = get_rand_level();
  backoff = 1;
	
  while (1) {
    found = optimistic_search(map, key, preds, succs, 1);
    if (found != -1) {
      node_found = succs[found];
      if (!node_found->marked) {
	while (!node_found->fullylinked) {}
	free(preds);
	free(succs);
	return 0;
      }
      continue;
    }
    highest_locked = -1;
    prev_pred = NULL;
    valid = 1;
    for (i = 0; valid && (i < toplevel); i++) {
      pred = preds[i];
      succ = succs[i];
      if (pred != prev_pred) {
	if (LOCK(&pred->lock) != 0) 
	  fprintf(stderr, "Error cannot lock pred->key:%ld\n", (long)pred->key);
	highest_locked = i;
	prev_pred = pred;
      }	
			
      valid = (!pred->marked && !succ->marked && 
	       ((volatile sl_node_t*) pred->next[i] == 
		(volatile sl_node_t*) succ));
    }	
    if (!valid) {
      /* Unlock the predecessors before leaving */ 
      unlock_levels(preds, highest_locked, 11);
      if (backoff > 5000) {
	timeout.tv_sec = backoff / 5000;
	timeout.tv_nsec = (backoff % 5000) * 1000000;
	nanosleep(&timeout, NULL);
      }
      backoff *= 2;
      continue;
    }
		
    new_node = sl_new_simple_node(key, val, toplevel, 2);
    for (i = 0; i < toplevel; i++) {
      new_node->next[i] = succs[i];
      preds[i]->next[i] = new_node;
    }
		
    new_node->fullylinked = 1;
    unlock_levels(preds, highest_locked, 12);
    /* Freeing the previously allocated memory */
    free(preds);
    free(succs);
    return 1;
  }
}

/*
 * Function optimistic_delete is similar to the method remove of the paper.
 * Here we avoid the fast search parameter as the comparison is faster in C 
 * than calling the Java compareTo method of the Comparable interface 
 * (cf. p132 of SIROCCO'07 proceedings).
 */
int optimistic_delete(sl_map_t *map, sl_key_t key) {
  sl_node_t **preds, **succs;
  sl_node_t *node_todel, *prev_pred; 
  sl_node_t *pred, *succ;
  int is_marked, toplevel, highest_locked, i, valid, found;	
  unsigned int backoff;
  struct timespec timeout;

  preds = (sl_node_t **)xmalloc(levelmax * sizeof(sl_node_t *));
  succs = (sl_node_t **)xmalloc(levelmax * sizeof(sl_node_t *));
  node_todel = NULL;
  is_marked = 0;
  toplevel = -1;
  backoff = 1;
	
  while(1) {
    found = optimistic_search(map, key, preds, succs, 1);
    /* If not marked and ok to delete, then mark it */
    if (is_marked || (found != -1 && ok_to_delete(succs[found], found))) {	
      if (!is_marked) {
	node_todel = succs[found];
				
	if (LOCK(&node_todel->lock) != 0) 
	  fprintf(stderr, "Error cannot lock node_todel->key:%ld\n", 
		  (long)node_todel->key);
	toplevel = node_todel->toplevel;
	/* Unless it has been marked meanwhile */
	if (node_todel->marked) {
	  if (UNLOCK(&node_todel->lock) != 0)
	    fprintf(stderr, "Error cannot unlock node_todel->key:%ld\n", 
		    (long)node_todel->key);
	  free(preds);
	  free(succs);
	  return 0;
	}
	node_todel->marked = 1;
	is_marked = 1;
      }
      /* Physical deletion */
      highest_locked = -1;
      prev_pred = NULL;
      valid = 1;
      for (i = 0; valid && (i < toplevel); i++) {
	pred = preds[i];
	succ = succs[i];
	if (pred != prev_pred) {
	  LOCK(&pred->lock);
	  highest_locked = i;
	  prev_pred = pred;
	}
	valid = (!pred->marked && ((volatile sl_node_t*) pred->next[i] == 
				   (volatile sl_node_t*)succ));
      }
      if (!valid) {	
	unlock_levels(preds, highest_locked, 21);
	if (backoff > 5000) {
	  timeout.tv_sec = backoff / 5000;
	  timeout.tv_nsec = (backoff % 5000) / 1000000;
	  nanosleep(&timeout, NULL);
	}
	backoff *= 2;
	continue;
      }
			
      for (i = (toplevel-1); i >= 0; i--) 
	preds[i]->next[i] = node_todel->next[i];
      UNLOCK(&node_todel->lock);	
      unlock_levels(preds, highest_locked, 22);
      /* Freeing the previously allocated memory */
      free(preds);
      free(succs);
      return 1;
    } else {
      /* Freeing the previously allocated memory */
      free(preds);
      free(succs);
      return 0;
    }
  }
}
