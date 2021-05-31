/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXSTREAM_H
#define CXML_CXSTREAM_H

#include "core/cxmem.h"

typedef struct {
    // is the lexer in an open state?
    bool _is_open;

    // stream buffer
    char *_stream_buff;

    // current size
    size_t _chunk_curr_size;

    // start size
    size_t _chunk_start_size;

    // The number of bytes read from file into the stream buffer
    // also serves as an index into the stream buffer
    size_t _nbytes_read_into_sbuff;

    // current file being processed.
    FILE *_file;

    // name of current file being processed.
    const char *file_name;
} _cxml_stream;


/*
 * stream utility functions
 */

void _cxml_stream_init(
        _cxml_stream *stream_obj,
        const char *filename,
        size_t chunk_size);

void _cxml__open_stream(
        _cxml_stream *stream,
        const char *fn,
        size_t chunk_size);

void _cxml__close_stream(_cxml_stream *stream);

#endif //CXML_CXSTREAM_H
