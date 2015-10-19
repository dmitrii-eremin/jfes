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
    return jfes_success;
}