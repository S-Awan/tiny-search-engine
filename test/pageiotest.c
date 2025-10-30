/*
 * pageiotest.c - test program for the 'pageio' module
 *
 * Author: Insecticide
 * Date: 10-30-2025
 *
 * Usage: ./pageiotest <pageDirectory>
 *
 * Description:
 * 1. Loads a known page (page 1) from the <pageDirectory>.
 * 2. Saves it to a temp file "999" using pagesave().
 * 3. Loads it from "999" into a new webpage.
 * 4. Saves the new webpage to "888".
 * 5. Runs 'diff' to compare the two temp files.
 * 6. Reports PASS/FAIL and cleans up.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "webpage.h"
#include "pageio.h"

int main(int argc, char* argv[]) {
    // Use integer IDs for temp files, as required by the module
    const int temp_id_1 = 999;
    const int temp_id_2 = 888;
    
    char temp_file_1[20];
    char temp_file_2[20];
    sprintf(temp_file_1, "%d", temp_id_1);
    sprintf(temp_file_2, "%d", temp_id_2);

    int status = 0; // 0 = PASS

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pageDirectory>\n", argv[0]);
        return 1;
    }
    char* pageDir = argv[1];

    printf("Starting pageiotest...\n");

    // 1. Load page 1 from the specified directory
    webpage_t* page1 = pageload(1, pageDir);
    if (page1 == NULL) {
        fprintf(stderr, "FAIL: pageload() failed to load page 1 from %s\n", pageDir);
        return 1;
    }

    // 2. Save the loaded page to a temporary file (e.g., "./999")
    if (pagesave(page1, temp_id_1, ".") != 0) {
        fprintf(stderr, "FAIL: pagesave() failed to write to file %s\n", temp_file_1);
        status = 1;
    }

    // 3. Load the new temporary file back into memory
    webpage_t* page2 = pageload(temp_id_1, ".");
    if (page2 == NULL) {
        fprintf(stderr, "FAIL: pageload() failed to read back from %s\n", temp_file_1);
        status = 1;
    }

    // 4. Save the re-loaded page to a second temporary file
    if (status == 0) {
        if (pagesave(page2, temp_id_2, ".") != 0) {
            fprintf(stderr, "FAIL: pagesave() failed on reloaded page.\n");
            status = 1;
        }
    }

    // 5. Run 'diff' to compare the two temporary files
    if (status == 0) {
        printf("Comparing %s and %s...\n", temp_file_1, temp_file_2);
        char diff_cmd[512];
        sprintf(diff_cmd, "diff %s %s", temp_file_1, temp_file_2);
        
        int diff_status = system(diff_cmd);
        if (diff_status != 0) {
            fprintf(stderr, "FAIL: Files %s and %s differ.\n", temp_file_1, temp_file_2);
            status = 1;
        } else {
            printf("PASS: pageload() and pagesave() are consistent.\n");
        }
    }

    // 6. Clean up
    printf("Cleaning up...\n");
    webpage_delete(page1);
    if (page2 != NULL) {
        webpage_delete(page2);
    }
    remove(temp_file_1);
    remove(temp_file_2);

    return status;
}
