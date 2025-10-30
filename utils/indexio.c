/*
 * indexio.c - implementation of index save/load module
 *
 * Author: Insecticide
 * Date: 10-30-2025
 *
 * Description: Saves and loads index data structures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "index.h"    // Contains the struct definitions
#include "indexio.h"  // Contains our function prototypes
#include "hash.h"
#include "queue.h"

// --- Static helper function prototypes for saving ---
static void save_word_entry(void* data);
static void save_doc_queue(void* data);

// --- Static file pointer for apply helpers ---
static FILE* save_fp; // Used by the apply helper functions

/*
 * indexsave - Saves the index to a file.
 */
int indexsave(hashtable_t* index, const char* indexnm) {
    save_fp = fopen(indexnm, "w");
    if (save_fp == NULL) {
        perror("Error: indexsave failed to open file");
        return 1;
    }

    // Use happly to iterate over every word in the index
    happly(index, save_word_entry);

    fclose(save_fp);
    return 0;
}

// Helper for happly (saves one word_entry)
static void save_word_entry(void* data) {
    word_entry_t* word = (word_entry_t*)data;
    // Print the word
    fprintf(save_fp, "%s", word->word);
    
    // Use qapply to iterate over the docs for this word
    qapply(word->docs, save_doc_queue);
    
    // End the line
    fprintf(save_fp, "\n");
}

// Helper for qapply (saves one doc_entry)
static void save_doc_queue(void* data) {
    doc_entry_t* doc = (doc_entry_t*)data;
    // Print " <docID> <count>"
    fprintf(save_fp, " %d %d", doc->docID, doc->count);
}


/*
 * indexload - Loads an index from a file.
 */
hashtable_t* indexload(const char* indexnm) {
    FILE* fp = fopen(indexnm, "r");
    if (fp == NULL) {
        perror("Error: indexload failed to open file");
        return NULL;
    }

    // Create a new index
    hashtable_t* index = hopen(500); // 500 is a reasonable size
    if (index == NULL) {
        fclose(fp);
        return NULL;
    }

    // Max line length assumption
    char line[10000]; 
    while (fgets(line, sizeof(line), fp) != NULL) {
        char word[256];
        int offset = 0;
        int n_read = 0;

        // 1. Read the word
        if (sscanf(line, "%255s%n", word, &n_read) != 1) {
            continue; // Skip malformed lines
        }
        offset += n_read;

        // 2. Create the index entry for this word
        word_entry_t* word_entry = malloc(sizeof(word_entry_t));
        word_entry->word = malloc(strlen(word) + 1);
        strcpy(word_entry->word, word);
        word_entry->docs = qopen();
        
        // 3. Loop, reading (doc, count) pairs from the rest of the line
        int docID, count;
        while (sscanf(line + offset, " %d %d%n", &docID, &count, &n_read) == 2) {
            // Create doc_entry_t and add to queue
            doc_entry_t* doc_entry = malloc(sizeof(doc_entry_t));
            doc_entry->docID = docID;
            doc_entry->count = count;
            qput(word_entry->docs, doc_entry);
            
            offset += n_read; // Move offset past the pair
        }
        
        // 4. Add the complete word_entry_t to the hash table
        hput(index, word_entry, word_entry->word, strlen(word_entry->word));
    }

    fclose(fp);
    return index;
}
