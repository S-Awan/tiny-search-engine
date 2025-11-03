/*
 * indextest.c - test program for the 'indexio' module
 *
 * Author: Insecticide
 * Date: 10-30-2025
 *
 * Usage: ./indextest
 *
 * Description:
 * 1. Creates a simple in-memory index.
 * 2. Saves it to a file "test.dat" using indexsave().
 * 3. Loads it from "test.dat" into a new index structure using indexload().
 * 4. Saves the new index to "test_reload.dat".
 * 5. Runs 'diff' to compare "test.dat" and "test_reload.dat".
 * 6. Reports PASS/FAIL and cleans up memory and files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "queue.h"
#include "index.h"
#include "indexio.h"

// --- Helper function prototypes ---
static hashtable_t* create_test_index(void);
static void free_doc_entry(void* data);
static void free_word_entry(void* data);

// --- Main Test Function ---
int main(void) {
    const char* testfile = "test.dat";
    const char* reloadfile = "test_reload.dat";
    int status = 0; // 0 = PASS

    printf("Starting indextest...\n");

    // 1. Create a known index
    hashtable_t* index1 = create_test_index();
    if (index1 == NULL) {
        fprintf(stderr, "Failed to create test index.\n");
        return 1;
    }

    // 2. Save the index to a file
    if (indexsave(index1, testfile) != 0) {
        fprintf(stderr, "indexsave() failed.\n");
        status = 1; // Mark as FAIL
    }

    // 3. Load the index from that file
    hashtable_t* index2 = indexload(testfile);
    if (index2 == NULL) {
        fprintf(stderr, "indexload() failed.\n");
        status = 1; // Mark as FAIL
    }

    // 4. Save the newly loaded index to a second file
    if (status == 0 && indexsave(index2, reloadfile) != 0) {
        fprintf(stderr, "indexsave() failed on reloaded index.\n");
        status = 1; // Mark as FAIL
    }

    // 5. Run 'diff' to compare the two saved files
    if (status == 0) {
        printf("Comparing %s and %s...\n", testfile, reloadfile);
        char diff_cmd[512];
        sprintf(diff_cmd, "diff %s %s", testfile, reloadfile);
        
        int diff_status = system(diff_cmd);
        if (diff_status != 0) {
            fprintf(stderr, "FAIL: Files %s and %s differ.\n", testfile, reloadfile);
            status = 1;
        } else {
            printf("PASS: indexload() successfully recreated the index.\n");
        }
    }

    // 6. Clean up
    printf("Cleaning up...\n");
    happly(index1, free_word_entry);
    hclose(index1);
    if (index2 != NULL) {
        happly(index2, free_word_entry);
        hclose(index2);
    }
    remove(testfile);
    remove(reloadfile);

    return status;
}

/**
 * Creates a simple, hard-coded index for testing.
 *
 * Index will contain:
 * "cat": (doc 1, count 2), (doc 3, count 1)
 * "dog": (doc 2, count 5)
 */
static hashtable_t* create_test_index(void) {
    hashtable_t* index = hopen(10);
    if (index == NULL) return NULL;

    // --- "cat" ---
    word_entry_t* cat_entry = malloc(sizeof(word_entry_t));
    cat_entry->word = "cat"; // Use string literal for simplicity
    cat_entry->docs = qopen();
    
    doc_entry_t* cat_doc1 = malloc(sizeof(doc_entry_t));
    cat_doc1->docID = 1; cat_doc1->count = 2;
    qput(cat_entry->docs, cat_doc1);
    
    doc_entry_t* cat_doc3 = malloc(sizeof(doc_entry_t));
    cat_doc3->docID = 3; cat_doc3->count = 1;
    qput(cat_entry->docs, cat_doc3);
    
    hput(index, cat_entry, cat_entry->word, strlen(cat_entry->word));

    // --- "dog" ---
    word_entry_t* dog_entry = malloc(sizeof(word_entry_t));
    dog_entry->word = "dog"; // Use string literal
    dog_entry->docs = qopen();
    
    doc_entry_t* dog_doc2 = malloc(sizeof(doc_entry_t));
    dog_doc2->docID = 2; dog_doc2->count = 5;
    qput(dog_entry->docs, dog_doc2);
    
    hput(index, dog_entry, dog_entry->word, strlen(dog_entry->word));

    return index;
}

// --- Cleanup Helper Functions ---

// Frees a doc_entry_t (for qapply)
static void free_doc_entry(void* data) {
    doc_entry_t* doc = (doc_entry_t*)data;
    if (doc) free(doc);
}

// Frees a word_entry_t (for happly)
static void free_word_entry(void* data) {
    word_entry_t* word = (word_entry_t*)data;
    if (word) {
        // NOTE: We don't free(word->word) here because we used
        // string literals in create_test_index(). 
        // A real index would malloc/free the word string.
        qapply(word->docs, free_doc_entry);
        qclose(word->docs);
        free(word);
    }
}
