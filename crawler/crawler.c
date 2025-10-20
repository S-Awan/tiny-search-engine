/* 
 * crawler.c --- 
 * 
 * Author: Insecticide
 * Created: 10-19-2025
 * Version: 1.0
 * 
 * Description: a simple web crawler
 
 * Usage: ./crawler seedURL pageDirectory maxDepth
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For access()
#include "webpage.h"
#include "queue.h"
#include "hash.h"

// --- Local Function Prototypes ---
static void parse_args(const int argc, char* argv[], char** seedURL, char** pageDir, int* maxDepth);
static void crawl(char* seedURL, char* pageDir, const int maxDepth);
static int32_t pagesave(webpage_t* pagep, int id, const char* dirname);
static bool search_url(void* elementp, const void* keyp);
static void free_item(void* item);

// --- Main Program ---
int main(const int argc, char* argv[]) {
    char* seedURL;
    char* pageDir;
    int maxDepth;

    parse_args(argc, argv, &seedURL, &pageDir, &maxDepth);
    crawl(seedURL, pageDir, maxDepth);

    return EXIT_SUCCESS;
}

/**
 * Parses and validates command-line arguments.
 * Exits the program if arguments are invalid.
 */
static void parse_args(const int argc, char* argv[], char** seedURL, char** pageDir, int* maxDepth) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s seedURL pageDirectory maxDepth\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    *seedURL = argv[1];
    *pageDir = argv[2];

    // Validate pageDirectory
    char filepath[256];
    sprintf(filepath, "%s/.crawler", *pageDir);
    FILE* fp = fopen(filepath, "w");
    if (fp == NULL) {
        fprintf(stderr, "Error: pageDirectory '%s' is not a writable directory.\n", *pageDir);
        exit(EXIT_FAILURE);
    }
    fclose(fp);
    remove(filepath); // Clean up the test file

    // Validate maxDepth
    if (sscanf(argv[3], "%d", maxDepth) != 1 || *maxDepth < 0) {
        fprintf(stderr, "Error: maxDepth must be a non-negative integer.\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * Contains the main crawling loop and logic.
 */
static void crawl(char* seedURL, char* pageDir, const int maxDepth) {
    hashtable_t* seen_urls = hopen(200);
    queue_t* pages_to_crawl = qopen();
    int docID = 1;

    // Normalize the seed URL and add it to the hash table first
    char* seedURL_copy = malloc(strlen(seedURL) + 1);
    strcpy(seedURL_copy, seedURL);
    NormalizeURL(seedURL_copy);
    hput(seen_urls, seedURL_copy, seedURL_copy, strlen(seedURL_copy));
    
    // Create the first webpage and add it to the queue
    webpage_t* first_page = webpage_new(seedURL, 0, NULL);
    qput(pages_to_crawl, first_page);

    webpage_t* current_page;
    while ((current_page = qget(pages_to_crawl)) != NULL) {
        printf("Crawling: %s\n", webpage_getURL(current_page));

        if (!webpage_fetch(current_page)) {
            fprintf(stderr, "Warning: Failed to fetch HTML for %s\n", webpage_getURL(current_page));
            webpage_delete(current_page);
            continue; // Ignore this URL and move on
        }

        pagesave(current_page, docID++, pageDir);

        if (webpage_getDepth(current_page) < maxDepth) {
            int pos = 0;
            char* result_url;
            while ((pos = webpage_getNextURL(current_page, pos, &result_url)) > 0) {
                if (IsInternalURL(result_url)) {
                    if (hsearch(seen_urls, search_url, result_url, strlen(result_url)) == NULL) {
                        char* url_copy = malloc(strlen(result_url) + 1);
                        strcpy(url_copy, result_url);
                        hput(seen_urls, url_copy, url_copy, strlen(url_copy));

                        webpage_t* new_page = webpage_new(result_url, webpage_getDepth(current_page) + 1, NULL);
                        qput(pages_to_crawl, new_page);
                    }
                }
                free(result_url);
            }
        }
        webpage_delete(current_page);
    }

    // Clean up
    happly(seen_urls, free_item);
    hclose(seen_urls);
    qclose(pages_to_crawl);
}

/**
 * Saves a fetched page to a file in a specified directory.
 */
static int32_t pagesave(webpage_t* pagep, int id, const char* dirname) {
    char filepath[256];
    sprintf(filepath, "%s/%d", dirname, id);

    FILE* fp = fopen(filepath, "w");
    if (fp == NULL) {
        perror("Error opening file for writing");
        return 1;
    }

    fprintf(fp, "%s\n", webpage_getURL(pagep));
    fprintf(fp, "%d\n", webpage_getDepth(pagep));
    fprintf(fp, "%d\n", webpage_getHTMLlen(pagep));
    fprintf(fp, "%s", webpage_getHTML(pagep));

    fclose(fp);
    return 0;
}

/**
 * Helper function to compare URLs in the hash table.
 */
static bool search_url(void* elementp, const void* keyp) {
    char* url_in_table = (char*)elementp;
    const char* search_key = (const char*)keyp;
    return strcmp(url_in_table, search_key) == 0;
}

/**
 * Helper function to free strings stored in the hash table.
 */
static void free_item(void* item) {
    if (item != NULL) {
        free(item);
    }
}
