# JFES
*Based on [jsmn](https://github.com/zserge/jsmn) project.*

Json For Embedded Systems (JFES) is a minimalistic [json](http://www.json.org/) engine, written in plain C. It can be easily integrated into the code for embedded systems.

## Features
* compatible with C99
* no dependencies (I'm seriously!)
* highly portable
* you can use it only like json parser
* incremental single-pass parsing

## API

### Initializing

Before use you need to initialize `jfes_config_t` object.
```
/** JFES config structure. */
typedef struct jfes_config {
    jfes_malloc_t           jfes_malloc;        /**< Memory allocation function. */
    jfes_free_t             jfes_free;          /**< Memory deallocation function. */
} jfes_config_t;
```

Below you can see prototypes of memory management functions:
```
/** Memory allocator function type. */
typedef void *(__cdecl *jfes_malloc_t)(jfes_size_t);

/** Memory deallocator function type. */
typedef void (__cdecl *jfes_free_t)(void*);
```
As you can see, these functions has the same prototype with C functions from standard library.

So, you can initialize JFES configuration with this code:
```
#include <stdlib.h>

/* ...some useful stuff... */

jfes_config_t config;

config.jfes_malloc = malloc;
config.jfes_free = free;
```

But, if you need to use your own memory management functions, use them.

### Parser (optional)

If you need only to parse *.json file without allocating any values (like [jsmn](https://github.com/zserge/jsmn)), you can only parse json string and separate it on tokens.
In this case, you need to use only two functions:
```
/**
    JFES parser initialization.
    \param[out]     parser              Pointer to the jfes_parser_t object.
    \param[in]      config              JFES configuration.
    \return         jfes_success if everything is OK.
*/
jfes_status_t jfes_init_parser(jfes_parser_t *parser, jfes_config_t *config);

/******************************************************************/

/**
    Run JSON parser. It parses a JSON data string into and
    array of tokens, each describing a single JSON object.
    \param[in]      parser              Pointer to the jfes_parser_t object.
    \param[in]      json                JSON data string.
    \param[in]      length              JSON data length.
    \param[out]     tokens              Tokens array to fill.
    \param[in, out] max_tokens_count    Maximal count of tokens in tokens array.
                                        Will contain tokens count.
    \return         jfes_success if everything is OK.
*/
jfes_status_t jfes_parse_tokens(jfes_parser_t *parser, const char *json,
    jfes_size_t length, jfes_token_t *tokens, jfes_size_t *max_tokens_count);
```

You can see parsing example below.

### Loading *.json into value

You can load any json data into `jfes_value_t`.

```
/** JSON value structure. */
struct jfes_value {
    jfes_value_type_t       type;               /**< JSON value type. */
    jfes_value_data_t       data;               /**< Value data. */
};
```

Value type (`jfes_value_type_t`) can be one of this:
* `jfes_boolean`
* `jfes_integer`
* `jfes_double`
* `jfes_string`
* `jfes_array`
* `jfes_object`

And `jfes_value_data_t` is:
```
/** JFES value data union. */
typedef union jfes_value_data {
    int                     bool_val;           /**< Boolean JSON value. */

    int                     int_val;            /**< Integer JSON value. */
    double                  double_val;         /**< Double JSON value. */
    jfes_string_t           string_val;         /**< String JSON value. */

    jfes_array_t            *array_val;         /**< Array JSON value. */
    jfes_object_t           *object_val;        /**< Object JSON value. */
} jfes_value_data_t;
```

You can easily load json string into the value with this code:
```
jfes_config_t config;
config.jfes_malloc = malloc;
config.jfes_free = free;

jfes_value_t value;
jfes_parse_to_value(&config, json_data, json_size, &value);
/* Do something with value */
jfes_free_value(&config, &value);
```
That's all!

### Value modification
You can modify or create `jfes_value_t` with any of these functions:
```
jfes_value_t *jfes_create_boolean_value(jfes_config_t *config, int value);
jfes_value_t *jfes_create_integer_value(jfes_config_t *config, int value);
jfes_value_t *jfes_create_double_value(jfes_config_t *config, double value);
jfes_value_t *jfes_create_string_value(jfes_config_t *config, const char *value, jfes_size_t length);
jfes_value_t *jfes_create_array_value(jfes_config_t *config);
jfes_value_t *jfes_create_object_value(jfes_config_t *config);

jfes_value_t *jfes_get_child(jfes_value_t *value, const char *key, jfes_size_t key_length);
jfes_object_map_t *jfes_get_mapped_child(jfes_value_t *value, const char *key, jfes_size_t key_length);

jfes_status_t jfes_place_to_array(jfes_config_t *config, jfes_value_t *value, jfes_value_t *item);
jfes_status_t jfes_place_to_array_at(jfes_config_t *config, jfes_value_t *value, jfes_value_t *item, jfes_size_t place_at);
jfes_status_t jfes_remove_from_array(jfes_config_t *config, jfes_value_t *value, jfes_size_t index);

jfes_status_t jfes_set_object_property(jfes_config_t *config, jfes_value_t *value, jfes_value_t *item, const char *key, jfes_size_t key_length);
jfes_status_t jfes_remove_object_property(jfes_config_t *config, jfes_value_t *value, const char *key, jfes_size_t key_length);
```

### Serializing to json string

You can serialize any `jfes_value_t` to string with one line (actually, three lines, but two of them is for help):
```
char dump[1024];
jfes_size_t dump_size = 1024;
jfes_value_to_string(&value, beauty_dump, &dump_size, 1);
beauty_dump[dump_size] = '\0';  /* If you need null-terminated string. */
```
`dump_size` will store dump size.
If you pass fourth argument as 1, dump will be beautified. And if zero, dump will be ugly.

## Examples
You can find examples [here](https://github.com/NeonMercury/jfes/tree/master/examples).

## Licence
**The MIT License (MIT)**  
[See full text.](https://github.com/NeonMercury/jfes/blob/master/LICENSE)
