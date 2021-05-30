/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include <errno.h>
#include "xml/cxlexer.h"

#define _cxml__def_chunk_size   (0x100000)


void _cxml__open_stream(_cxml_stream *stream, const char *fn, size_t chunk_size) {
    FILE *file = fopen(fn, "r");
    if (!file){
        cxml_error("Could not open file (%s): <errno: %d>\n", fn, errno);
    }
    fseek(file, 0L, SEEK_SET);
    stream->_file = file;
    char* buff = CALLOCR(char, chunk_size, "Not enough memory to read file (%s)\n", fn);
    stream->_stream_buff = buff;
}

void _cxml_stream_init(_cxml_stream* stream_obj,
                       const char *filename,
                       size_t chunk_size)
{
    if (filename){
        // store the current chunk (starting) size, so that it can be used in resetting stream buffer
        stream_obj->_chunk_start_size = chunk_size < 10 ? _cxml__def_chunk_size : chunk_size;
        stream_obj->_chunk_curr_size = stream_obj->_chunk_start_size;
        _cxml__open_stream(stream_obj, filename, stream_obj->_chunk_curr_size);
        stream_obj->_is_open = 1;
        stream_obj->_nbytes_read_into_sbuff = 0;
        stream_obj->file_name = filename;
    }
}

void _cxml__close_stream(_cxml_stream *stream) {
    stream->_is_open = 0;
    stream->_chunk_curr_size ? FREE(stream->_stream_buff) : (void)0;
    stream->_file ? fclose(stream->_file) : 0;
}

void _cxml__adjust_stream_buffer(_cxml_lexer *cxlexer) {
    _cxml_stream* stream = &cxlexer->_stream_obj;
    size_t new_size = stream->_chunk_curr_size + stream->_chunk_start_size;
    // save the lexer's `current` and `start` (char) pointer position so we don't
    // lose it after resizing the buffer
    size_t curr_pos = (cxlexer->current - stream->_stream_buff);
    size_t start_pos = (cxlexer->start - stream->_stream_buff);
    char* n_buff = RALLOCR(char, stream->_stream_buff, new_size,
                           "Not enough memory to continue "
                           "streaming file: '%s'\n", stream->file_name);
    stream->_stream_buff = n_buff;
    stream->_chunk_curr_size = new_size;
    // reassign lexer's `start` and `current` (char) pointer to its original
    // position before resize happened.
    cxlexer->current = stream->_stream_buff + curr_pos;
    cxlexer->start = stream->_stream_buff + start_pos;
}

void _cxml__reset_stream_buffer(_cxml_lexer *cxlexer) {
    /*      x_ _ _ _ _ _ _ _ _curr _ _ _ _ _end _
     *
     *      x                -> beginning of the stream buffer
     *      curr             -> position of cxlexer.current in the stream buffer
     *      end              -> end of stream buffer
     *
     * _nbytes_read_into_sbuff - measures the number of bytes already
     * read from file into the stream buffer, and indexes into the stream buffer.
     *
     * if we adjusted the stream buffer in the if block, then we would need to adjust the
     * chunk ptr accordingly because it serves as a form of index into the stream buffer.
     * Hence the cases to be taken care of:
     * used       --> (curr - stream->_stream_buff)
     * total read --> (stream->_nbytes_read_into_sbuff)
     *
     */

    _cxml_stream* stream = &cxlexer->_stream_obj;

    // * create cpy_size, for storing the amount of bytes to be copied into the stream buffer.
    // store old_size as the steam's current chunk size.
    size_t cpy_size;

    // curr points to a later part of the stream buffer
    const char* curr = cxlexer->current;
    char* tmp = NULL;

    int consumed_bytes_count = (int)(curr - stream->_stream_buff);

     /*
      * use cpy_size to determine the amount of bytes to be copied.
      * b<----------->curr|<--cpy_size--->
      * b<----------------|-------------->(stream->_stream_buff + stream->_chunk_curr_size)<->total read
      * b<---------total allocated------->(stream->_stream_buff + stream->_chunk_curr_size)
      */
    cpy_size = ((stream->_stream_buff + stream->_chunk_curr_size) - curr);

    // cut down the stream buffer to the new reduced size if the buffer's _chunk_curr_size
    // surpasses the initially chosen size which is _chunk_start_size
    if (stream->_chunk_curr_size > stream->_chunk_start_size){
        /*
         * use new_size to determine the number of bytes to resize the stream buffer into
         * set new_size to _chunk_start_size (which is the originally set stream size) if less,
         * to keep the stream buffer size as close as possible to the size originally specified.
         */
        size_t new_size = cpy_size < stream->_chunk_start_size ? stream->_chunk_start_size : cpy_size;

         /*
          * malloc call here cuts down the stream_buff size (stream->_chunk_curr_size)
          * we use malloc directly and not via ALLOC because ALLOC would end the program
          * on allocation failure, but we want to keep it going at this point
          */
        tmp = malloc(new_size);
        // if malloc works, we need to adjust the stream's chunk size, and buffer
        // to its new size.
        if (tmp){
            memcpy(tmp, curr, cpy_size);
            FREE(stream->_stream_buff); // free old buffer
            stream->_stream_buff = tmp;
            stream->_chunk_curr_size = new_size;
        }else{
            /*
             * if malloc fails, we would temporarily be unable to trim down the
             * stream buffer's size. so we choose instead to just move the valid text
             * to the beginning of the stream, though there's a possibility that
             * memmove() would fail as well.
             * copy from `curr` downwards.., into the beginning/start of stream buffer
             */
            memmove(stream->_stream_buff, curr, cpy_size);
        }
    }
    else{
        // copy from `curr` downwards.., into the beginning/start of stream buffer
        memmove(stream->_stream_buff, curr, cpy_size);
    }
    stream->_nbytes_read_into_sbuff = stream->_nbytes_read_into_sbuff - consumed_bytes_count;
    cxlexer->current = cxlexer->start = stream->_stream_buff;
}
