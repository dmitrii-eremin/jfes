/**
    \file       jfes.c
    \author     Eremin Dmitry (http://github.com/NeonMercury)
    \date       October, 2015
    \brief      Json For Embedded Systems library source code.
*/

#include "jfes.h"

/**
    Memory comparing function.

    \param[in]      p1                  Pointer to the first buffer.
    \param[in]      p2                  Pointer to the second buffer.
    \param[in]      count               Number of bytes to compare.

    \return         Zero if all bytes are equal.
*/
static int jfes_memcmp(const void *p1, const void *p2, jfes_size_t count) {
    int delta = 0;

    unsigned char *ptr1 = (unsigned char *)p1;
    unsigned char *ptr2 = (unsigned char *)p2;

    while (count-- > 0 && delta == 0) {
        delta = *(ptr1++) - *(ptr2++);
    }

    return delta;
}

/**
    Copies memory. Doesn't support overlapping.

    \param[out]     dst                 Output memory block.
    \param[in]      src                 Input memory block.
    \param[in]      count               Bytes count to copy.

    \return         Pointer to the destination memory.
*/
static const void *jfes_memcpy(const void *dst, const void *src, jfes_size_t count) {
    unsigned char *destination  = (unsigned char *)dst;
    unsigned char *source       = (unsigned char *)src;
    while (count-- > 0) {
        *(destination++) = *(source++);
    }

    return dst;
}

/** 
    Allocates jfes_string.

    \param[in]      config              JFES configuration.
    \param[out]     str                 String to be allocated.
    \param[in]      size                Size to allocate.

    \return         jfes_success if everything is OK.
*/
static jfes_status_t jfes_allocate_string(jfes_config_t *config, jfes_string_t *str, jfes_size_t size) {
    if (!config || !str || size == 0) {
        return jfes_invalid_arguments;
    }

    str->size = size;
    str->data = config->jfes_malloc(str->size);

    return jfes_success;
}

/**
    Creates string object.

    \param[in]      config              JFES configuration.
    \param[out]     str                 String to be created.
    \param[in]      string              Initial string value.
    \param[in]      size                Initial string length.

    \return         jfes_success if everything is OK.
*/
static jfes_status_t jfes_create_string(jfes_config_t *config, jfes_string_t *str, const char *string, jfes_size_t size) {
    if (!config || !str || !string || size == 0) {
        return jfes_invalid_arguments;
    }

    jfes_status_t status = jfes_allocate_string(config, str, size + 1);
    if (jfes_status_is_bad(status)) {
        return status;
    }

    jfes_memcpy(str->data, string, size);
    str->data[size] = '\0';

    return jfes_success;
}

/**
    Finds length of the null-terminated string.

    \param[in]      data                Null-terminated string.

    \return         Length if the null-terminated string.
*/
static jfes_size_t jfes_strlen(const char *data) {
    if (!data) {
        return 0;
    }

    const char *p = data;
    while (*p++);

    return (jfes_size_t)(p - data);
}

/**
    Analyzes input string on the subject of whether it is an boolean.

    \param[in]      data                Input string.
    \param[in]      length              Length if the input string.

    \return         Zero, if input string not an boolean. Otherwise anything.
*/
static int jfes_is_boolean(const char *data, jfes_size_t length) {
    if (!data || length < 4) {
        return 0;
    }

    return  jfes_memcmp(data, "true", 4) == 0 ||
            jfes_memcmp(data, "false", 5) == 0;
}

/**
    Analyzes input string on the subject of whether it is an integer.

    \param[in]      data                Input string.
    \param[in]      length              Length if the input string.

    \return         Zero, if input string not an integer. Otherwise anything.
*/
static int jfes_is_integer(const char *data, jfes_size_t length) {
    if (!data || length == 0) {
        return 0;
    }

    int offset = 0;
    if (data[0] == '-') {
        offset = 1;
    }

    for (jfes_size_t i = offset; i < length; i++) {
        if (data[i] < (int)'0' || data[i] > (int)'9') {
            return 0;
        }
    }

    return 1;
}

