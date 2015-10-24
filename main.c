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

int set_file_content(const char *filename, const char *content, unsigned long content_size) {
    if (!filename || !content || content_size == 0) {
        return 0;
    }

    FILE *f = fopen(filename, "w");
    if (!f) {
        return 0;
    }

    int result = (int)fwrite(content, sizeof(char), content_size, f);
    fclose(f);

    return result;
}

int get_file_content(const char *filename, char *content, unsigned long *max_content_size) {
    if (!filename || !content || !max_content_size || *max_content_size == 0) {
        return 0;
    }

    FILE *f = fopen(filename, "r");
    if (!f) {
        return 0;
    }

    *max_content_size = fread(content, sizeof(char), *max_content_size, f);
    fclose(f);

    return 1;
}
