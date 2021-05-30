/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "utils/cxutils.h"

//src
int _cxml_read_file(const char *file_name, char **dest_buffer) {
#define BUFF_SIZE (4096)
    if (!file_name || !dest_buffer) return 0;
    FILE *file = fopen(file_name, "r");
    if (!file){
        cxml_error("Could not open file (%s): <errno: %d>\n", file_name, errno);
    }
    fseek(file, 0L, SEEK_END);
    size_t bytecount = ftell(file);
    rewind(file);
    if (bytecount == 0){
        fclose(file);
        cxml_error("Perhaps \"%s\" is an empty file?\n", file_name);
    }
    *dest_buffer = ALLOCR(char, (bytecount+1), "Not enough memory to read file (%s)\n", file_name);
    setvbuf(file, NULL, _IOFBF, BUFF_SIZE);

    if (fread((*dest_buffer), sizeof(char), bytecount, file) < bytecount) {
        if (ferror(file) != 0){
            clearerr(file);
        }
        fclose(file);
        cxml_error("Error reading file (%s): <errno: %d>\n", file_name, errno);
    }
    (*dest_buffer)[bytecount] = '\0';
    fclose(file);
    return 1;
#undef BUFF_SIZE
}

int _cxml_write_file(const char *file_name, const char *dest_buffer, size_t len){
    if (!file_name || !dest_buffer) return 0;
    FILE *file = fopen(file_name, "w");
    if (!file){
        cxml_error("Could not open file (%s): <errno: %d>\n", file_name, errno);
    }
    if (fwrite(dest_buffer, sizeof(char), len, file) < len){
        if (ferror(file) != 0){
            clearerr(file);
        }
        fclose(file);
        cxml_error("Error writing to file (%s): <errno: %d>\n", file_name, errno);
    }
    fclose(file);
    return 1;
}
