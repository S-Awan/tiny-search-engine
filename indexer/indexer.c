/*
 * indexer.c - builds an index from crawler page files
 *
 * Author: Insecticide
 * Date: 10-30-2025
 *
 * Usage: ./indexer pageDirectory indexFilename
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // For isalpha, tolower
#include <unistd.h> // For access()
#include "webpage.h"
#include "pageio.h"
#include "hash.h"
#include "queue.h"
#include "index.h"    // Contains the shared struct definitions
#include "indexio.h"  // For indexsave() and indexload()

// --- Local Function Prototypes ---
static void parse_args(const int argc, char* argv[], char** pageDir, char** indexFile);
static hashtable_t* build_index(char* pageDir);
static char* NormalizeWord(char* word);

// Helper functions for data structures
static bool search_word(void* elementp, const void* keyp);
static bool search_doc(void* elementp, const void* keyp);
static void free_doc_entry(void* data);
static void free_word_entry(void* data);

// --- Main Function ---

int main(int argc, char* argv[]) {
    char* pageDir;
    char* indexFile;

    // 1. Validate command-line arguments
    parse_args(argc, argv, &pageDir, &indexFile);

    // 2. Build the index from the page directory
    hashtable_t* index = build_index(pageDir);
    if (index == NULL) {
        fprintf(stderr, "Failed to build index.\n");
        return EXIT_FAILURE;
    }

    // 3. Save the index to the output file
    if (indexsave(index, indexFile) != 0) {
        fprintf(stderr, "Failed to save index to file: %s\n", indexFile);
        happly(index, free_word_entry);
        hclose(index);
        return EXIT_FAILURE;
    }
    
    printf("Index saved to %s\n", indexFile);

    // 4. Clean up all allocated memory
    happly(index, free_word_entry);
    hclose(index);

    return EXIT_SUCCESS;
}

/**
 * Parses and validates command-line arguments.
 * Exits the program if arguments are invalid.
 */
static void parse_args(const int argc, char* argv[], char** pageDir, char** indexFile) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s pageDirectory indexFilename\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    *pageDir = argv[1];
    *indexFile = argv[2];

    // Validate pageDirectory by checking if the first page is readable
    webpage_t* first_page = pageload(1, *pageDir);
    if (first_page == NULL) {
        fprintf(stderr, "Error: pageDirectory '%s' is not a valid crawler directory (or page 1 is missing).\n", *pageDir);
        exit(EXIT_FAILURE);
    }
    webpage_delete(first_page);
}

/**
 * Loops through all page files in pageDir, building the index.
 * Returns a pointer to the new index.
 */
static hashtable_t* build_index(char* pageDir) {
    hashtable_t* index = hopen(500); // Our main index
    if (index == NULL) {
        return NULL;
    }

    int docID = 1;
    webpage_t* page;

    // Loop from docID 1 until pageload fails
    while ((page = pageload(docID, pageDir)) != NULL) {
        printf("Processing page %d\n", docID);
        
        int pos = 0;
        char* word;
        
        while ((pos = webpage_getNextWord(page, pos, &word)) > 0) {
            char* normalized = NormalizeWord(word);
            
            if (normalized != NULL) {
                word_entry_t* found_word = hsearch(index, search_word, normalized, strlen(normalized));
                
                if (found_word == NULL) {
                    // New word, not in hash table
                    word_entry_t* new_word_entry = malloc(sizeof(word_entry_t));
                    new_word_entry->word = malloc(strlen(normalized) + 1);
                    strcpy(new_word_entry->word, normalized);
                    new_word_entry->docs = qopen();
                    
                    doc_entry_t* new_doc_entry = malloc(sizeof(doc_entry_t));
                    new_doc_entry->docID = docID;
                    new_doc_entry->count = 1;
                    
                    qput(new_word_entry->docs, new_doc_entry);
                    hput(index, new_word_entry, new_word_entry->word, strlen(new_word_entry->word));
                } else {
                    // Word is already in the hash table
                    doc_entry_t* found_doc = qsearch(found_word->docs, search_doc, &docID);
                    
                    if (found_doc == NULL) {
                        // Word's first time in this doc
                        doc_entry_t* new_doc_entry = malloc(sizeof(doc_entry_t));
                        new_doc_entry->docID = docID;
                        new_doc_entry->count = 1;
                        qput(found_word->docs, new_doc_entry);
                    } else {
                        // Word seen before in this doc, increment count
                        found_doc->count++;
                    }
                }
            }
            free(word); // webpage_getNextWord allocates memory for word
        }
        webpage_delete(page); // Free the page
        docID++; // Move to the next page file
    }
    
    printf("Indexed %d pages.\n", docID - 1);
    return index;
}


// --- Helper Functions ---

/**
 * Normalizes a word: converts to lowercase, skips if non-alphabetic
 * or length < 3.
 * Returns the word if valid, NULL otherwise.
 */
static char* NormalizeWord(char* word) {
    if (word == NULL || strlen(word) < 3) {
        return NULL;
    }
    for (int i = 0; word[i] != '\0'; i++) {
        if (!isalpha(word[i])) {
            return NULL; // Contains non-alpha char
        }
        word[i] = tolower(word[i]);
    }
    return word;
}

// Search function for hash table (compares word)
static bool search_word(void* elementp, const void* keyp) {
    word_entry_t* entry = (word_entry_t*)elementp;
    return strcmp(entry->word, (const char*)keyp) == 0;
}

// Search function for inner queue (compares docID)
static bool search_doc(void* elementp, const void* keyp) {
    doc_entry_t* entry = (doc_entry_t*)elementp;
    return entry->docID == *(int*)keyp;
}

// Frees a doc_entry_t (for qapply)
static void free_doc_entry(void* data) {
    doc_entry_t* doc = (doc_entry_t*)data;
    if (doc) free(doc);
}

// Frees a word_entry_t (for happly)
static void free_word_entry(void* data) {
    word_entry_t* word = (word_entry_t*)data;
    if (word) {
        free(word->word); // Free the word string
        qapply(word->docs, free_doc_entry); // Free all doc entries
        qclose(word->docs); // Free the queue itself
        free(word); // Free the word entry struct
    }
}
