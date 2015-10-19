/**
    \file       example_1.c
    \author     Eremin Dmitry (http://github.com/NeonMercury)
    \date       October, 2015
    \brief      This example shows loading *.json file into the application.
*/

#include "jfes.h"

#include <stdio.h>
#include <stdlib.h>

/** Entry point. */
int main(int argc, char **argv) {
    FILE *f = fopen("samples/example_1.json", "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *json_data = malloc((fsize + 1) * sizeof(char));
    fread(json_data, fsize, sizeof(char), f);
    json_data[fsize] = '\0';
    fclose(f);

    jfes_parser_t parser;
    jfes_token_t tokens[1024];
    jfes_size_t tokens_count = 1024;

    jfes_init_parser(&parser, malloc, free);
    jfes_status_t status = jfes_parse_tokens(&parser, json_data, fsize, tokens, &tokens_count);

    free(json_data);
    return 0;
}