/**
    Analyzes input string on the subject of whether it is an double.

    \param[in]      data                Input string.
    \param[in]      length              Length if the input string.

    \return         Zero, if input string not an double. Otherwise anything.
*/
static int jfes_is_double(const char *data, jfes_size_t length) {
    if (!data || length == 0) {
        return 0;
    }

    int offset = 0;
    if (data[0] == '-') {
        offset = 1;
    }

    int dot_already_been = 0;
    int exp_already_been = 0;

    for (jfes_size_t i = offset; i < length; i++) {
        if (data[i] < (int)'0' || data[i] > (int)'9') {
            if (data[i] == '.' && !dot_already_been) {
                dot_already_been = 1;
                continue;
            }
            else if ((data[i] == 'e' || data[i] == 'E') && i + 2 < length &&
                (data[i + 1] == '+' || data[i + 1] == '-') && !exp_already_been) {
                exp_already_been = 1;
                i++;
                continue;
            }

            return 0;
        }
    }

    return 1;
}

/**
    Analyzes string and returns its boolean value.

    \param[in]      data                String to analysis.
    \param[in]      length              String length.

    \return         1, if data == 'true'. Otherwise 0.
*/
static int jfes_string_to_boolean(const char *data, jfes_size_t length) {
    if (!data || length < 4) {
        return 0;
    }

    if (jfes_memcmp(data, "true", 4) == 0) {
        return 1;
    }

    return 0;
}

/**
    Analyses string and returns its integer value.

    \param[in]      data                String to analysis.
    \param[in]      length              String length.

    \return         Integer representation of the input data.
*/
static int jfes_string_to_integer(const char *data, jfes_size_t length) {
    if (!data || length == 0) {
        return 0;
    }

    int result = 0;
    int sign = 1;

    jfes_size_t offset = 0;

    if (data[0] == '-') {
        sign = -1;
        offset = 1;
    }

    for (jfes_size_t i = offset; i < length; i++) {
        char c = data[i];
        if (c >= '0' && c <= '9') {
            result = result * 10 + (c - '0');
        }
    }

    return result * sign;
}

/**
    Analyses string and returns its double value.

    \param[in]      data                String to analysis.
    \param[in]      length              String length.

    \return         Double representation of the input data.
*/
static double jfes_string_to_double(const char *data, jfes_size_t length) {
    if (!data || length == 0) {
        return 0.0;
    }

    double result = 0.0;
    double sign = 1.0;

    int after_dot = 0;
    int exp = 0;

    int direction = 0;

    jfes_size_t index = 0;

    if (data[0] == '-') {
        sign = -1.0;
        index = 1;
    }

    for (; index < length; index++) {
        char c = data[index];
        if (c >= '0' && c <= '9') {
            result = result * 10.0 + (c - '0');
            if (after_dot) {
                exp--;
            }
        } 
        else if (c == '.') {
            after_dot = 1;
            continue;
        }
        else if (index + 2 < length && (c == 'e' || c == 'E')) {
            index++;
            if (data[index] == '+') {
                direction = 1;
                index++;
            }
            else if (data[index] == '-') {
                direction = -1;
                index++;
            }
            break;
        }
    }

    if (index < length) {
        if (jfes_is_integer(data + index, length - index)) {
            int new_exp = jfes_string_to_integer(data + index, length - index);
            exp += direction * new_exp;
        }
    }

    while (exp < 0) {
        result /= 10.0;
        exp++;
    }
    while (exp > 0) {
        result *= 10.0;
        exp--;
    }

    return sign * result;
}

