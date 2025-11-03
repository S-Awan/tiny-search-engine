/* * pageio.c --- saving and loading crawler webpage files
 *
 * Author: Insecticide
 * Date: 10-30-2025
 *
 * Description: Implements pagesave and pageload.
 * pagesave saves an existing webpage to a file.
 * pageload creates a new page by loading a file.
 */

#include "pageio.h"

/*
 * pagesave -- save the page in filename id in directory dirnm
 *
 * returns: 0 for success; nonzero otherwise
 *
 * The file format is:
 * <url>
 * <depth>
 * <html-length>
 * <html>
 */
int32_t pagesave(webpage_t *pagep, int id, char *dirnm) {
    char filepath[256];
    sprintf(filepath, "%s/%d", dirnm, id);

    FILE *fp = fopen(filepath, "w");
    if (fp == NULL) {
        perror("Error: pagesave failed to open file for writing");
        return 1; // Indicate failure
    }

    // Write the page content in the required format
    fprintf(fp, "%s\n", webpage_getURL(pagep));
    fprintf(fp, "%d\n", webpage_getDepth(pagep));
    fprintf(fp, "%d\n", webpage_getHTMLlen(pagep));
    fprintf(fp, "%s", webpage_getHTML(pagep));

    fclose(fp);
    return 0; // Indicate success
}


/*
 * pageload -- loads the numbered filename <id> in direcory <dirnm>
 * into a new webpage
 *
 * returns: non-NULL for success; NULL otherwise
 */
webpage_t *pageload(int id, char *dirnm) {
    char filepath[256];
    sprintf(filepath, "%s/%d", dirnm, id);

    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        // This is an expected failure if the file doesn't exist,
        // so we don't need to print a big error.
        return NULL;
    }

    char url[1000]; // Buffer for URL
    int depth;
    int html_len;

    // Read URL (using %999s to prevent buffer overflow)
    if (fscanf(fp, "%999s\n", url) != 1) {
        fprintf(stderr, "Error: pageload failed to read URL from %s.\n", filepath);
        fclose(fp);
        return NULL;
    }

    // Read depth
    if (fscanf(fp, "%d\n", &depth) != 1) {
        fprintf(stderr, "Error: pageload failed to read depth from %s.\n", filepath);
        fclose(fp);
        return NULL;
    }

    // Read HTML length
    if (fscanf(fp, "%d\n", &html_len) != 1) {
        fprintf(stderr, "Error: pageload failed to read HTML length from %s.\n", filepath);
        fclose(fp);
        return NULL;
    }

    // Allocate memory for HTML (+1 for the null-terminator)
    char *html = malloc(html_len + 1);
    if (html == NULL) {
        fprintf(stderr, "Error: pageload failed to allocate memory for HTML.\n");
        fclose(fp);
        return NULL;
    }

    // Read the rest of the file (the HTML)
    if (fread(html, sizeof(char), html_len, fp) != html_len) {
        fprintf(stderr, "Error: pageload failed to read HTML content from %s.\n", filepath);
        free(html);
        fclose(fp);
        return NULL;
    }
    html[html_len] = '\0'; // Ensure it's null-terminated

    fclose(fp);

    // Create the webpage. 
    // webpage_new copies the URL, but takes ownership of the html pointer.
    webpage_t *page = webpage_new(url, depth, html);
    if (page == NULL) {
        free(html); // If creation fails, we must free the html we allocated
    }

    return page;
}
