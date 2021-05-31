/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#ifndef CXML_CXLITERAL_H
#define CXML_CXLITERAL_H

#include "cxstr.h"

// literal types
typedef enum {
    CXML_INTEGER_LITERAL,
    CXML_XINTEGER_LITERAL,
    CXML_DOUBLE_LITERAL,    // decimal || double
    CXML_STRING_LITERAL,
    CXML_NON_LITERAL        // for non literal tokens like <, >
} cxml_literal_t;

// for now, we support only NaN, and double number types.
typedef enum {
    CXML_NUMERIC_DOUBLE_T,
    CXML_NUMERIC_NAN_T
}cxml_number_t;

// literal values
typedef struct {
    cxml_number_t type;
    double dec_val;
} cxml_number;

void cxml_number_init(cxml_number *literal);

cxml_number new_cxml_number();

long cxml_literal_to_long(cxml_string *str);

double cxml_literal_to_double(cxml_string *str);

void cxml_set_literal(
        cxml_number *literal,
        cxml_literal_t literal_type,
        cxml_string *str);

int _cxml_is_integer(const char *start, int len);

int _cxml_is_double(const char *start, int len);

cxml_number cxml_literal_to_num(cxml_string *str);

bool cxml_number_is_d_equal(double a, double b);

bool cxml_number_is_equal(cxml_number* num1, cxml_number* num2);

bool cxml_number_is_greater(cxml_number* num1, cxml_number* num22);

bool cxml_number_is_less(cxml_number* num1, cxml_number* num2);

bool cxml_number_is_not_equal(cxml_number* num1, cxml_number* num2);

#endif //CXML_CXLITERAL_H