/**
    Allocates a fresh unused token from the token pool.

    \param[in, out] parser              Pointer to the jfes_parser_t object.
    \param[in, out] tokens              Tokens array.
    \param[in]      max_tokens          Maximal tokens count.

    \return         Pointer to an allocated token from token pool.
*/
static jfes_token_t *jfes_allocate_token(jfes_parser_t *parser, jfes_token_t *tokens, jfes_size_t max_tokens) {
    if (!parser || !tokens || max_tokens == 0 || parser->next_token >= max_tokens) {
        return JFES_NULL;
    }
    
    jfes_token_t *token = &tokens[parser->next_token++];
    token->start = token->end = -1;
    token->size = 0;

    return token;
}

/** 
    Analyzes the source string and returns most likely type.

    \param[in]      data                Source string bytes.
    \param[in]      length              Source string length.

    \return         Most likely token type or `jfes_undefined`.
*/
static jfes_token_type_t jfes_get_token_type(const char *data, jfes_size_t length) {
    if (!data || length == 0) {
        return jfes_undefined;
    }

    jfes_token_type_t type = jfes_undefined;
    if (jfes_is_boolean(data, length)) {
        return jfes_boolean;
    }
    else if (jfes_is_integer(data, length)) {
        return jfes_integer;
    }
    else if (jfes_is_double(data, length)) {
        return jfes_double;
    }
   
    return jfes_undefined;
}

/**
    Fills token type and boundaries.

    \param[in, out] token               Pointer to the token to fill.
    \param[in]      type                Token type.
    \param[in]      start               Token boundary start.
    \param[in]      end                 Token boundary end.
*/
static void jfes_fill_token(jfes_token_t *token, jfes_token_type_t type, int start, int end) {
    if (token) {
        token->type = type;
        token->start = start;
        token->end = end;
        token->size = 0;
    }
}

/**
    Fills next available token with JSON primitive.

    \param[in, out] parser              Pointer to the jfes_parser_t object.
    \param[in]      json                JSON data string.
    \param[in]      length              JSON data length.
    \param[out]     tokens              Tokens array to fill.
    \param[in]      max_tokens_count    Maximal count of tokens in tokens array.

    \return         jfes_success if everything is OK.
*/
static jfes_status_t jfes_parse_primitive(jfes_parser_t *parser, const char *json, jfes_size_t length,
    jfes_token_t *tokens, jfes_size_t max_tokens_count) {
    if (!parser || !json || length == 0 || !tokens || max_tokens_count == 0) {
        return jfes_invalid_arguments;
    }

    int found = 0;

    char c = '\0';
    jfes_size_t start = parser->pos;
    while (length && json[parser->pos] != '\0') {
        c = json[parser->pos];
        if (c == '\t' || c == '\n' || c == '\r' || c == ' ' ||
            c == ',' || c == ']' || c == '}'
#ifndef JFES_STRICT
            || c == ':'
#endif
            ) {
            found = 1;
            break;
        }

        parser->pos++;
    }

#ifdef JFES_STRICT
    if (!found) {
        parser->pos = start;
        return jfes_error_part;
    }
#endif

    jfes_token_t *token = jfes_allocate_token(parser, tokens, max_tokens_count);
    if (!token) {
        parser->pos = start;
        return jfes_no_memory;
    }

    jfes_size_t token_length = parser->pos - start;
    jfes_token_type_t type = jfes_get_token_type(json + start, token_length);

    jfes_fill_token(token, type, start, parser->pos);
    parser->pos--;

    return jfes_success;
}

