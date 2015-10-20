/**
\file       examples.h
\author     Eremin Dmitry (http://github.com/NeonMercury)
\date       October, 2015
\brief      Header file for declarations of examples entry points.
*/

#ifndef EXAMPLE_1_H_INCLUDE_GUARD
#define EXAMPLE_1_H_INCLUDE_GUARD

/** Entry point for example_1. */
int example_1_entry(int argc, char **argv);

/** Entry point for example_2. */
int example_2_entry(int argc, char **argv);

/** 
    Helper function. Loads file content.

    \param[in]      filename            Filename to get content.
    \param[out]     content             Pointer to content buffer.
    \param[in, out] max_content_size    Maximal content buffer size. Returns file size.

    \return         Zero, if function failed. Anything otherwise.
*/
int get_file_content(const char *filename, char *content, long *max_content_size);

#endif