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
    if ((length == 4 && jfes_memcmp(data, "true", 4) == 0) ||
        (length == 5 && jfes_memcmp(data, "false", 5) == 0)) {
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

jfes_status_t jfes_init_parser(jfes_parser_t *parser, jfes_malloc_t malloc, jfes_free_t free) {
    if (!parser || !malloc || !free) {
        return jfes_invalid_arguments;
    }

    parser->jfes_malloc = malloc;
    parser->jfes_free = free;

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