/**
    Fills next available token with JSON string.

    \param[in, out] parser              Pointer to the jfes_parser_t object.
    \param[in]      json                JSON data string.
    \param[in]      length              JSON data length.
    \param[out]     tokens              Tokens array to fill.
    \param[in]      max_tokens_count    Maximal count of tokens in tokens array.

    \return         jfes_success if everything is OK.
*/
static jfes_status_t jfes_parse_string(jfes_parser_t *parser, const char *json, jfes_size_t length,
    jfes_token_t *tokens, jfes_size_t max_tokens_count) {
    if (!parser || !json || length == 0 || !tokens || max_tokens_count == 0) {
        return jfes_invalid_arguments;
    }

    jfes_size_t start = parser->pos++;
    while (parser->pos < length && json[parser->pos] != '\0') {
        char c = json[parser->pos];
        if (c == '\"') {
            jfes_token_t *token = jfes_allocate_token(parser, tokens, max_tokens_count);
            if (!token) {
                parser->pos = start;
                return jfes_no_memory;
            }

            jfes_fill_token(token, jfes_string, start + 1, parser->pos);
            return jfes_success;
        }
        else if (c == '\\' && parser->pos + 1 < length) {
            parser->pos++;
            switch (json[parser->pos]) {
            case '\"': case '/': case '\\': case 'b': case 'f':
            case 'r': case 'n': case 't':
                break;

            case 'u':
                parser->pos++;
                for (jfes_size_t i = 0;  i < 4 && parser->pos < length && json[parser->pos] != '\0'; i++, parser->pos++) {
                    char symbol = json[parser->pos];
                    if ((symbol < (int)'0' || symbol > (int)'9') &&
                        (symbol < (int)'A' || symbol > (int)'F') &&
                        (symbol < (int)'a' || symbol > (int)'f')) {
                        parser->pos = start;
                        return jfes_invalid_input;
                    }
                }
                parser->pos--;
                break;
            default:
                parser->pos = start;
                return jfes_invalid_input;
            }
        }
        parser->pos++;
    }

    parser->pos = start;
    return jfes_error_part;
}

int jfes_status_is_good(jfes_status_t status) {
    return status == jfes_success;
}

int jfes_status_is_bad(jfes_status_t status) {
    return !jfes_status_is_good(status);
}

jfes_status_t jfes_init_parser(jfes_parser_t *parser, jfes_config_t *config) {
    if (!parser || !config) {
        return jfes_invalid_arguments;
    }

    parser->config = config;

    return jfes_reset_parser(parser);
}

jfes_status_t jfes_reset_parser(jfes_parser_t *parser) {
    if (!parser) {
        return jfes_invalid_arguments;
    }

    parser->pos = 0;
    parser->next_token = 0;
    parser->superior_token = -1;

    return jfes_success;
}

jfes_status_t jfes_parse_tokens(jfes_parser_t *parser, const char *json,
    jfes_size_t length, jfes_token_t *tokens, jfes_size_t *max_tokens_count) {

    if (!parser || !json || length == 0 || !tokens || !max_tokens_count || *max_tokens_count == 0) {
        return jfes_invalid_arguments;
    }

    jfes_reset_parser(parser);

    jfes_token_t *token = JFES_NULL;

    jfes_size_t count = parser->next_token;
    while (parser->pos < length && json[parser->pos] != '\0') {
        char c = json[parser->pos];
        switch (c) {
        case '{': case '[':
            {
                count++;
                token = jfes_allocate_token(parser, tokens, *max_tokens_count);
                if (!token) {
                    return jfes_no_memory;
                }
                if (parser->superior_token != -1) {
                    tokens[parser->superior_token].size++;
                }

                token->type = (c == '{' ? jfes_object : jfes_array);
                token->start = parser->pos;
                parser->superior_token = parser->next_token - 1;
            }
            break;

        case '}': case ']':
            {
                jfes_token_type_t type = (c == '}' ? jfes_object : jfes_array);

                int i = 0;
                for (i = (int)parser->next_token - 1; i >= 0; i--) {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        parser->superior_token = -1;
                        token->end = parser->pos + 1;
                        break;
                    }
                }

                if (i == -1) {
                    return jfes_invalid_input;
                }

                while (i >= 0) {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        parser->superior_token = i;
                        break;
                    }
                    i--;
                }
            }
            break;

        case '\"':
            {
                jfes_status_t status = jfes_parse_string(parser, json, length, tokens, *max_tokens_count);
                if (jfes_status_is_bad(status)) {
                    return status;
                }

                count++;

                if (parser->superior_token != -1 && tokens != JFES_NULL) {
                    tokens[parser->superior_token].size++;
                }
            }
            break;

        case '\t': case '\r': case '\n': case ' ':
            break;

        case ':':
            parser->superior_token = parser->next_token - 1;
            break;

        case ',':
            {
                if (parser->superior_token != -1 &&
                    tokens[parser->superior_token].type != jfes_array &&
                    tokens[parser->superior_token].type != jfes_object) {
                    for (int i = parser->next_token - 1; i >= 0; i--) {
                        if (tokens[i].type == jfes_array || tokens[i].type == jfes_object) {
                            if (tokens[i].start != -1 && tokens[i].end == -1) {
                                parser->superior_token = i;
                                break;
                            }
                        }
                    }
                }
            }
            break;

#ifdef JFES_STRICT
        case '-': case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 't': case 'f': case 'n':
            if (parser->superior_token != -1) {
                jfes_token_t *token = &tokens[parser->superior_token];
                if (token->type == jfes_object || (token->type == jfes_string && token->size != 0)) {
                    return jfes_invalid_input;
                }
            }
#else
        default:
#endif
            {
                jfes_status_t status = jfes_parse_primitive(parser, json, length, tokens, *max_tokens_count);
                if (jfes_status_is_bad(status)) {
                    return status;
                }

                count++;
                if (parser->superior_token != -1) {
                    tokens[parser->superior_token].size++;
                }
            }
            break;
#ifdef JFES_STRICT
        default:
            return jfes_invalid_input;
#endif
        }
        parser->pos++;
    }

    for (int i = (int)parser->next_token - 1; i >= 0; i--) {
        if (tokens[i].start != -1 && tokens[i].end == -1) {
            return jfes_error_part;
        }
    }

    *max_tokens_count = count;
    return jfes_success;
}

