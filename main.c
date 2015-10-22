/**
\file       main.c
\author     Eremin Dmitry (http://github.com/NeonMercury)
\date       October, 2015
\brief      Examples loader.
*/

#include "jfes.h"
#include "examples/examples.h"

/* Only for file functions. */
#include <stdio.h>

/** Entry point. */
int main(int argc, char **argv) {
    return example_3_entry(argc, argv);
}

int get_file_content(const char *filename, char *content, long *max_content_size) {
    if (!filename || !content || !max_content_size || *max_content_size <= 0) {
        return 0;
    }

    FILE *f = fopen(filename, "r");
    if (!f) {
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    long read_size = fsize;
    if (*max_content_size < read_size) {
        read_size = *max_content_size;
    }

    fread(content, read_size, sizeof(char), f);
    fclose(f);

    *max_content_size = read_size;

    return 1;
}