/* 
 * hash.c --- 
 * 
 * Author: Insecticide
 * Created: 10-13-2025
 * Version: 1.0
 * 
 * Description: The following is the fully implemented hash.c file
 * 
 */

/* 
 * hash.c -- implements a generic hash table as an indexed set of queues.
 *
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "hash.h"
#include "queue.h"

/* 
 * SuperFastHash() -- produces a number between 0 and the tablesize-1.
 * 
 * The following (rather complicated) code, has been taken from Paul
 * Hsieh's website under the terms of the BSD license. It's a hash
 * function used all over the place nowadays, including Google Sparse
 * Hash.
 */
#define get16bits(d) (*((const uint16_t *) (d)))

static uint32_t SuperFastHash (const char *data,int len,uint32_t tablesize) {
  uint32_t hash = len, tmp;
  int rem;
  
  if (len <= 0 || data == NULL)
		return 0;
  rem = len & 3;
  len >>= 2;
  /* Main loop */
  for (;len > 0; len--) {
    hash  += get16bits (data);
    tmp    = (get16bits (data+2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    data  += 2*sizeof (uint16_t);
    hash  += hash >> 11;
  }
  /* Handle end cases */
  switch (rem) {
  case 3: hash += get16bits (data);
    hash ^= hash << 16;
    hash ^= data[sizeof (uint16_t)] << 18;
    hash += hash >> 11;
    break;
  case 2: hash += get16bits (data);
    hash ^= hash << 11;
    hash += hash >> 17;
    break;
  case 1: hash += *data;
    hash ^= hash << 10;
    hash += hash >> 1;
  }
  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;
  return hash % tablesize;
}

typedef struct i_hashtable {
  uint32_t size;
  queue_t **slots;
} i_hashtable;

hashtable_t *hopen(uint32_t hsize) {
  i_hashtable *htp = malloc(sizeof(i_hashtable));
  if (htp == NULL) {
    return NULL;
  }
  htp->size = hsize;
  htp->slots = calloc(hsize, sizeof(queue_t*));
  if (htp->slots == NULL) {
    free(htp);
    return NULL;
  }
  for (uint32_t i = 0; i < hsize; i++) {
    htp->slots[i] = qopen();
  }
  return (hashtable_t*)htp;
}

void hclose(hashtable_t *htp) {
  i_hashtable *ht = (i_hashtable*)htp;
  if (ht != NULL) {
    for (uint32_t i = 0; i < ht->size; i++) {
      if (ht->slots[i] != NULL) {
        qclose(ht->slots[i]);
      }
    }
    free(ht->slots);
    free(ht);
  }
}

int32_t hput(hashtable_t *htp, void *ep, const char *key, int keylen) {
  i_hashtable *ht = (i_hashtable*)htp;
  if (ht == NULL || ep == NULL || key == NULL) {
    return 1;
  }
  uint32_t index = SuperFastHash(key, keylen, ht->size);
  queue_t *qp = ht->slots[index];
  return qput(qp, ep);
}

void happly(hashtable_t *htp, void (*fn)(void* ep)) {
  i_hashtable *ht = (i_hashtable*)htp;
  if (ht != NULL) {
    for (uint32_t i = 0; i < ht->size; i++) {
      qapply(ht->slots[i], fn);
    }
  }
}

void *hsearch(hashtable_t *htp, bool (*searchfn)(void* elementp, const void* searchkeyp), const char *key, int32_t keylen) {
  i_hashtable *ht = (i_hashtable*)htp;
  if (ht == NULL || key == NULL) {
    return NULL;
  }
  uint32_t index = SuperFastHash(key, keylen, ht->size);
  return qsearch(ht->slots[index], searchfn, key);
}

void *hremove(hashtable_t *htp, bool (*searchfn)(void* elementp, const void* searchkeyp), const char *key, int32_t keylen) {
  i_hashtable *ht = (i_hashtable*)htp;
  if (ht == NULL || key == NULL) {
    return NULL;
  }
  uint32_t index = SuperFastHash(key, keylen, ht->size);
  return qremove(ht->slots[index], searchfn, key);
}