/**
    Creates jfes value node from tokens sequence.

    \param[in]      tokens_data         Pointer to the jfes_tokens_data_t object.
    \param[out]     value               Pointer to the value to create node.

    \return         jfes_success if everything is OK.
*/
jfes_status_t jfes_create_node(jfes_tokens_data_t *tokens_data, jfes_value_t *value) {
    if (!tokens_data || !value) {
        return jfes_invalid_arguments;
    }

    if (tokens_data->current_token >= tokens_data->tokens_count) {
        return jfes_invalid_arguments;
    }

    jfes_malloc_t jfes_malloc   = tokens_data->config->jfes_malloc;
    jfes_free_t   jfes_free     = tokens_data->config->jfes_free;

    jfes_token_t *token = &tokens_data->tokens[tokens_data->current_token];
    tokens_data->current_token++;
    
    value->type = (jfes_value_type_t)token->type;

    switch (token->type) {
    case jfes_boolean:
        value->data.bool_val = jfes_string_to_boolean(tokens_data->json_data + token->start, 
            token->end - token->start);
        break;

    case jfes_integer:
        value->data.int_val = jfes_string_to_integer(tokens_data->json_data + token->start,
            token->end - token->start);
        break;

    case jfes_double:
        value->data.double_val = jfes_string_to_double(tokens_data->json_data + token->start,
            token->end - token->start);
        break;

    case jfes_string:
        jfes_create_string(tokens_data->config, &value->data.string_val, 
            tokens_data->json_data + token->start, token->end - token->start);
        break;

    case jfes_array:
        value->data.array_val = jfes_malloc(sizeof(jfes_array_t));
        if (token->size > 0) {
            value->data.array_val->count = token->size;
            value->data.array_val->items = jfes_malloc(token->size * sizeof(jfes_value_t*));

            jfes_status_t status = jfes_success;

            for (jfes_size_t i = 0; i < token->size; i++) {
                jfes_value_t *item = jfes_malloc(sizeof(jfes_value_t));
                value->data.array_val->items[i] = item;

                status = jfes_create_node(tokens_data, item);
                if (jfes_status_is_bad(status)) {
                    value->data.array_val->count = i + 1;

                    jfes_free_value(tokens_data->config, value);
                    return status;
                }
            }
        }
        break;

    case jfes_object:
        value->data.object_val = jfes_malloc(sizeof(jfes_object_t));
        if (token->size > 0) {
            value->data.object_val->count = token->size;
            value->data.object_val->items = jfes_malloc(token->size * sizeof(jfes_object_map_t*));

            jfes_status_t status = jfes_success;

            for (jfes_size_t i = 0; i < token->size; i++) {
                jfes_object_map_t *item = jfes_malloc(sizeof(jfes_object_map_t));
                value->data.object_val->items[i] = item;

                jfes_token_t *key_token = &tokens_data->tokens[tokens_data->current_token++];
                
                jfes_size_t key_length = key_token->end - key_token->start;

                jfes_create_string(tokens_data->config, &item->key, 
                    tokens_data->json_data + key_token->start, key_length);

                item->value = jfes_malloc(sizeof(jfes_value_t));

                status = jfes_create_node(tokens_data, item->value);
                if (jfes_status_is_bad(status)) {
                    value->data.object_val->count = i + 1;

                    jfes_free_value(tokens_data->config, value);
                    return status;
                }
            }
        }
        break;

    default:
        return jfes_unknown_type;
    }
    return jfes_success;
}

