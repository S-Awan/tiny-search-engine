/*
 * index.h - header file for TSE index data structures
 *
 * Author: Insecticide
 * Date: 10-30-2025
 *
 * Description: Defines the data structures used by the
 * indexer and indexio modules.
 */

#pragma once

#include "queue.h"

// Entry in the document queue (stores count for a doc)
typedef struct doc_entry {
    int docID;
    int count;
} doc_entry_t;

// Entry in the index (stores the word and its queue of docs)
typedef struct word_entry {
    char *word;       // The word itself
    queue_t *docs; // Queue of doc_entry_t
} word_entry_t;
