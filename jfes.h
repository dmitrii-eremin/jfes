/**
    \file       jfes.h
    \author     Eremin Dmitry (http://github.com/NeonMercury)
    \date       October, 2015
    \brief      Json For Embedded Systems library headers.
*/

#ifndef JFES_H_INCLUDE_GUARD
#define JFES_H_INCLUDE_GUARD

/** Strict JSON mode. **/
//#define JFES_STRICT

/** NULL define for the jfes library. */
#ifndef JFES_NULL
#define JFES_NULL               ((void*)0)
#endif

/** size_t type for the jfes library. */
typedef unsigned int jfes_size_t;

/** JFES return statuses. */
typedef enum jfes_status {
    jfes_unknown            = 0x00,             /**< Unknown status */

    jfes_success            = 0x01,             /**< Last operation finished sucessfully. */

    jfes_invalid_arguments  = 0x02,             /**< Invalid arguments were passed to the function. */
    jfes_no_memory          = 0x03,             /**< Not enough tokens were provided. */
    jfes_invalid_input      = 0x04,             /**< Invalid character in JSON string. */
    jfes_error_part         = 0x05,             /**< The string is not a full JSON packet. More bytes expected. */
} jfes_status_t;

/** Memory allocator function type. */
typedef void *(__cdecl *jfes_malloc_t)(jfes_size_t);

/** Memory deallocator function type. */
typedef void (__cdecl *jfes_free_t)(void*);

/** JFES token types */
typedef enum jfes_token_type {
    jfes_undefined          = 0x00,             /**< Undefined token type. */
    
    jfes_boolean            = 0x01,             /**< Boolean token type. */
    jfes_integer            = 0x02,             /**< Integer token type. */
    jfes_double             = 0x03,             /**< Double token type. */
    jfes_string             = 0x04,             /**< String token type. */

    jfes_object             = 0x05,             /**< Object token type. */
    jfes_array              = 0x06              /**< Array token type. */
} jfes_token_type_t;

/** JFES token structure. */
typedef struct jfes_token {
    jfes_token_type_t       type;               /**< Token type. */

    int                     start;              /**< Token start position. */
    int                     end;                /**< Token end position. */
    jfes_size_t             size;               /**< Token children count. */
} jfes_token_t;

/** JFES parser structure. */
typedef struct jfes_parser {
    jfes_size_t             pos;                /**< Current offset in json string. */ 
    jfes_size_t             next_token;         /**< Next token to allocate. */
    int                     superior_token;     /**< Superior token node. */

    jfes_malloc_t           jfes_malloc;        /**< Memory allocation function. */
    jfes_free_t             jfes_free;          /**< Memory deallocation function. */
} jfes_parser_t;

/** 
    JFES status analizer function.

    \param[in]      status              Status variable.

    \return Zero, if status not equals jfes_success. Otherwise anything else.
*/
int jfes_status_is_good(jfes_status_t status);

/**
JFES status analizer function.

\param[in]      status              Status variable.

\return Zero, if status equals jfes_success. Otherwise anything else.
*/
int jfes_status_is_bad(jfes_status_t status);

/**
    JFES parser initialization.

    \param[out]     parser              Pointer to the jfes_parser_t object.
    \param[in]      malloc              Memory allocation function.
    \param[in]      free                Memory deallocation function.

    \return         jfes_success if everything is OK.
*/
jfes_status_t jfes_init_parser(jfes_parser_t *parser, jfes_malloc_t malloc, jfes_free_t free);

/**
    Resets all parser fields, except memory allocation functions.

    \param[out]     parser              Pointer to the jfes_parser_t object.

    \return         jfes_success if everything is OK.
*/
jfes_status_t jfes_reset_parser(jfes_parser_t *parser);

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

#endif