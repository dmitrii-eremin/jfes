/**
    \file       example_2.c
    \author     Eremin Dmitry (http://github.com/NeonMercury)
    \date       October, 2015
    \brief      Loading *.json file into the application and parsing it to the jfes_value.
*/

#include "../jfes.h"
#include "examples.h"

/* For malloc and free functions. You can use your own memory functions. */
#include <stdlib.h>

/** Example 2 entry point. */
int example_2_entry(int argc, char **argv) {
    unsigned long buffer_size = 2048;
    char *json_data = malloc(buffer_size * sizeof(char));

    if (!get_file_content("json/example_2.json", json_data, &buffer_size)) {
        return -1;
    }

    jfes_config_t config;
    config.jfes_malloc = malloc;
    config.jfes_free = free;

    jfes_value_t value;
    jfes_status_t status = jfes_parse_to_value(&config, json_data, buffer_size, &value);

    /*          ...             */
    /* Do something with value. */
    /*          ...             */

    jfes_free_value(&config, &value);
    free(json_data);
    return 0;
}