/*
 * query.c --- TSE Querier (Module 6, Step 5)
 *
 * Author: Insecticide (Adapted by Gemini)
 * Date: 11-02-2025
 *
 * Description: Implements the querier component of the Tiny Search Engine.
 * Reads queries from stdin, validates and normalizes them,
 * searches the index, and ranks the results based on AND/OR logic.
 *
 * Usage: ./query <pageDirectory> <indexFile> [-q]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>   // For isalpha, tolower
#include <stdbool.h> // For bool

// TSE Utility Libraries
#include "index.h"    // For index data structures
#include "indexio.h"  // For indexload()
#include "pageio.h"   // For pageload()
#include "webpage.h"  // For webpage_delete()
#include "hash.h"
#include "queue.h"

#define MAX_WORDS 100 // Max words/operators in a query
#define MAX_LINE 512  // Max query line length

// --- Local Structs ---
typedef struct {
    int docID;
    int rank;
} query_result_t;

// --- Globals for iterator helpers ---
static queue_t* g_results_queue;  // For populating the initial queue
static int g_count;               // For counting items
static query_result_t** g_results_array; // For filling the sortable array

// --- Local function prototypes ---
static void parse_args(const int argc, char* argv[], char** pageDir, char** indexFile, bool* quiet_mode);
static int validate_and_parse_query(char* line, char* tokens[]);
static bool validate_word(char* word);
static void process_query(hashtable_t* index, char* pageDirectory, char* tokens[], int num_tokens);
static queue_t* compute_and_intersection(hashtable_t* index, char* tokens[], int start, int end);
static void merge_or_results(queue_t* final_results, queue_t* and_results);
static void print_results(queue_t* final_results, char* pageDirectory);

// --- Iterator & Helper Prototypes ---
static void init_results_helper(void* elementp);
static void count_helper(void* elementp);
static void fill_array_helper(void* elementp);
static void free_result_helper(void* elementp);
static int compare_results(const void* a, const void* b);
static char* read_url_from_file(char* pageDirectory, int docID);
static bool search_docid_queue(void* elementp, const void* keyp); // For final_results queue

// --- Data structure helper prototypes ---
static bool search_word(void* elementp, const void* keyp);
static bool search_doc(void* elementp, const void* keyp);
static void free_doc_entry(void* data);
static void free_word_entry(void* data);


/**
 * Main function: loops, gets, validates, and prints queries.
 */
int main(int argc, char* argv[]) {
    char* pageDirectory;
    char* indexFile;
    bool quiet_mode = false;

    // 1. Parse and validate command-line arguments
    parse_args(argc, argv, &pageDirectory, &indexFile, &quiet_mode);

    // 2. Load the index from the indexFile
    hashtable_t* index = indexload(indexFile);
    if (index == NULL) {
        fprintf(stderr, "Error: Failed to load index from '%s'.\n", indexFile);
        return EXIT_FAILURE;
    }

    // --- Main Query Loop ---
    char line[MAX_LINE];
    char* tokens[MAX_WORDS]; // Now stores words AND operators
    int num_tokens;

    if (!quiet_mode) {
        printf("> ");
    }
    
    // Loop reading lines from stdin until EOF (Ctrl-D)
    while (fgets(line, sizeof(line), stdin) != NULL) {
        
        if (!quiet_mode) {
            printf("Query: %s", line);
        }

        num_tokens = validate_and_parse_query(line, tokens);

        if (num_tokens == -1) {
            printf("[invalid query]\n");
        } else if (num_tokens > 0) {
            // Print the normalized query
            if (!quiet_mode) {
                printf("Normalized: ");
                for(int i = 0; i < num_tokens; i++) {
                    printf("%s ", tokens[i]);
                }
                printf("\n");
            }
            // Process the full query
            process_query(index, pageDirectory, tokens, num_tokens);
        }
        
        if (!quiet_mode) {
            printf("-----------------------------------------------\n> ");
        }
    }
    
    if (!quiet_mode) {
        printf("\n"); // Print a newline for a clean exit
    }

    // 3. Clean up
    happly(index, free_word_entry);
    hclose(index);

    return EXIT_SUCCESS;
}

/**
 * Parses and validates command-line arguments.
 */
