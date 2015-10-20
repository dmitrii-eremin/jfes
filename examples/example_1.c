/**
    \file       example_1.c
    \author     Eremin Dmitry (http://github.com/NeonMercury)
    \date       October, 2015
    \brief      Loading *.json file into the application and tokenizing it.
*/

#include "../jfes.h"
#include "examples.h"

/* For malloc and free functions. You can use your own memory functions. */
#include <stdlib.h>

/** Example 1 entry point. */
int example_1_entry(int argc, char **argv) {
    
    long buffer_size = 2048;
    char *json_data = malloc(buffer_size * sizeof(char));

    if (!get_file_content("samples/example_1.json", json_data, &buffer_size)) {
        return -1;
    }

    jfes_parser_t parser;
    jfes_token_t tokens[1024];
    jfes_size_t tokens_count = 1024;

    jfes_config_t config;
    config.jfes_malloc = malloc;
    config.jfes_free = free;

    jfes_init_parser(&parser, &config);
    jfes_status_t status = jfes_parse_tokens(&parser, json_data, buffer_size, tokens, &tokens_count);

    free(json_data);
    return 0;
}