/**
    \file       example_3.c
    \author     Eremin Dmitry (http://github.com/NeonMercury)
    \date       October, 2015
    \brief      Loading *.json file into the application, parsing it to the jfes_value and editing it.
*/

#include "../jfes.h"
#include "examples.h"

/* For malloc and free functions. You can use your own memory functions. */
#include <stdlib.h>

/** Example 2 entry point. */
int example_3_entry(int argc, char **argv) {
    long buffer_size = 2048;
    char *json_data = malloc(buffer_size * sizeof(char));

    if (!get_file_content("json/example_3.json", json_data, &buffer_size)) {
        return -1;
    }

    jfes_config_t config;
    config.jfes_malloc = malloc;
    config.jfes_free = free;

    jfes_value_t value;
    jfes_status_t status = jfes_parse_to_value(&config, json_data, buffer_size, &value);

    if (jfes_status_is_good(status)) {
        jfes_value_t *children = jfes_get_child(&value, "children", 0);
        if (children) {
            jfes_value_t *child = jfes_create_object_value(&config);

            /* 
                Below we create a property with a key "first_name" and json value "Paul".
                We place this property into the `child` variable.
            */
            jfes_set_object_property(&config, child, jfes_create_string_value(&config, "Paul", 0), "first_name", 0);

            /* The same thing with "middle_name" and "age". */
            jfes_set_object_property(&config, child, jfes_create_string_value(&config, "Smith", 0), "middle_name", 0);
            jfes_set_object_property(&config, child, jfes_create_integer_value(&config, 1), "age", 0);

            /* And now we wants to rewrite age value with 2. */
            jfes_set_object_property(&config, child, jfes_create_integer_value(&config, 2), "age", 0);

            jfes_remove_object_property(&config, child, "middle_name", 0);
            
            /* Place value `child` in `children` json array at index 1. */
            status = jfes_place_to_array_at(&config, children, child, 1);

            jfes_set_object_property(&config, &value, jfes_create_null_value(&config), "null_property", 0);

            /* And now we dumps out new object to the memory. */
            char beauty_dump[1024];
            jfes_size_t dump_size = 1024;
            jfes_value_to_string(&value, &beauty_dump[0], &dump_size, 1);
            beauty_dump[dump_size] = '\0';

            set_file_content("~tmp_example_3.beauty.out.json", beauty_dump, dump_size);

            char ugly_dump[1024];
            dump_size = 1024;
            jfes_value_to_string(&value, &ugly_dump[0], &dump_size, 0);
            ugly_dump[dump_size] = '\0';
            
            set_file_content("~tmp_example_3.ugly.out.json", ugly_dump, dump_size);
        }

        jfes_free_value(&config, &value);
    }

    free(json_data);
    return 0;
}