static void parse_args(const int argc, char* argv[], char** pageDir, char** indexFile, bool* quiet_mode) {
    // Check for correct number of arguments
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <pageDirectory> <indexFile> [-q]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    *pageDir = argv[1];
    *indexFile = argv[2];
    *quiet_mode = false; // Default

    // Check for quiet mode flag
    if (argc == 4) {
        if (strcmp(argv[3], "-q") == 0) {
            *quiet_mode = true;
        } else {
            fprintf(stderr, "Usage: %s <pageDirectory> <indexFile> [-q]\n", argv[0]);
            fprintf(stderr, "Error: Unrecognized 4th argument '%s'.\n", argv[3]);
            exit(EXIT_FAILURE);
        }
    }

    // Validate pageDirectory (by trying to load page 1)
    webpage_t* test_page = pageload(1, *pageDir);
    if (test_page == NULL) {
        fprintf(stderr, "Error: '%s' is not a valid crawler directory.\n", *pageDir);
        exit(EXIT_FAILURE);
    }
    webpage_delete(test_page);

    // Validate indexFile (by trying to open for reading)
    FILE* fp = fopen(*indexFile, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot read index file '%s'.\n", *indexFile);
        exit(EXIT_FAILURE);
    }
    fclose(fp);
}

/**
 * Validates the syntax of the query (operators) and normalizes words.
 * Fills 'tokens' with all words and operators.
 * Returns -1 on syntax error, or the number of tokens.
 */
static int validate_and_parse_query(char* line, char* tokens[]) {
    int count = 0;
    char* token = strtok(line, " \t\n");

    while (token != NULL) {
        if (count >= MAX_WORDS) {
            fprintf(stderr, "Error: Query exceeds %d words/operators.\n", MAX_WORDS);
            return -1;
        }
        tokens[count++] = token;
        token = strtok(NULL, " \t\n");
    }
    if (count == 0) return 0; // Empty line
    
    // --- Syntax Validation ---
    if (strcmp(tokens[0], "and") == 0 || strcmp(tokens[0], "or") == 0) {
        fprintf(stderr, "Error: Query cannot begin with an operator.\n");
        return -1;
    }
    if (strcmp(tokens[count-1], "and") == 0 || strcmp(tokens[count-1], "or") == 0) {
        fprintf(stderr, "Error: Query cannot end with an operator.\n");
        return -1;
    }

    bool last_was_operator = false;
    for (int i = 0; i < count; i++) {
        bool this_is_operator = (strcmp(tokens[i], "and") == 0 || strcmp(tokens[i], "or") == 0);

        if (this_is_operator) {
            if (last_was_operator) {
                fprintf(stderr, "Error: Cannot have adjacent operators.\n");
                return -1;
            }
            last_was_operator = true;
        } else {
            if (!validate_word(tokens[i])) {
                fprintf(stderr, "Error: Invalid characters in query (must be letters).\n");
                return -1;
            }
            last_was_operator = false;
        }
    }

    return count; // Valid syntax
}

/**
 * Validates and normalizes a word (in-place).
 */
static bool validate_word(char* word) {
    if (word == NULL) return false;
    for (int i = 0; word[i] != '\0'; i++) {
        if (!isalpha((unsigned char)word[i])) {
            return false;
        }
        word[i] = tolower((unsigned char)word[i]);
    }
    return true;
}

/**
 * Main query processor. Splits the query by 'or' and merges the results.
 */
static void process_query(hashtable_t* index, char* pageDirectory, char* tokens[], int num_tokens) {
    // Master results list: a queue of query_result_t*
    queue_t* final_results = qopen();
    
    int and_start_index = 0;

    for (int i = 0; i <= num_tokens; i++) {
        if (i == num_tokens || strcmp(tokens[i], "or") == 0) {
            queue_t* and_results = compute_and_intersection(index, tokens, and_start_index, i - 1);
            if (and_results != NULL) {
                merge_or_results(final_results, and_results);
                qclose(and_results);
            }
            and_start_index = i + 1;
        }
    }

    print_results(final_results, pageDirectory);

    qapply(final_results, free_result_helper);
    qclose(final_results);
}

/**
 * Computes the intersection (AND) of a sequence of tokens.
 * Returns a queue of query_result_t* for the matches.
 */
static queue_t* compute_and_intersection(hashtable_t* index, char* tokens[], int start, int end) {
    queue_t* results_queue = qopen();
    
    int first_word_idx = -1;
    word_entry_t* first_word = NULL;
    for (int i = start; i <= end; i++) {
        if (strcmp(tokens[i], "and") != 0 && strlen(tokens[i]) >= 3) {
            first_word = hsearch(index, search_word, tokens[i], strlen(tokens[i]));
            if (first_word != NULL) {
                first_word_idx = i;
                break;
            } else {
                qclose(results_queue);
                return NULL;
            }
        }
    }

    if (first_word == NULL) {
        qclose(results_queue);
        return NULL;
    }
    
    g_results_queue = results_queue;
    qapply(first_word->docs, init_results_helper);

    for (int i = first_word_idx + 1; i <= end; i++) {
        if (strcmp(tokens[i], "and") == 0 || strlen(tokens[i]) < 3) {
            continue;
        }

        word_entry_t* next_word = hsearch(index, search_word, tokens[i], strlen(tokens[i]));

        if (next_word == NULL) {
            qapply(results_queue, free_result_helper);
            qclose(results_queue);
            return NULL;
        }

        g_count = 0;
        qapply(results_queue, count_helper);
        int num_to_check = g_count;

        for (int j = 0; j < num_to_check; j++) {
            query_result_t* qr = qget(results_queue);
            doc_entry_t* found = qsearch(next_word->docs, search_doc, &(qr->docID));

            if (found) {
                if (found->count < qr->rank) {
                    qr->rank = found->count;
                }
                qput(results_queue, qr);
            } else {
                free(qr);
            }
        }
    }
    return results_queue;
}

