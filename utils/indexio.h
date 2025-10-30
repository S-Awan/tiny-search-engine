/*
 * indexio.h - header file for index save/load module
 *
 * Author: Insecticide
 * Date: 10-30-2025
 *
 * Description: Saves and loads index data structures.
 */

#pragma once

#include "hash.h"

/*
 * indexsave - Saves the index to a file.
 * @index: pointer to the index hash table.
 * @indexnm: name of the file to save to.
 * Returns 0 on success, non-zero on failure.
 */
int indexsave(hashtable_t *index, const char *indexnm);

/*
 * indexload - Loads an index from a file.
 * @indexnm: name of the file to load from.
 * Returns a new hashtable_t* on success, NULL on failure.
 * Caller is responsible for hclosing the returned table.
 */
hashtable_t *indexload(const char *indexnm);