jfes_status_t jfes_parse_to_value(jfes_config_t *config, const char *json,
    jfes_size_t length, jfes_value_t *value) {
    if (!config || !json || length == 0 || !value) {
        return jfes_invalid_arguments;
    }

    jfes_parser_t parser;
    jfes_status_t status = jfes_init_parser(&parser, config);
    if (jfes_status_is_bad(status)) {
        return status;
    }

    jfes_size_t tokens_count = 1024;
    jfes_token_t *tokens = JFES_NULL;

    status = jfes_no_memory;
    while (status == jfes_no_memory && tokens_count <= JFES_MAX_TOKENS_COUNT) {
        jfes_reset_parser(&parser);

        tokens = parser.config->jfes_malloc(tokens_count * sizeof(jfes_token_t));

        long current_tokens_count = tokens_count;
        status = jfes_parse_tokens(&parser, json, length, tokens, &current_tokens_count);
        if (jfes_status_is_good(status)) {
            tokens_count = current_tokens_count;
            break;
        }

        tokens_count *= 2;
        parser.config->jfes_free(tokens);
    }
    
    if (jfes_status_is_bad(status)) {
        return status;
    }

    jfes_tokens_data_t tokens_data = {
        config,
        json, length,
        tokens, tokens_count, 0
    };

    status = jfes_create_node(&tokens_data, value);

    parser.config->jfes_free(tokens);
    return jfes_success;
}

jfes_status_t jfes_free_value(jfes_config_t *config, jfes_value_t *value) {
    if (!config || !value) {
        return jfes_invalid_arguments;
    }

    if (value->type == jfes_array) {
        if (value->data.array_val->count > 0) {
            for (jfes_size_t i = 0; i < value->data.array_val->count; i++) {
                jfes_value_t *item = value->data.array_val->items[i];
                jfes_free_value(config, item);
                config->jfes_free(item);
            }

            config->jfes_free(value->data.array_val->items);
        }

        config->jfes_free(value->data.array_val);
    }
    else if (value->type == jfes_object) {
        if (value->data.object_val->count > 0) {
            for (jfes_size_t i = 0; i < value->data.object_val->count; i++) {
                jfes_object_map_t *object_map = value->data.object_val->items[i];

                config->jfes_free(object_map->key.data);

                jfes_free_value(config, object_map->value);
                config->jfes_free(object_map->value);

                config->jfes_free(object_map);
            }

            config->jfes_free(value->data.object_val->items);
        }

        config->jfes_free(value->data.object_val);
    }
    else if (value->type == jfes_string) {
        if (value->data.string_val.size > 0) {
            config->jfes_free(value->data.string_val.data);
        }
    }

    return jfes_success;
}