/**
 * Merges a queue of AND results into the final master queue.
 */
static void merge_or_results(queue_t* final_results, queue_t* and_results) {
    query_result_t* qr;
    while ((qr = qget(and_results)) != NULL) {
        query_result_t* master_qr = qsearch(final_results, search_docid_queue, &(qr->docID));
        
        if (master_qr == NULL) {
            qput(final_results, qr);
        } else {
            master_qr->rank += qr->rank;
            free(qr);
        }
    }
}

/**
 * Converts the final results queue into a sorted array and prints.
 */
static void print_results(queue_t* final_results, char* pageDirectory) {
    g_count = 0;
    qapply(final_results, count_helper);
    int num_final = g_count;

    if (num_final == 0) {
        printf("No documents match.\n");
        return;
    }

    query_result_t** results_array = calloc(num_final, sizeof(query_result_t*));
    g_count = 0;
    g_results_array = results_array;
    qapply(final_results, fill_array_helper);

    qsort(results_array, num_final, sizeof(query_result_t*), compare_results);

    printf("Matches %d documents (ranked):\n", num_final);
    for (int i = 0; i < num_final; i++) {
        query_result_t* qr = results_array[i];
        
        char* url = read_url_from_file(pageDirectory, qr->docID);
        if (url) {
            printf("rank: %d: doc: %d: %s\n", qr->rank, qr->docID, url);
            free(url);
        } else {
            printf("rank: %d: doc: %d: [Could not read URL]\n", qr->rank, qr->docID);
        }
    }
    
    free(results_array);
}


// --- Iterator & Helper Functions ---

static void init_results_helper(void* elementp) {
    doc_entry_t* d_entry = (doc_entry_t*)elementp;
    
    query_result_t* qr = malloc(sizeof(query_result_t));
    if (qr) {
        qr->docID = d_entry->docID;
        qr->rank = d_entry->count;
        qput(g_results_queue, qr);
    }
}

static void count_helper(void* elementp) {
    if (elementp != NULL) g_count++;
}

static void fill_array_helper(void* elementp) {
    if (elementp != NULL) {
        g_results_array[g_count++] = (query_result_t*)elementp;
    }
}

static void free_result_helper(void* elementp) {
    if (elementp) {
        free(elementp);
    }
}

static int compare_results(const void* a, const void* b) {
    query_result_t* res_a = *(query_result_t**)a;
    query_result_t* res_b = *(query_result_t**)b;
    return res_b->rank - res_a->rank; // b - a for descending order
}

static char* read_url_from_file(char* pageDirectory, int docID) {
    char filepath[MAX_LINE];
    sprintf(filepath, "%s/%d", pageDirectory, docID);

    FILE* fp = fopen(filepath, "r");
    if (fp == NULL) return NULL;

    char* url = malloc(MAX_LINE);
    if (url == NULL) {
        fclose(fp);
        return NULL;
    }

    if (fgets(url, MAX_LINE, fp) == NULL) {
        free(url);
        fclose(fp);
        return NULL;
    }
    
    fclose(fp);
    url[strcspn(url, "\n")] = 0;
    return url;
}

//
// --- Data Structure Helper Functions ---
//

static bool search_word(void* elementp, const void* keyp) {
    word_entry_t* entry = (word_entry_t*)elementp;
    return strcmp(entry->word, (const char*)keyp) == 0;
}

static bool search_doc(void* elementp, const void* keyp) {
    doc_entry_t* entry = (doc_entry_t*)elementp;
    return entry->docID == *(int*)keyp;
}

static bool search_docid_queue(void* elementp, const void* keyp) {
    query_result_t* entry = (query_result_t*)elementp;
    return entry->docID == *(int*)keyp;
}

static void free_doc_entry(void* data) {
    doc_entry_t* doc = (doc_entry_t*)data;
    if (doc) free(doc);
}

static void free_word_entry(void* data) {
    word_entry_t* word = (word_entry_t*)data;
    if (word) {
        free(word->word); 
        qapply(word->docs, free_doc_entry); 
        qclose(word->docs); 
        free(word); 
    }
}
