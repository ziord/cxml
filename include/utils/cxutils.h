/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXUTILS_H
#define CXML_CXUTILS_H
#include <errno.h>
#include "core/cxmem.h"

/*
 * cxml utility functions
 */


int _cxml_read_file(const char *file_name, char **dest_buffer);

int _cxml_write_file(const char *file_name, const char *dest_buffer, size_t len);

#endif //CXML_CXUTILS_H
